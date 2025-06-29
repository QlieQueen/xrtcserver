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

    void set_port_range(int min_port, int max_port);
    int min_port() { return _min_port; }
    int max_port() { return _max_port; }

private:
    std::unique_ptr<NetWorkManager> _network_manager;
    int _min_port;
    int _max_port;
};

} // namespace xrtc

#endif 