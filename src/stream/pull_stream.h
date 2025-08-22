#ifndef __PULL_STREAM_H_
#define __PULL_STREAM_H_

#include "ice/port_allocator.h"
#include "pc/stream_params.h"
#include "stream/rtc_stream.h"

namespace xrtc {

class PullStream : public RtcStream {
public:
    PullStream(EventLoop* el, PortAllocator* alloctor, uint64_t uid, 
        const std::string& stream_name,
        bool audio, bool video, uint32_t log_id);

    ~PullStream() override;
    std::string create_offer() override;
    RtcStreamType stream_type() override { return RtcStreamType::k_pull; }

    void add_audio_source(const std::vector<StreamParams>& source);
    void add_video_source(const std::vector<StreamParams>& source);
};

}

#endif