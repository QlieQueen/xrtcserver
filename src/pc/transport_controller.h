#ifndef __TRANSPORT_CONTROLLER_H_
#define __TRANSPORT_CONTROLLER_H_

#include "ice/candidate.h"
#include "ice/ice_agent.h"
#include "ice/ice_def.h"
#include "ice/port_allocator.h"
#include "pc/session_description.h"
#include <rtc_base/third_party/sigslot/sigslot.h>

namespace xrtc {

class TransportController : public sigslot::has_slots<> {
public:
    TransportController(EventLoop* el, PortAllocator* allocator);
    ~TransportController();

    int set_local_description(SessionDescription* desc);
    int set_remote_description(SessionDescription* desc);

public:
    sigslot::signal4<TransportController*, const std::string&, IceCandidateComponent,
        const std::vector<Candidate>&> signal_candidate_allocate_done;

private:
    void on_candidate_allocate_done(IceAgent* agent, 
            const std::string& transport_name,
            IceCandidateComponent component,
            const std::vector<Candidate>& candidates);

private:
    EventLoop* _el;
    IceAgent* _ice_agent;
};

};

#endif