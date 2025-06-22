#ifndef __RTC_SERVER_H_
#define __RTC_SERVER_H_

#include <queue>
#include <mutex>
#include <memory>
#include <thread>

#include "xrtc_server_def.h"
#include "base/event_loop.h"
//#include "server/rtc_worker.h"

namespace xrtc {

struct RtcServerOptions {
    int worker_num;
};

class RtcWorker;

class RtcServer {
public:
    enum {
        QUIT = 0,
        RTC_MSG = 1
    };

    RtcServer();
    ~RtcServer();

    int init(const char* conf_file);
    bool Start();
    void stop();
    int notify(int msg);
    void join();
    int send_rtc_msg(std::shared_ptr<RtcMsg>);
    void push_msg(std::shared_ptr<RtcMsg>);
    std::shared_ptr<RtcMsg> pop_msg();

    friend void rtc_server_recv_notify(EventLoop*, IOWatcher*, int, int, void*);

private:
    void _process_notify(int msg);
    void _stop();
    void _process_rtc_msg();
    int _create_worker(int worker_id);
    RtcWorker* _get_worker(const std::string& stream_name);

private:
    EventLoop* _el;    
    RtcServerOptions _options;
    std::thread* _thread = nullptr;

    IOWatcher* _pipe_watcher = nullptr;
    int _notify_recv_fd = -1;
    int _notify_send_fd = -1;

    std::queue<std::shared_ptr<RtcMsg>> _q_msg;
    std::mutex _q_msg_mutex;

    std::vector<RtcWorker*> _workers;
};

} // namespace xrtc

#endif