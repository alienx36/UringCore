#ifndef URINGCORE_NET_SOCKET_HPP
#define URINGCORE_NET_SOCKET_HPP

#include <cstdint>

namespace uringcore::net {

class Socket {
public:
    static int create_nonblocking_tcp();
    static bool set_nonblocking(int fd);
    static bool set_reuseaddr(int fd);
    static bool set_reuseport(int fd);
    static bool bind_and_listen(int fd, uint16_t port, int backlog = 1024);
    static void close_fd(int fd);
};

} // namespace uringcore::net

#endif // URINGCORE_NET_SOCKET_HPP
