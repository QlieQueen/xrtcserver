#include "stream/rtc_stream.h"
#include "ice/port_allocator.h"
#include "pc/peer_connection.h"
#include "pc/peer_connection_def.h"

#include <rtc_base/rtc_certificate.h>
#include <rtc_base/logging.h>

namespace xrtc {

RtcStream::RtcStream(EventLoop* el, PortAllocator* allocator, 
        uint64_t uid, const std::string& stream_name,
        bool audio, bool video, uint32_t log_id) :
    _el(el), _uid(uid), _stream_name(stream_name), _audio(audio),
    _video(video), _log_id(log_id),
    _pc(new PeerConnection(el, allocator))
{
    _pc->signal_connection_state.connect(this, &RtcStream::_on_connection_state);
}
    
RtcStream::~RtcStream() {

}

void RtcStream::_on_connection_state(PeerConnection*, PeerConnectionState state) {
    if (_state == state) {
        return;
    }

    RTC_LOG(LS_INFO) << "PeerConnectionState change from " << _state
        << " to " << state;
    _state = state;
}

int RtcStream::start(rtc::RTCCertificate* certificate) {
    return _pc->init(certificate);
}

int RtcStream::set_remote_sdp(const std::string& sdp) {
    return _pc->set_remote_sdp(sdp);
}

} // namespace xrtc