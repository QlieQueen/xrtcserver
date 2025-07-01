#ifndef __ICE_TRANSPORT_CHANNEL_H_
#define __ICE_TRANSPORT_CHANNEL_H_

#include <vector>
#include <string>

#include <rtc_base/third_party/sigslot/sigslot.h>

#include "base/event_loop.h"
#include "ice/candidate.h"
#include "ice/ice_credentials.h"
#include "ice/ice_def.h"
#include "ice/port_allocator.h"

namespace xrtc {

class IceTransportChannel {
public:
    IceTransportChannel(EventLoop* el, PortAllocator* allocator, 
            const std::string transport_name,
            IceCandidateComponent component);
    virtual ~IceTransportChannel();

    std::string transport_name() {
        return _transport_name;
    }

    IceCandidateComponent component() {
        return _component;
    }

    void set_ice_params(const IceParamters& ice_params);
    void set_remote_ice_params(const IceParamters& ice_params);
    void gathering_candidate();

public:
    sigslot::signal2<IceTransportChannel*, const std::vector<Candidate>&>
        signal_candidate_allocate_done;

private:
    EventLoop* _el;
    std::string _transport_name; // audio video
    IceCandidateComponent _component;
    PortAllocator* _allocator;
    IceParamters _ice_params;
    IceParamters _remote_ice_params;
    std::vector<Candidate> _local_candidates;
};

}

#endif