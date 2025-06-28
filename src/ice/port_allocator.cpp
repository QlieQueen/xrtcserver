#include "ice/port_allocator.h"
#include "base/network.h"

namespace xrtc {

PortAllocator::PortAllocator() :
    _network_manager(new NetWorkManager())
{
    _network_manager->create_networks();
}

PortAllocator::~PortAllocator() = default;

const std::vector<Network*>& PortAllocator::get_networks() {
    return _network_manager->get_networks();
}

} //namespace xrtc 