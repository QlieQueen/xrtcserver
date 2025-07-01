#ifndef __ASYNC_UDP_SOCKET_H_
#define __ASYNC_UDP_SOCKET_H_

#include <rtc_base/third_party/sigslot/sigslot.h>
#include <rtc_base/socket_address.h>

#include "base/event_loop.h"

namespace xrtc {

class AsyncUdpSocket {
public:
    AsyncUdpSocket(EventLoop* el, int socket);
    ~AsyncUdpSocket();

    void recv_data();

public:
    sigslot::signal5<AsyncUdpSocket*, char*, size_t, const::rtc::SocketAddress&, int64_t>
        signal_read_packet;

private:
    EventLoop* _el;
    int _socket;
    IOWatcher* _socket_watcher;
    char* _buf;
    size_t _size;
};

}


#endif
