#ifndef __BASE_SOCKET_H_
#define __BASE_SOCKET_H_

#include <sys/socket.h>

namespace xrtc {

int create_tcp_server(const char* addr, int port);

} // namespace xrtc


#endif // __BASE_SOCKET_H_