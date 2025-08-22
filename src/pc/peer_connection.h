#ifndef __PEER_CONNECTION_H_
#define __PEER_CONNECTION_H_

#include <memory>
#include <string>

#include <rtc_base/rtc_certificate.h>

#include "base/event_loop.h"
#include "ice/candidate.h"
#include "ice/ice_def.h"
#include "ice/port_allocator.h"
#include "pc/peer_connection_def.h"
#include "pc/session_description.h"
#include "pc/transport_controller.h"
#include "pc/stream_params.h"
#include "rtc_base/third_party/sigslot/sigslot.h"



namespace xrtc {

struct RTCOfferAnswerOptions {
    bool send_audio = true;
    bool send_video = true;
    bool recv_audio = true;
    bool recv_video = true;
    bool use_rtp_mux = true; // bundle
    bool use_rtcp_mux = true; // rtp和rtcp是否复用同一传输通道的选项
    bool dtls_on = true;
};

class PeerConnection : public sigslot::has_slots<> {
public:
    PeerConnection(EventLoop* el, PortAllocator* allocator);

    int init(rtc::RTCCertificate* certificate);
    void destroy();
    std::string create_offer(const RTCOfferAnswerOptions& options);
    int set_remote_sdp(const std::string& sdp);

    SessionDescription* remote_desc() { return _remote_desc.get(); }
    SessionDescription* local_desc() { return _local_desc.get(); }

    void add_audio_source(const std::vector<StreamParams>& source) {
        _audio_source = source;
    }

    void add_video_source(const std::vector<StreamParams>& source) {
        _video_source = source;
    }

public:
    sigslot::signal2<PeerConnection*, PeerConnectionState> signal_connection_state;

private:
    ~PeerConnection();
    void on_candidate_allocate_done(TransportController* transport_controller,
            const std::string& transport_name,
            IceCandidateComponent component,
            const std::vector<Candidate>& candidates);
    void _on_connection_state(TransportController*, PeerConnectionState state);
    friend void destroy_timer_cb(EventLoop* el, TimerWatcher* w, void* data);

private:
    EventLoop* _el;
    std::unique_ptr<SessionDescription> _local_desc;
    std::unique_ptr<SessionDescription> _remote_desc;
    rtc::RTCCertificate* _certificate = nullptr;
    std::unique_ptr<TransportController> _transport_controller;
    TimerWatcher* _destroy_timer = nullptr;
    std::vector<StreamParams> _audio_source;
    std::vector<StreamParams> _video_source;
};

} // namespace xrtc


#endif