#include "stream/rtc_stream.h"

namespace xrtc{

RtcStream::RtcStream(EventLoop* el, uint64_t uid, const std::string& stream_name,
        bool audio, bool video, uint32_t log_id) :
    _el(el), _uid(uid), _stream_name(stream_name), _audio(audio),
    _video(video), _log_id(log_id),
    _pc(new PeerConnection(el))
{

}
    
RtcStream::~RtcStream() {

}

} // namespace xrtc