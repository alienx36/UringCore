#include "core/worker.hpp"
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>

namespace uringcore::core {

Worker::Worker(uint32_t worker_id, int cpu_core_id) 
    : worker_id_(worker_id), cpu_core_id_(cpu_core_id), running_(false) {}

Worker::~Worker() {
    stop();
}

bool Worker::listen_on_port(uint16_t port) {
    listen_fd_ = net::Socket::create_nonblocking_tcp();
    if (listen_fd_ < 0) return false;

    net::Socket::set_reuseaddr(listen_fd_);
    net::Socket::set_reuseport(listen_fd_);

    return net::Socket::bind_and_listen(listen_fd_, port);
}

bool Worker::post_task(const Task& task) {
    if (!running_.load(std::memory_order_relaxed)) return false;
    {
        std::lock_guard<std::mutex> lock(task_mutex_);
        task_queue_.push(task);
    }
    return true;
}

bool Worker::post_task(Task&& task) {
    if (!running_.load(std::memory_order_relaxed)) return false;
    {
        std::lock_guard<std::mutex> lock(task_mutex_);
        task_queue_.push(std::move(task));
    }
    return true;
}

bool Worker::start(uint32_t queue_depth) {
    if (!engine_.init(queue_depth)) return false;

    running_ = true;
    thread_ = std::thread(&Worker::run, this);
    return true;
}

void Worker::stop() {
    bool expected = true;
    if (running_.compare_exchange_strong(expected, false)) {
        if (thread_.joinable()) thread_.join();
        engine_.shutdown();
        if (listen_fd_ >= 0) {
            net::Socket::close_fd(listen_fd_);
            listen_fd_ = -1;
        }
    }
}

void Worker::run() {
    if (cpu_core_id_ >= 0) {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(cpu_core_id_, &cpuset);
        pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    }

    struct ConnectionState : public IOEvent {
        enum Op { ACCEPT, READ, WRITE } op;
        int client_fd{-1};
        char buffer[1024]{};
    };

    ConnectionState* accept_ev = nullptr;
    if (listen_fd_ >= 0) {
        accept_ev = new ConnectionState();
        accept_ev->op = ConnectionState::ACCEPT;
        engine_.submit_accept(listen_fd_, accept_ev);
    }

    constexpr char HTTP_RESP[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 13\r\nConnection: keep-alive\r\n\r\nHello, World!";
    struct io_uring* ring = engine_.get_ring();

    while (running_.load(std::memory_order_relaxed)) {
        // Drain posted tasks cleanly
        std::queue<Task> local_tasks;
        {
            std::lock_guard<std::mutex> lock(task_mutex_);
            local_tasks.swap(task_queue_);
        }
        while (!local_tasks.empty()) {
            if (local_tasks.front()) {
                local_tasks.front()();
            }
            local_tasks.pop();
        }

        if (!ring) {
            usleep(100);
            continue;
        }

        struct io_uring_cqe* cqe = nullptr;
        int ret = io_uring_peek_cqe(ring, &cqe);
        if (ret < 0 || !cqe) {
            if (running_.load(std::memory_order_relaxed)) {
                io_uring_submit(ring);
            }
            usleep(10);
            continue;
        }

        auto* ev = reinterpret_cast<IOEvent*>(io_uring_cqe_get_data(cqe));
        int res = cqe->res;
        io_uring_cqe_seen(ring, cqe);

        if (!ev) continue;
        auto* conn = static_cast<ConnectionState*>(ev);

        if (conn->op == ConnectionState::ACCEPT) {
            if (res >= 0 && running_.load(std::memory_order_relaxed)) {
                auto* client_conn = new ConnectionState();
                client_conn->op = ConnectionState::READ;
                client_conn->client_fd = res;
                engine_.submit_read(res, client_conn->buffer, sizeof(client_conn->buffer), client_conn);
            }
            if (running_.load(std::memory_order_relaxed) && listen_fd_ >= 0) {
                engine_.submit_accept(listen_fd_, accept_ev);
            }
        } else if (conn->op == ConnectionState::READ) {
            if (res > 0 && running_.load(std::memory_order_relaxed)) {
                conn->op = ConnectionState::WRITE;
                engine_.submit_write(conn->client_fd, HTTP_RESP, sizeof(HTTP_RESP) - 1, conn);
            } else {
                if (conn->client_fd >= 0) net::Socket::close_fd(conn->client_fd);
                delete conn;
            }
        } else if (conn->op == ConnectionState::WRITE) {
            if (res > 0 && running_.load(std::memory_order_relaxed)) {
                conn->op = ConnectionState::READ;
                engine_.submit_read(conn->client_fd, conn->buffer, sizeof(conn->buffer), conn);
            } else {
                if (conn->client_fd >= 0) net::Socket::close_fd(conn->client_fd);
                delete conn;
            }
        }
    }

    if (accept_ev) {
        delete accept_ev;
    }
}

} // namespace uringcore::core
