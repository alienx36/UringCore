#include "net/socket.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

namespace uringcore::net {

int Socket::create_nonblocking_tcp() {
    return ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
}

bool Socket::set_nonblocking(int fd) {
    int flags = ::fcntl(fd, F_GETFL, 0);
    if (flags == -1) return false;
    return ::fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1;
}

bool Socket::set_reuseaddr(int fd) {
    int opt = 1;
    return ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == 0;
}

bool Socket::set_reuseport(int fd) {
    int opt = 1;
    return ::setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == 0;
}

bool Socket::bind_and_listen(int fd, uint16_t port, int backlog) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << "[Socket] Failed to bind to port " << port << ": " << strerror(errno) << "\n";
        return false;
    }

    if (::listen(fd, backlog) < 0) {
        std::cerr << "[Socket] Failed to listen on port " << port << ": " << strerror(errno) << "\n";
        return false;
    }

    return true;
}

void Socket::close_fd(int fd) {
    if (fd >= 0) {
        ::close(fd);
    }
}

} // namespace uringcore::net
