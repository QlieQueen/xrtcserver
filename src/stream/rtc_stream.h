#ifndef __RTC_STREAM_H_
#define __RTC_STREAM_H_

#include <memory>
#include <string>
#include <stdint.h>
#include <rtc_base/rtc_certificate.h>


#include "base/event_loop.h"
#include "ice/port_allocator.h"
#include "pc/peer_connection.h"

namespace xrtc {

class RtcStream {
public:
    RtcStream(EventLoop* el, PortAllocator* allocator, uint64_t uid, const std::string& stream_name,
        bool audio, bool video, uint32_t log_id);
    
    virtual ~RtcStream();

    int start(rtc::RTCCertificate* certificate);
    int set_remote_sdp(const std::string& sdp);

    virtual std::string create_offer() = 0;

    uint64_t uid() { return _uid; }

protected:
    EventLoop* _el;
    uint64_t _uid;
    std::string _stream_name;
    bool _audio;
    bool _video;
    uint32_t _log_id;

    std::unique_ptr<PeerConnection> _pc;
};


}

#endif