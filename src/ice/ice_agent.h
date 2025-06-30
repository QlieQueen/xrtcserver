#ifndef __ICE_AGENT_H_
#define __ICE_AGENT_H_

#include <vector>
#include <string>

#include "ice/candidate.h"
#include "ice/icg_credentials.h"
#include "ice/icg_def.h"
#include "base/event_loop.h"
#include "ice/icg_transport_channel.h"
#include "ice/port_allocator.h"
#include "rtc_base/third_party/sigslot/sigslot.h"

namespace xrtc {

class IceAgent : public sigslot::has_slots<> {
public:
    IceAgent(EventLoop* el, PortAllocator* allocator);
    ~IceAgent();

    bool create_channel(EventLoop* el, const std::string& transport_name,
            IceCandidateComponent component);
    IceTransportChannel* get_channel(const std::string& transport_name,
            IceCandidateComponent component);

    void set_ice_params(const std::string& transport_name,
        IceCandidateComponent component,
        const IceParamters& ice_params);
    void gathering_candidate();

    void on_candidate_allocate_done(IceTransportChannel*, 
            const std::vector<Candidate>&);

public:
    sigslot::signal4<IceAgent*, const std::string&, IceCandidateComponent,
        const std::vector<Candidate>&> signal_candidate_allocate_done;        

private:
    std::vector<IceTransportChannel*>::iterator _get_channel(
        const std::string& transport_name,
        IceCandidateComponent component
    );

private:
    EventLoop* _el;
    std::vector<IceTransportChannel*> _channels;
    PortAllocator* _allocator;
};

}


#endif