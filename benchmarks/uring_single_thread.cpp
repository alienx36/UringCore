#include "core/uring_engine.hpp"
#include "net/socket.hpp"
#include <iostream>
#include <unistd.h>
#include <csignal>

using namespace uringcore;

static bool g_running = true;

void signal_handler(int) {
    g_running = false;
}

int main() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    int server_fd = net::Socket::create_nonblocking_tcp();
    if (server_fd < 0) return 1;

    net::Socket::set_reuseaddr(server_fd);
    net::Socket::set_reuseport(server_fd);

    if (!net::Socket::bind_and_listen(server_fd, 8081)) {
        return 1;
    }

    core::UringEngine engine;
    if (!engine.init(1024)) return 1;

    struct ConnectionState : public core::IOEvent {
        enum Op { ACCEPT, READ, WRITE } op;
        int client_fd{-1};
        char buffer[1024]{};
    };

    ConnectionState accept_ev;
    accept_ev.op = ConnectionState::ACCEPT;
    engine.submit_accept(server_fd, &accept_ev);

    constexpr char HTTP_RESP[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 13\r\nConnection: keep-alive\r\n\r\nHello, World!";
    struct io_uring* ring = engine.get_ring();

    std::cout << "[Uring Single Thread] Running on port 8081...\n";

    while (g_running) {
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
                engine.submit_read(res, client_conn->buffer, sizeof(client_conn->buffer), client_conn);
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
                engine.submit_read(conn->client_fd, conn->buffer, sizeof(conn->buffer), conn);
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
