#ifndef  __ICE_CONTROLLER_H_
#define  __ICE_CONTROLLER_H_

#include <set>

#include "ice/ice_connection.h"

namespace xrtc {

class IceTransportChannel;

struct PingResult {
    PingResult(const IceConnection* conn, int ping_interval) :
        conn(conn), ping_interval(ping_interval) {}

    const IceConnection* conn = nullptr;
    int ping_interval = 0;
};

class IceController {
public:
    IceController(IceTransportChannel* ice_channel) : _ice_channel(ice_channel) {}
    ~IceController() = default;
  
    void add_connection(IceConnection* conn);
    const std::vector<IceConnection*> connections() { return _connections; }
    bool has_pingable_connection();
    PingResult select_connection_to_ping(int64_t _last_ping_sent_ms);

private:
    bool _is_pingable(IceConnection* conn);
    const IceConnection* _find_next_pingable_connection(int64_t now);
    bool _is_connection_past_ping_interval(const IceConnection* conn, int64_t now);
    int _get_connection_ping_interval(const IceConnection* conn, int64_t now);

    bool _weak() {
        // 当channel没有选出最佳的connection或者最佳的connection处于weak状态时，channel就属于weak状态
        return _selected_connection == nullptr ||  _selected_connection->weak();
    }
 
    bool _more_pingable(IceConnection* conn1, IceConnection* conn2);

private:
    IceTransportChannel* _ice_channel;
    IceConnection* _selected_connection = nullptr;
    std::vector<IceConnection*> _connections;
    std::set<IceConnection*> _unpinged_connections;
    std::set<IceConnection*> _pinged_connections;
};

} // namespace xrtc

#endif  //__ICE_CONTROLLER_H_