#ifndef __BASE_EVENT_LOOP_H_
#define __BASE_EVENT_LOOP_H_

struct ev_loop;

namespace xrtc {

class EventLoop;
class IOWatcher;

typedef void (*io_cb_t)(EventLoop* el, IOWatcher* w, int fd, int events, void* data);

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

private:
    void* _owner;
    struct ev_loop* _loop;
};

} // namespace xrtc


#endif // __BASE_EVENT_LOOP_H_