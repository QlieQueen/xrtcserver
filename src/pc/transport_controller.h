#ifndef __TRANSPORT_CONTROLLER_H_
#define __TRANSPORT_CONTROLLER_H_

#include <map>

#include "ice/ice_agent.h"
#include "pc/session_description.h"
#include "pc/peer_connection_def.h"
#include "rtc_base/copy_on_write_buffer.h"
#include "rtc_base/third_party/sigslot/sigslot.h"

namespace xrtc {

class DtlsTransport;
class DtlsSrtpTransport;
enum class DtlsTransportState;

class TransportController : public sigslot::has_slots<> {
public:
    TransportController(EventLoop* el, PortAllocator* allocator);
    ~TransportController();

    int set_local_description(SessionDescription* desc);
    int set_remote_description(SessionDescription* desc);
    void set_local_certificate(rtc::RTCCertificate* cert);

public:
    sigslot::signal4<TransportController*, const std::string&, IceCandidateComponent,
        const std::vector<Candidate>&> signal_candidate_allocate_done;
    sigslot::signal2<TransportController*, PeerConnectionState> signal_connection_state;
    sigslot::signal3<TransportController*, rtc::CopyOnWriteBuffer*, int64_t>
        signal_rtp_packet_received;
    sigslot::signal3<TransportController*, rtc::CopyOnWriteBuffer*, int64_t>
        signal_rtcp_packet_received;
private:
    void on_candidate_allocate_done(IceAgent* agent, 
            const std::string& transport_name,
            IceCandidateComponent component,
            const std::vector<Candidate>& candidates);
    void _on_dtls_receiving_state(DtlsTransport*);
    void _on_dtls_writable_state(DtlsTransport*);
    void _on_dtls_state(DtlsTransport*, DtlsTransportState);
    void _on_rtp_packet_received(DtlsSrtpTransport*,
            rtc::CopyOnWriteBuffer* packet, int64_t ts);
    void _on_rtcp_packet_received(DtlsSrtpTransport*,
            rtc::CopyOnWriteBuffer* packet, int64_t ts);
    void _add_dtls_transport(DtlsTransport* dtls);
    void _on_ice_state(IceAgent*, IceTransportState);
    void _update_state();
    DtlsTransport* _get_dtls_transport(const std::string& transport_name);

private:
    EventLoop* _el;
    IceAgent* _ice_agent;
    std::map<std::string, DtlsTransport*> _dtls_transport_by_name;
    rtc::RTCCertificate* _local_certificate = nullptr;
    PeerConnectionState _pc_state = PeerConnectionState::k_new;
};

};

#endif