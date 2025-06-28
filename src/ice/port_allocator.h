#ifndef __PORT_ALLOCATOR_H_
#define __PORT_ALLOCATOR_H_

#include <memory>
#include "base/network.h"

namespace xrtc {

class PortAllocator {
public:
    PortAllocator();
    ~PortAllocator();

    const std::vector<Network*>& get_networks();

private:
    std::unique_ptr<NetWorkManager> _network_manager;

};

} // namespace xrtc

#endif 