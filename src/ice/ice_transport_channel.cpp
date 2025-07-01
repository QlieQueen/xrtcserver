#include <rtc_base/logging.h>

#include "ice/ice_transport_channel.h"
#include "ice/candidate.h"
#include "ice/ice_credentials.h"
#include "ice/ice_def.h"
#include "ice/port_allocator.h"
#include "ice/udp_port.h"

namespace xrtc {

IceTransportChannel::IceTransportChannel(EventLoop* el, 
        PortAllocator* allocator,
        const std::string transport_name,
        IceCandidateComponent component) :
    _el(el),
    _transport_name(transport_name),
    _component(component),
    _allocator(allocator)
{
    RTC_LOG(LS_INFO) << "ice transport channel created, transport-name: " << _transport_name
        << ", component: " << _component;
}

IceTransportChannel::~IceTransportChannel()
{

}

void IceTransportChannel::set_ice_params(const IceParamters& ice_params) {
    RTC_LOG(LS_INFO) << "set gathering ICE param"
        << ", transport_name: " << _transport_name
        << ", component: " << _component
        << ", ufrag: " << ice_params.ice_ufrag
        << ", pwd: " << ice_params.ice_pwd;
    _ice_params = ice_params;
}

void IceTransportChannel::set_remote_ice_params(const IceParamters& ice_params) {
    RTC_LOG(LS_INFO) << "set remote ICE param"
        << ", transport_name: " << _transport_name
        << ", component: " << _component
        << ", ufrag: " << ice_params.ice_ufrag
        << ", pwd: " << ice_params.ice_pwd;
    _remote_ice_params = ice_params;
}

void IceTransportChannel::gathering_candidate() {
    if (_ice_params.ice_ufrag.empty() || _ice_params.ice_pwd.empty()) {
        RTC_LOG(LS_WARNING) << "cannot gathering candidate. because ICE param is empty."
            << ", transport_name: " << _transport_name
            << ", component: " << _component
            << ", ufrag: " << _ice_params.ice_ufrag
            << ", pwd: " << _ice_params.ice_pwd;
        return;
    }

    auto network_list = _allocator->get_networks();
    if (network_list.empty()) {
        RTC_LOG(LS_WARNING) << "cannot gathering candidate. because network list is empty."
            << ", transport_name: " << _transport_name
            << ", component: " << _component;
        return;
    }

    for (auto network : network_list) {
        UDPPort* port = new UDPPort(_el, _transport_name, _component, _ice_params);
        Candidate c;
        int ret = port->create_ice_candidate(network, _allocator->min_port(), 
            _allocator->max_port(), c);
        if (ret != 0) {
            continue;
        }

        _local_candidates.push_back(c);
    }

    signal_candidate_allocate_done(this, _local_candidates);
}

void PortAllocator::set_port_range(int min_port, int max_port) {
    if (min_port > 0) {
        _min_port = min_port;
    }

    if (max_port > 0) {
        _max_port = max_port;
    }
}

} // namespace xrtc
