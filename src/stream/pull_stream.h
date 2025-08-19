#ifndef __PULL_STREAM_H_
#define __PULL_STREAM_H_

#include "ice/port_allocator.h"
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
};

}

#endif