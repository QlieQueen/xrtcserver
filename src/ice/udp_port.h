#ifndef __UDP_PORT_H_
#define __UDP_PORT_H_

#include <memory>
#include <vector>
#include <string>
#include <rtc_base/socket_address.h>

#include "base/event_loop.h"
#include "base/network.h"
#include "base/async_udp_socket.h"
#include "ice/ice_credentials.h"
#include "ice/ice_def.h"
#include "ice/candidate.h"
#include "ice/stun.h"
#include "rtc_base/third_party/sigslot/sigslot.h"

namespace xrtc {

class UDPPort : public sigslot::has_slots<> {
public:
    UDPPort(EventLoop* el,
            const std::string& transport_name,
            IceCandidateComponent component,
            IceParamters ice_params);
    ~UDPPort();

    int create_ice_candidate(Network* network, int min_port, int max_port, Candidate& c);
    bool get_stun_message(const char* data, size_t len,
            std::unique_ptr<StunMessage>* out_msg);

private:
    void _on_read_packet(AsyncUdpSocket* socket, char* buf, size_t size,
        const rtc::SocketAddress& addr, int64_t ts);
    bool _parse_stun_username(StunMessage* stun_msg, std::string* local_ufrag,
        std::string* remote_ufrag);

private:
    EventLoop* _el;
    std::string _transport_name;
    IceCandidateComponent _component;
    IceParamters _ice_params;
    int _socket = -1;
    std::unique_ptr<AsyncUdpSocket> _async_socket;
    rtc::SocketAddress _local_addr;
    std::vector<Candidate> _candidate;
};

} // namepsace xrtc

#endif