#include "stream/pull_stream.h"
#include "ice/port_allocator.h"
#include <rtc_base/logging.h>

namespace xrtc {

PullStream::PullStream(EventLoop* el, PortAllocator* allocator, uint64_t uid, const std::string& stream_name,
        bool audio, bool video, uint32_t log_id) :
    RtcStream(el, allocator, uid, stream_name, audio, video, log_id)
{

}

PullStream::~PullStream() {
    RTC_LOG(LS_INFO) << to_string() << ": Push stream destroy";
}

// 对于xrtcserver来说，是向PullStream发送音视频
std::string PullStream::create_offer() {
    RTCOfferAnswerOptions options;
    options.send_audio = _audio;
    options.send_video = _video;
    options.recv_audio = false;
    options.recv_video = false;
    //options.use_rtcp_mux = false;  // rtp和rtcp是否复用的选项

    return _pc->create_offer(options);
}

void PullStream::add_audio_source(const std::vector<StreamParams>& source) {
    if (_pc) {
        _pc->add_audio_source(source);
    }
}

void PullStream::add_video_source(const std::vector<StreamParams>& source) {
    if (_pc) {
        _pc->add_video_source(source);
    }
}

}