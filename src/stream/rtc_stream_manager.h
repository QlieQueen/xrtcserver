#ifndef __RTC_STREAM_MANAGER_H_
#define __RTC_STREAM_MANAGER_H_

#include <cstdint>
#include <string>
#include <stdint.h>
#include <unordered_map>

#include <rtc_base/rtc_certificate.h>

#include "base/event_loop.h"
#include "ice/port_allocator.h"
#include "pc/peer_connection_def.h"
#include "stream/rtc_stream.h"

namespace xrtc {

class PushStream;

class RtcStreamManager : public RtcStreamListener {
public:
    RtcStreamManager(EventLoop* el);
    ~RtcStreamManager();

    int create_push_stream(uint64_t uid, const std::string& stream_name, 
        bool audio, bool video, uint32_t log_id,
        rtc::RTCCertificate* certificate,
        std::string& offer);

    int stop_push(uint64_t uid, const std::string& stream_name);

    int set_answer(uint64_t uid, const std::string& stream_name, 
        const std::string& answer, const std::string& stream_type,
        uint32_t log_id);

    PushStream* find_push_stream(const std::string& stream_name);
    void remove_push_stream(RtcStream* stream);
    void remove_push_stream(uint64_t uid, const std::string& stream_name);

    void on_connection_state(RtcStream* stream, PeerConnectionState state) override;

private:
    EventLoop* _el;
    std::unordered_map<std::string, PushStream*> _push_streams;
    std::unique_ptr<PortAllocator> _allocator;
};


}

#endif