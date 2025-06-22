#include <unistd.h>
#include <rtc_base/logging.h>
#include "server/rtc_worker.h"


namespace xrtc {

void rtc_worker_recv_notify(EventLoop* /*el*/, IOWatcher* /*w*/, int fd, 
    int /*events*/, void* data)
{
    int msg;
    if (read(fd, &msg, sizeof(int)) != sizeof(int)) {
        RTC_LOG(LS_WARNING) << "read from pipe error, errno: " << errno
            << ", errmsg: " << strerror(errno);
        return;
    }

    RtcWorker* worker = (RtcWorker*)data;
    worker->_process_notify(msg);

}

RtcWorker::RtcWorker(int worker_id, const RtcServerOptions& options) :
    _options(options),
    _worker_id(worker_id),
    _el(new EventLoop(this))
{

}

RtcWorker::~RtcWorker() {
    if (_el) {
        delete _el;
        _el = nullptr;
    }

    if (_thread) {
        delete _thread;
        _thread = nullptr;  
    }
}

int RtcWorker::init() {
    int fds[2];
    if (pipe(fds)) {
        RTC_LOG(WARNING) << "create pipe error, errno: " << errno 
            << ", errmsg: " << strerror(errno);
        return -1;
    }

    _notify_recv_fd = fds[0];
    _notify_send_fd = fds[1];

    _pipe_watcher = _el->create_io_event(rtc_worker_recv_notify, this);
    _el->start_io_event(_pipe_watcher, _notify_recv_fd, EventLoop::READ);

    return 0;
}

bool RtcWorker::start() {
    if (_thread) {
        RTC_LOG(WARNING) << "rtc worker already start, worker_id: " << _worker_id;
        return false;
    }

    _thread = new std::thread([=]() {
        RTC_LOG(INFO) << "rtc worker event loop start, worker_id: " << _worker_id;
        _el->start();
        RTC_LOG(INFO) << "rtc worker event loop stop, worker_id: " << _worker_id;
    }); 

    return true;
}

void RtcWorker::stop() {
    notify(QUIT);
}

int RtcWorker::notify(int msg) {
    int written = write(_notify_send_fd, &msg, sizeof(int));
    return written == sizeof(int) ? 0 : -1;
}

void RtcWorker::join() {
    if (_thread && _thread->joinable()) {
        _thread->join();
    }
}

void RtcWorker::_stop() {
    if (!_thread) {
        RTC_LOG(WARNING) << "rtc worker not running, worker_id: " << _worker_id;
        return;
    }

    _el->delete_io_event(_pipe_watcher);
    _el->stop();
    close(_notify_recv_fd);
    close(_notify_send_fd);

}

void RtcWorker::_process_notify(int msg) {
    switch (msg) {
        case QUIT:
            _stop();
        break;
        
        default:
            RTC_LOG(LS_WARNING) << "unknown msg: " << msg;
        break;
    }
    

}

} // namespace xrtc