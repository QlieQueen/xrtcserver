#ifndef __TCP_CONNECTION_H_
#define __TCP_CONNECTION_H_

#include <rtc_base/sds.h>

#include "base/xhead.h"
#include "base/event_loop.h"

namespace xrtc {

class TcpConnection {
public:
    enum {
        STATE_HEAD = 0,
        STATE_BODY = 1
    };

    TcpConnection(int fd);
    ~TcpConnection();
public:
    int fd;
    char ip[64];
    int port;
    IOWatcher* io_watcher = nullptr;
    TimerWatcher* timer_watcher = nullptr;
    sds querybuf; // 存储读取的数据 redis
    size_t bytes_expected = XHEAD_SIZE; // 第一次读取头部大小
    size_t bytes_processed = 0; // 标记当前处理了buf里多少字节的数据
    int current_state = STATE_HEAD;
    unsigned long last_interaction = 0;
};


} // namespace xrtc

#endif