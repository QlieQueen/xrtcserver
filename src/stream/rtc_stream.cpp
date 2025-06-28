#include "stream/rtc_stream.h"
#include "ice/port_allocator.h"
#include "rtc_base/rtc_certificate.h"

namespace xrtc{

RtcStream::RtcStream(EventLoop* el, PortAllocator* allocator, 
        uint64_t uid, const std::string& stream_name,
        bool audio, bool video, uint32_t log_id) :
    _el(el), _uid(uid), _stream_name(stream_name), _audio(audio),
    _video(video), _log_id(log_id),
    _pc(new PeerConnection(el, allocator))
{

}
    
RtcStream::~RtcStream() {

}

int RtcStream::start(rtc::RTCCertificate* certificate) {
    return _pc->init(certificate);
}


} // namespace xrtc