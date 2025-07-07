#ifndef XRTC_SERVER_DEF_H_
#define XRTC_SERVER_DEF_H_

#define MAX_RES_BUF 4096

#define CMDNO_PUSH     1
#define CMDNO_PULL     2
#define CMDNO_ANSWER   3
#define CMDNO_STOPPUSH 4
#define CMDNO_STOPPULL 5

#include <string>

namespace xrtc {

struct RtcMsg {
    int cmdno = -1;
    uint64_t uid = 0;
    std::string stream_name;
    std::string stream_type;
    int audio = 0;
    int video = 0;
    uint32_t log_id = 0;
    void* worker = nullptr;
    void* conn = nullptr;
    int fd = 0;
    std::string sdp;
    int err_no = 0;
    void* certificate = nullptr;  // rtc_worker初始化时生成(RtcServer::init)
};


} // namespace xrtc

#endif