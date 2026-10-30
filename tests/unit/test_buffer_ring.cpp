#include "net/buffer_ring.hpp"
#include "core/uring_engine.hpp"
#include <iostream>
#include <cassert>
#include <cstring>

using namespace uringcore::core;
using namespace uringcore::net;

int main() {
    std::cout << "--- Starting Phase 4 Step 3 BufferRing Unit Tests ---\n";

    UringEngine engine;
    assert(engine.init(64));

    BufferRing ring;
    assert(ring.init(engine.get_ring(), 16));

    // Test buffer indexing arithmetic
    uint8_t* buf0 = ring.get_buffer(0);
    uint8_t* buf1 = ring.get_buffer(1);

    assert(buf0 != nullptr);
    assert(buf1 != nullptr);
    assert(buf1 == buf0 + 4096);

    // Test buffer payload write and recycling
    std::memcpy(buf0, "UringCore Test", 14);
    assert(std::memcmp(ring.get_buffer(0), "UringCore Test", 14) == 0);

    ring.recycle_buffer(0);
    ring.cleanup();

    std::cout << "--- BufferRing Unit Tests Passed Successfully ---\n";
    return 0;
}
