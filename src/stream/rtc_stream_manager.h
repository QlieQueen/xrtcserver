#ifndef __RTC_STREAM_MANAGER_H_
#define __RTC_STREAM_MANAGER_H_

#include <string>
#include <stdint.h>

#include "base/event_loop.h"

namespace xrtc {

class RtcStreamManager {
public:
    RtcStreamManager(EventLoop* el);
    ~RtcStreamManager();

    int create_push_stream(uint64_t uid, const std::string& stream_name, 
        bool audio, bool video, uint32_t log_id,
        std::string& offer);
    

private:
    EventLoop* _el;
};


}

#endif