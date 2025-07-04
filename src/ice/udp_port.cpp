#include <memory>
#include <netinet/in.h>
#include <sstream>
#include <rtc_base/logging.h>
#include <rtc_base/crc32.h>
#include "rtc_base/socket_address.h"
#include "rtc_base/string_encode.h"
#include <string>

#include "ice/udp_port.h"
#include "base/async_udp_socket.h"
#include "base/socket.h"
#include "base/network.h"
#include "ice/ice_def.h"
#include "ice/stun.h"

namespace xrtc {

UDPPort::UDPPort(EventLoop* el,
        const std::string& transport_name,
        IceCandidateComponent component,
        IceParamters ice_params) :
    _el(el),
    _transport_name(transport_name),
    _component(component),
    _ice_params(ice_params)
{

}

UDPPort::~UDPPort() {

}

std::string compute_foundation(const std::string& type,
        const std::string& protocol,
        const std::string& relay_protocol,
        const rtc::SocketAddress& base)
{
    std::stringstream ss;
    ss << type << base.HostAsURIString() << protocol << relay_protocol;
    return std::to_string(rtc::ComputeCrc32(ss.str()));
}

int UDPPort::create_ice_candidate(Network* network, int min_port, int max_port, 
        Candidate& c)
{
    _socket = create_udp_socket(network->ip().family());
    if (_socket < 0) {
        return -1;
    }

    if (sock_setnonblock(_socket) != 0) {
        return -1;
    }

    sockaddr_in addr_in;
    addr_in.sin_family = network->ip().family();
    // 因为目前network里是强制填写公网IP，当bind的IP地址不是本地的地址时，会报错（errno：99）。
    //addr_in.sin_addr = network->ip().ipv4_address();
    addr_in.sin_addr.s_addr = INADDR_ANY;

    if (sock_bind(_socket, (struct sockaddr*)&addr_in, sizeof(sockaddr), 
            min_port, max_port))
    {
        return -1;
    }

    int port = 0;
    if (sock_get_address(_socket, nullptr, &port) != 0) {
        return -1;
    }

    _local_addr.SetIP(network->ip());
    _local_addr.SetPort(port);


    _async_socket = std::make_unique<AsyncUdpSocket>(_el, _socket);
    _async_socket->signal_read_packet.connect(this, 
            &UDPPort::_on_read_packet);

    RTC_LOG(LS_INFO) << "prepared socket address: " << _local_addr.ToString();

    c.component = _component;
    c.protocol = "udp";
    c.address = _local_addr;
    c.port = port;
    c.priority = c.get_priority(ICE_TYPE_PREFERENCE_HOST, 0, 0); // 无3G/WIFI
    c.username = _ice_params.ice_ufrag;
    c.password = _ice_params.ice_pwd;
    c.type = LOCAL_PORT_TYPE;
    c.foundation = compute_foundation(c.type, c.protocol, "", c.address);

    _candidate.push_back(c);

    return 0;
}

void UDPPort::_on_read_packet(AsyncUdpSocket* socket, char* buf, size_t size,
        const rtc::SocketAddress& addr, int64_t ts)
{
    std::unique_ptr<StunMessage> stun_msg;
    std::string remote_ufrag;
    bool res = get_stun_message(buf, size, addr, &stun_msg, &remote_ufrag);

    RTC_LOG(LS_WARNING) << "============res: " << res;

}

bool UDPPort::get_stun_message(const char* data, size_t len,
        const rtc::SocketAddress& addr,
        std::unique_ptr<StunMessage>* out_msg,
        std::string* out_username)
{
    if (!StunMessage::validate_fingerprint(data, len)) {
        return false;
    }

    out_username->clear();

    std::unique_ptr<StunMessage> stun_msg = std::make_unique<StunMessage>();
    rtc::ByteBufferReader buf(data, len);
    if (!stun_msg->read(&buf) || buf.Length() != 0) {
        return false;
    }

    if (STUN_BINDING_REQUEST == stun_msg->type()) {
        if (!stun_msg->get_byte_string(STUN_ATTR_USERNAME) ||
                !stun_msg->get_byte_string(STUN_ATTR_MESSAGE_INTEGRITY))
        {
            RTC_LOG(LS_WARNING) << to_string() << ": received "
                << stun_method_to_string(stun_msg->type())
                << " without username/M-I from "
                << addr.ToString();
            send_binding_error_response(stun_msg.get(), addr, STUN_ERROR_BAD_REQUEST,
                STUN_ERROR_REASON_BAD_REQUEST);
            return true;
        }

        std::string local_ufrag;
        std::string remote_ufrag;
        if (!_parse_stun_username(stun_msg.get(), &local_ufrag, &remote_ufrag) ||
                local_ufrag != _ice_params.ice_ufrag)
        {
            // todo
            RTC_LOG(LS_WARNING) << to_string() << ": received "
                << stun_method_to_string(stun_msg->type())
                << " with bad local_ufrag: " << local_ufrag
                << " from " << addr.ToString();
            send_binding_error_response(stun_msg.get(), addr, STUN_ERROR_UNATHORIZED,
                STUN_ERROR_REASON_UNATHORIZED);
            return true;
        }

        if (stun_msg->validate_message_integrity(_ice_params.ice_pwd) != 
                StunMessage::IntegrityStatus::k_integrity_ok)
        {
            RTC_LOG(LS_WARNING) << to_string() << ": received "
                << stun_method_to_string(stun_msg->type())
                << " with Bad M-I from "
                << addr.ToString();
            send_binding_error_response(stun_msg.get(), addr, STUN_ERROR_UNATHORIZED,
                STUN_ERROR_REASON_UNATHORIZED);
            return true;
        }

        *out_username = remote_ufrag;

    }

    *out_msg = std::move(stun_msg);

    return true;
}

bool UDPPort::_parse_stun_username(StunMessage* stun_msg, std::string* local_ufrag,
        std::string* remote_ufrag)
{
    local_ufrag->clear();
    remote_ufrag->clear();

    const StunByteStringAttribute* attr = stun_msg->get_byte_string(STUN_ATTR_USERNAME);
    if (!attr) {
        return false;
    }

    // RFRAG:UFRAG
    std::string username = attr->get_string();
    std::vector<std::string> fields;
    rtc::split(username, ':', &fields);
    if (fields.size() != 2) {
        return false;
    }

    *local_ufrag = fields[0];
    *remote_ufrag = fields[1];

    RTC_LOG(LS_WARNING) << "local ufrag: " << *local_ufrag << ", remote ufrag: "
        << *remote_ufrag;

    return true;
}

std::string UDPPort::to_string() {
    std::stringstream ss;
    ss << "Port[" << this << ":" << _transport_name << ":" << _component
        << ":" << _ice_params.ice_ufrag << ":" << _ice_params.ice_pwd
        << ":" << _local_addr.ToString() << "]";
    return ss.str();
}

void UDPPort::send_binding_error_response(StunMessage* stun_msg,
        const rtc::SocketAddress& addr,
        int err_code,
        const std::string& reason)
{

}

} // namespace xrtc