#include "core/epoll_engine.hpp"
#include "core/uring_engine.hpp"
#include "net/connection.hpp"
#include "net/buffer_ring.hpp"
#include <iostream>
#include <cassert>
#include <unistd.h>
#include <sys/socket.h>

using namespace uringcore::core;
using namespace uringcore::net;

void test_epoll_teardown() {
    EpollEngine engine;
    assert(engine.init(64));

    int sv[2];
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0);

    Connection conn;
    conn.init(sv[0]);

    IOEvent cancel_event{};
    assert(engine.submit_read(conn.get_fd(), conn.get_read_buffer(), 4096, conn.get_io_event()));
    assert(engine.submit_cancel(conn.get_io_event(), &cancel_event));

    conn.reset();
    ::close(sv[0]);
    ::close(sv[1]);
    engine.shutdown();
    std::cout << "[PASS] EpollEngine Teardown Check\n";
}

void test_uring_teardown() {
    UringEngine engine;
    assert(engine.init(64));

    BufferRing buf_ring;
    assert(buf_ring.init(engine.get_ring(), 16));

    int sv[2];
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0);

    Connection conn;
    conn.init(sv[0]);

    IOEvent cancel_event{};
    assert(engine.submit_read(conn.get_fd(), conn.get_read_buffer(), 4096, conn.get_io_event()));
    assert(engine.submit_cancel(conn.get_io_event(), &cancel_event));

    buf_ring.cleanup();
    conn.reset();
    ::close(sv[0]);
    ::close(sv[1]);
    engine.shutdown();
    std::cout << "[PASS] UringEngine Teardown Check\n";
}

int main() {
    std::cout << "--- Starting Phase 4 Step 4 Teardown Sanitizer Tests ---\n";
    test_epoll_teardown();
    test_uring_teardown();
    std::cout << "--- All Phase 4 Teardown Sanitizer Tests Passed Successfully ---\n";
    return 0;
}
