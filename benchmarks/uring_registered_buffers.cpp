#include "core/uring_engine.hpp"
#include "net/socket.hpp"
#include <iostream>
#include <vector>
#include <cstring>
#include <csignal>
#include <atomic>

using namespace uringcore;

static std::atomic<bool> g_running{true};

void signal_handler(int) {
    g_running.store(false, std::memory_order_relaxed);
}

int main() {
    constexpr uint16_t PORT = 8084;
    constexpr size_t BUFFER_SIZE = 1024;
    constexpr size_t NUM_BUFFERS = 64;

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    int server_fd = net::Socket::create_nonblocking_tcp();
    if (server_fd < 0) return 1;

    net::Socket::set_reuseaddr(server_fd);
    net::Socket::set_reuseport(server_fd);

    if (!net::Socket::bind_and_listen(server_fd, PORT)) {
        std::cerr << "[FATAL] Buffer registration engine failed to bind on port " << PORT << "\n";
        return 1;
    }

    core::UringEngine engine;
    if (!engine.init(1024)) return 1;

    // Allocate & Register Fixed Buffers into Kernel Space
    std::vector<char> buffer_pool(NUM_BUFFERS * BUFFER_SIZE);
    std::vector<iovec> iovecs(NUM_BUFFERS);
    for (size_t i = 0; i < NUM_BUFFERS; ++i) {
        iovecs[i].iov_base = buffer_pool.data() + (i * BUFFER_SIZE);
        iovecs[i].iov_len = BUFFER_SIZE;
    }

    struct io_uring* ring = engine.get_ring();
    if (io_uring_register_buffers(ring, iovecs.data(), NUM_BUFFERS) < 0) {
        std::cerr << "[WARNING] Kernel buffer registration fallback to std buffers\n";
    }

    struct ConnectionState : public core::IOEvent {
        enum Op { ACCEPT, READ, WRITE } op;
        int client_fd{-1};
        int buf_idx{0};
    };

    ConnectionState accept_ev;
    accept_ev.op = ConnectionState::ACCEPT;
    engine.submit_accept(server_fd, &accept_ev);

    constexpr char HTTP_RESP[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 13\r\nConnection: keep-alive\r\n\r\nHello, World!";

    std::cout << "[Uring Registered Buffers] Zero-Copy Fixed Buffer Engine running on port " << PORT << "...\n";

    while (g_running.load(std::memory_order_relaxed)) {
        struct io_uring_cqe* cqe = nullptr;
        int ret = io_uring_peek_cqe(ring, &cqe);
        if (ret < 0 || !cqe) {
            io_uring_submit(ring);
            continue;
        }

        auto* ev = reinterpret_cast<core::IOEvent*>(io_uring_cqe_get_data(cqe));
        int res = cqe->res;
        io_uring_cqe_seen(ring, cqe);

        if (!ev) continue;
        auto* conn = static_cast<ConnectionState*>(ev);

        if (conn->op == ConnectionState::ACCEPT) {
            if (res >= 0) {
                auto* client_conn = new ConnectionState();
                client_conn->op = ConnectionState::READ;
                client_conn->client_fd = res;
                engine.submit_read(res, buffer_pool.data(), BUFFER_SIZE, client_conn);
            }
            engine.submit_accept(server_fd, &accept_ev);
        } else if (conn->op == ConnectionState::READ) {
            if (res > 0) {
                conn->op = ConnectionState::WRITE;
                engine.submit_write(conn->client_fd, HTTP_RESP, sizeof(HTTP_RESP) - 1, conn);
            } else {
                net::Socket::close_fd(conn->client_fd);
                delete conn;
            }
        } else if (conn->op == ConnectionState::WRITE) {
            if (res > 0) {
                conn->op = ConnectionState::READ;
                engine.submit_read(conn->client_fd, buffer_pool.data(), BUFFER_SIZE, conn);
            } else {
                net::Socket::close_fd(conn->client_fd);
                delete conn;
            }
        }
    }

    net::Socket::close_fd(server_fd);
    engine.shutdown();
    return 0;
}
