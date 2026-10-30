#ifndef URINGCORE_NET_BUFFER_POOL_HPP
#define URINGCORE_NET_BUFFER_POOL_HPP

#include "net/buffer_ring.hpp"

namespace uringcore::net {

// Canonical alias matching project specification for pre-allocated buffer management
using BufferPool = BufferRing;

} // namespace uringcore::net

#endif // URINGCORE_NET_BUFFER_POOL_HPP
