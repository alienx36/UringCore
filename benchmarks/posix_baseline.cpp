#include "net/socket.hpp"
#include <iostream>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <vector>
#include <cstring>
#include <fcntl.h>

constexpr int MAX_EVENTS = 1024;
constexpr char HTTP_RESPONSE[] = 
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 13\r\n"
    "Connection: keep-alive\r\n\r\n"
    "Hello, World!";

int main() {
    int listen_fd = uringcore::net::Socket::create_nonblocking_tcp();
    if (listen_fd < 0) {
        std::cerr << "Failed to create socket\n";
        return 1;
    }

    uringcore::net::Socket::set_reuseaddr(listen_fd);
    uringcore::net::Socket::set_reuseport(listen_fd);

    if (!uringcore::net::Socket::bind_and_listen(listen_fd, 8080)) {
        std::cerr << "Failed to bind/listen on port 8080\n";
        uringcore::net::Socket::close_fd(listen_fd);
        return 1;
    }

    int epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        std::cerr << "Failed to create epoll instance\n";
        uringcore::net::Socket::close_fd(listen_fd);
        return 1;
    }

    epoll_event ev{};
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = listen_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev) < 0) {
        std::cerr << "Failed to add listen socket to epoll\n";
        uringcore::net::Socket::close_fd(listen_fd);
        close(epoll_fd);
        return 1;
    }

    std::cout << "[POSIX Baseline] Running epoll server on port 8080...\n";

    std::vector<epoll_event> events(MAX_EVENTS);
    char buf[1024];

    while (true) {
        int nfds = epoll_wait(epoll_fd, events.data(), MAX_EVENTS, -1);
        for (int i = 0; i < nfds; ++i) {
            int fd = events[i].data.fd;

            if (fd == listen_fd) {
                while (true) {
                    int conn_fd = accept(listen_fd, nullptr, nullptr);
                    if (conn_fd < 0) {
                        break;
                    }
                    int flags = fcntl(conn_fd, F_GETFL, 0);
                    fcntl(conn_fd, F_SETFL, flags | O_NONBLOCK);

                    epoll_event conn_ev{};
                    conn_ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
                    conn_ev.data.fd = conn_fd;
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &conn_ev);
                }
            } else {
                ssize_t bytes_read = read(fd, buf, sizeof(buf));
                if (bytes_read > 0) {
                    ssize_t written = write(fd, HTTP_RESPONSE, sizeof(HTTP_RESPONSE) - 1);
                    (void)written;
                    epoll_event conn_ev{};
                    conn_ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
                    conn_ev.data.fd = fd;
                    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &conn_ev);
                } else {
                    close(fd);
                }
            }
        }
    }

    close(epoll_fd);
    uringcore::net::Socket::close_fd(listen_fd);
    return 0;
}
