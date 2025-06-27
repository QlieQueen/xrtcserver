#ifndef __ICE_AGENT_H_
#define __ICE_AGENT_H_

#include <vector>
#include <string>

#include "ice/icg_def.h"
#include "base/event_loop.h"
#include "ice/icg_transport_channel.h"

namespace xrtc {

class IceAgent {
public:
    IceAgent(EventLoop* el);
    ~IceAgent();

    bool create_channel(EventLoop* el, const std::string& transport_name,
            IceCandidateComponent component);
    IceTransportChannel* get_channel(const std::string& transport_name,
            IceCandidateComponent component);

private:
    std::vector<IceTransportChannel*>::iterator _get_channel(
        const std::string& transport_name,
        IceCandidateComponent component
    );

private:
    EventLoop* _el;
    std::vector<IceTransportChannel*> _channels;
};

}


#endif