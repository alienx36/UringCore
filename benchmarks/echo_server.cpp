#include "core/worker_group.hpp"
#include "net/socket.hpp"

#include <iostream>
#include <string>
#include <thread>
#include <csignal>
#include <atomic>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace uringcore::core;
using namespace uringcore::net;

namespace {
std::atomic<bool> g_running{true};

void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        g_running.store(false, std::memory_order_relaxed);
    }
}

int create_listener_socket(uint16_t port) {
    int listen_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) return -1;

    int opt = 1;
    ::setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    ::setsockopt(listen_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (::bind(listen_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        ::close(listen_fd);
        return -1;
    }

    if (::listen(listen_fd, SOMAXCONN) < 0) {
        ::close(listen_fd);
        return -1;
    }

    return listen_fd;
}

void process_client(int client_fd) {
    char buffer[4096];
    while (g_running.load(std::memory_order_relaxed)) {
        ssize_t bytes_read = ::read(client_fd, buffer, sizeof(buffer));
        if (bytes_read > 0) {
            ssize_t bytes_written = ::write(client_fd, buffer, bytes_read);
            (void)bytes_written;
        } else {
            break;
        }
    }
    ::close(client_fd);
}
} // namespace

int main(int argc, char* argv[]) {
    uint16_t port = 8080;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--port" && i + 1 < argc) port = std::atoi(argv[++i]);
    }

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    int listen_fd = create_listener_socket(port);
    if (listen_fd < 0) {
        std::cerr << "[FATAL] Failed to bind listening socket on port " << port << "\n";
        return 1;
    }

    std::cout << "=========================================\n";
    std::cout << " Starting UringCore Baseline Echo Server\n";
    std::cout << " Listening Port: " << port << "\n";
    std::cout << "=========================================\n";

    while (g_running.load(std::memory_order_relaxed)) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_fd = ::accept(listen_fd, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
        
        if (client_fd >= 0) {
            std::thread([client_fd]() {
                process_client(client_fd);
            }).detach();
        }
    }

    Socket::close_fd(listen_fd);
    return 0;
}
