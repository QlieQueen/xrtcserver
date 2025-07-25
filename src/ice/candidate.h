#ifndef __ICE_CANDIDATE_H_
#define __ICE_CANDIDATE_H_

#include <string>
#include "ice/ice_def.h"
#include <rtc_base/socket_address.h>

namespace xrtc {

class Candidate {
public:
    uint32_t get_priority(uint32_t type_preference,
        int network_adapter_preference,
        int relay_preference);

    std::string to_string() const;

public:
    IceCandidateComponent component;
    std::string protocol;
    rtc::SocketAddress address;
    int port = 0;
    uint32_t priority;
    std::string username;
    std::string password;
    std::string type;
    std::string foundation;
};

} // namespace xrtc


#endif 
