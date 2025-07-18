#include <algorithm>

#include <rtc_base/logging.h>

#include "ice/ice_agent.h"
#include "ice/ice_transport_channel.h"
#include "ice/port_allocator.h"

namespace xrtc {

IceAgent::IceAgent(EventLoop* el, PortAllocator* allocator) : 
    _el(el), _allocator(allocator) {}

IceAgent::~IceAgent() {}

IceTransportChannel* IceAgent::get_channel(const std::string& transport_name,
        IceCandidateComponent component)
{
    auto iter = _get_channel(transport_name, component);
    return iter == _channels.end() ? nullptr : *iter;    
}     

bool IceAgent::create_channel(EventLoop* el, const std::string& transport_name,
        IceCandidateComponent component) 
{
    if (get_channel(transport_name, component)) {
        return true;
    }
    
    auto channel = new IceTransportChannel(el, _allocator, transport_name, component);
    channel->signal_candidate_allocate_done.connect(this, 
        &IceAgent::on_candidate_allocate_done);
    _channels.push_back(channel);

    return true;
}

std::vector<IceTransportChannel*>::iterator IceAgent::_get_channel(
    const std::string& transport_name,
    IceCandidateComponent component)
{
    return std::find_if(_channels.begin(), _channels.end(),
        [transport_name, component](IceTransportChannel* channel) {
            return transport_name == channel->transport_name() && 
                component == channel->component();
        });
}

void IceAgent::set_ice_params(const std::string& transport_name,
    IceCandidateComponent component,
    const IceParamters& ice_params)
{
    auto channel = get_channel(transport_name, component);
    if (channel) {
        channel->set_ice_params(ice_params);
    }
}

void IceAgent::set_remote_ice_params(const std::string& transport_name,
        IceCandidateComponent component,
        const IceParamters& ice_params)
{
    auto channel = get_channel(transport_name, component);
    if (channel) {
        channel->set_remote_ice_params(ice_params);
    }   
}


void IceAgent::gathering_candidate() {
    for (auto channel : _channels) {
        channel->gathering_candidate();
    }
}


void IceAgent::on_candidate_allocate_done(IceTransportChannel* channel, 
        const std::vector<Candidate>& candidates)
{
    signal_candidate_allocate_done(this, channel->transport_name(),
        channel->component(), candidates);
}

} // namespace xrtc