#ifndef __ICE_DEF_H_
#define __ICE_DEF_H_

namespace xrtc {

#define LOCAL_PORT_TYPE "host"
#define PRFLX_PORT_TYPE "prfix"

extern const int ICE_UFRAG_LENGTH;
extern const int ICE_PWD_LENGTH;

enum IceCandidateComponent {
    RTP = 1,
    RTCP = 2
};

enum IcePriorityValue {
    ICE_TYPE_PREFERENCE_RELAY_UDP = 2,
    ICE_TYPE_PREFERENCE_SRFLX     = 100,
    ICE_TYPE_PREFERENCE_PRFLX     = 110,
    ICE_TYPE_PREFERENCE_HOST      = 126,
};

} // namespace xrtc


#endif