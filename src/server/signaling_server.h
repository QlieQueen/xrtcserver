#ifndef __SIGNALING_SERVER_H_
#define __SIGNALING_SERVER_H_

#include <string>
#include <thread>

#include "../base/event_loop.h"

namespace xrtc {

struct SignalingServerOptions{
    std::string host;
    int port;
    int worker_num;
    int connection_timeout;
};

class SignalingServer {
public:
    enum {
        QUIT = 0
    };
    SignalingServer();
    ~SignalingServer();

    int init(const char* conf_file);
    bool start();
    void stop();
    int notify(int msg);
    void join();

    friend void signaling_server_recv_nofity(EventLoop* el, IOWatcher* w, 
        int fd, int events, void* data);

private:
    void _process_notify(int msg);
    void _stop();
private:
    SignalingServerOptions _options;
    EventLoop* _el;
    IOWatcher* _io_watcher = nullptr;
    IOWatcher* _pipe_watcher = nullptr;
    int _notify_recv_fd = -1;
    int _notify_send_fd = -1;
    std::thread* _thread = nullptr;



    int _listen_fd = -1;
};

} // namespace xrtc

#endif