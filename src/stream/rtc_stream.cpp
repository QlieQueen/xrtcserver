#include "stream/rtc_stream.h"
#include "base/event_loop.h"
#include "ice/port_allocator.h"
#include "pc/peer_connection.h"
#include "pc/peer_connection_def.h"
#include "rtc_base/copy_on_write_buffer.h"

#include <rtc_base/rtc_certificate.h>
#include <rtc_base/logging.h>
#include <sstream>

namespace xrtc {

const size_t k_ice_timeout = 30000; // 30s

RtcStream::RtcStream(EventLoop* el, PortAllocator* allocator, 
        uint64_t uid, const std::string& stream_name,
        bool audio, bool video, uint32_t log_id) :
    _el(el), _uid(uid), _stream_name(stream_name), _audio(audio),
    _video(video), _log_id(log_id),
    _pc(new PeerConnection(el, allocator))
{
    _pc->signal_connection_state.connect(this, &RtcStream::_on_connection_state);
    _pc->signal_rtp_packet_received.connect(this, &RtcStream::_on_rtp_packet_received);
    _pc->signal_rtcp_packet_received.connect(this, &RtcStream::_on_rtcp_packet_received);
}
    
RtcStream::~RtcStream() {
    if (_ice_timeout_wather) {
        _el->delete_timer(_ice_timeout_wather);
        _ice_timeout_wather = nullptr;
    }
    _pc->destroy();
}

void RtcStream::_on_connection_state(PeerConnection*, PeerConnectionState state) {
    if (_state == state) {
        return;
    }

    RTC_LOG(LS_INFO) << to_string() << ": PeerConnectionState change from " << _state
        << " to " << state;
    _state = state;

    if (_state == PeerConnectionState::k_connected) {
        if (_ice_timeout_wather) {
            _el->delete_timer(_ice_timeout_wather);
            _ice_timeout_wather = nullptr;
        }
    }

    if (_listener) {
        _listener->on_connection_state(this, state);
    }
}

void RtcStream::_on_rtp_packet_received(PeerConnection*,
        rtc::CopyOnWriteBuffer* packet, int64_t /*ts*/)
{
    if (_listener) {
        _listener->on_rtp_packet_received(this, (const char*)packet->data(), packet->size());
    }
}

void RtcStream::_on_rtcp_packet_received(PeerConnection*,
        rtc::CopyOnWriteBuffer* packet, int64_t /*ts*/)
{
    if (_listener) {
        _listener->on_rtcp_packet_received(this, (const char*)packet->data(), packet->size());
    }
}

void ice_timeout_cb(EventLoop* el, TimerWatcher* w, void* data) {
    RtcStream* stream = (RtcStream*)data;
    if (stream->_state != PeerConnectionState::k_connected) {
        delete stream;
    }
}

int RtcStream::start(rtc::RTCCertificate* certificate) {
    _ice_timeout_wather = _el->create_timer(ice_timeout_cb, this, false);
    _el->start_timer(_ice_timeout_wather, k_ice_timeout * 1000);
    return _pc->init(certificate);
}

int RtcStream::set_remote_sdp(const std::string& sdp) {
    return _pc->set_remote_sdp(sdp);
}

int RtcStream::send_rtp(const char* data, size_t len) {
    if (_pc) {
        return _pc->send_rtp(data, len);
    }
    return -1;
}

int RtcStream::send_rtcp(const char* data, size_t len) {
    if (_pc) {
        return _pc->send_rtcp(data, len);
    }
    return -1;
}

std::string RtcStream::to_string() {
    std::stringstream ss;
    ss << "Stream[" << this << "|" << _uid << "|" << _stream_name << "]";
    return ss.str();
}

} // namespace xrtc