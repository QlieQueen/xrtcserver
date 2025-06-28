#include "pc/transport_controller.h"
#include "ice/ice_agent.h"
#include "ice/icg_def.h"
#include "ice/port_allocator.h"
#include <rtc_base/logging.h>

namespace xrtc {

TransportController::TransportController(EventLoop* el, PortAllocator* allocator) :
        _el(el),
        _ice_agent(new IceAgent(el, allocator))
{

}

TransportController::~TransportController() {

}


int TransportController::set_local_description(SessionDescription* desc) {
    if (!desc) {
        RTC_LOG(LS_WARNING) << "desc is null";
        return -1;
    }

    for (auto content : desc->contents()) {
        std::string mid = content->mid();

        if (desc->is_bundle(mid) && mid != desc->get_first_bundle_mid()) {
            continue;
        }

        _ice_agent->create_channel(_el, mid, IceCandidateComponent::RTP);
        if (!content->rtcp_mux()) {
            _ice_agent->create_channel(_el, mid, IceCandidateComponent::RTCP);
        }
    }

    _ice_agent->gathering_candidate();

    return 0;
}


} // namespace xrtc