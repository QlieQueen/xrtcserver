#ifndef __SIGNALING_SERVER_H_
#define __SIGNALING_SERVER_H_

#include <string>

namespace xrtc {

struct SignalingServerOptions{
    std::string host;
    int port;
    int worker_num;
    int connection_timeout;
};

class SiganlingServer {
public:
    SiganlingServer();
    ~SiganlingServer();

    int init(const char* conf_file);

private:
    SignalingServerOptions _options;
};

} // namespace xrtc

#endif