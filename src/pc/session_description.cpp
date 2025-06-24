#include <sstream>

#include "pc/session_description.h"

namespace xrtc {

SessionDescription::SessionDescription(SdpType type) :
    _sdp_type(type)
{

}

SessionDescription::~SessionDescription() {

}

std::string SessionDescription::to_string() {
    std::stringstream ss;

    // version
    ss << "v=0\r\n";

    return ss.str();
}

}