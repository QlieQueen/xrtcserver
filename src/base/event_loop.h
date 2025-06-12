#ifndef __BASE_EVENT_LOOP_H_
#define __BASE_EVENT_LOOP_H_

struct ev_loop;

namespace xrtc {

class EventLoop;
class IOWatcher;
class TimerWatcher;

typedef void (*io_cb_t)(EventLoop* el, IOWatcher* w, int fd, int events, void* data);
typedef void (*timer_cb_t)(EventLoop* el, TimerWatcher* w, void* data);

class EventLoop {
public:
    enum {
        READ = 0x1,
        WRITE = 0x2
    };

    EventLoop(void* owner);
    ~EventLoop();

    void start();
    void stop();

    IOWatcher* create_io_event(io_cb_t cb, void* data);
    void start_io_event(IOWatcher* w, int fd, int mask);
    void stop_io_event(IOWatcher* w, int fd, int mask);
    void delete_io_event(IOWatcher* w);

    TimerWatcher* create_timer(timer_cb_t cb, void* data, bool need_repeated);
    void start_timer(TimerWatcher* w, unsigned int usec);
    void stop_timer(TimerWatcher* w);
    void delete_timer(TimerWatcher* w);

private:
    void* _owner;
    struct ev_loop* _loop;
};

} // namespace xrtc


#endif // __BASE_EVENT_LOOP_H_