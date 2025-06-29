#ifndef __UDP_PORT_H_
#define __UDP_PORT_H_

#include <string>

#include "base/event_loop.h"
#include "base/network.h"
#include "ice/icg_credentials.h"
#include "ice/icg_def.h"
#include "ice/candidate.h"

namespace xrtc {

class UDPPort {
public:
    UDPPort(EventLoop* el,
            const std::string& transport_name,
            IceCandidateComponent component,
            IceParamters ice_params);
    ~UDPPort();

    int create_ice_candidate(Network* network, Candidate& c);

private:
    EventLoop* _el;
    std::string _transport_name;
    IceCandidateComponent _component;
    IceParamters _ice_params;
};

} // namepsace xrtc

#endif