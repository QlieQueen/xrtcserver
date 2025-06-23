#ifndef __RTC_STREAM_H_
#define __RTC_STREAM_H_

#include <string>
#include <stdint.h>

#include "base/event_loop.h"

namespace xrtc {

class RtcStream {
public:
    RtcStream(EventLoop* el, uint64_t uid, const std::string& stream_name,
        bool audio, bool video, uint32_t log_id);
    
    virtual ~RtcStream();

protected:
    EventLoop* _el;
    uint64_t _uid;
    std::string _stream_name;
    bool _audio;
    bool _video;
    uint32_t _log_id;
};


}

#endif