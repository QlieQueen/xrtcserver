#include <cstdint>
#include <rtc_base/logging.h>
#include <rtc_base/time_utils.h>

#include "ice/ice_controller.h"
#include "ice/ice_def.h"
#include "ice/udp_port.h"

namespace xrtc {

void IceController::add_connection(IceConnection* conn) {
    _connections.push_back(conn);
}

bool IceController::has_pingable_connection() {
    for (auto conn : _connections) {
        if (_is_pingable(conn)) {
            return true;
        }
    }

    return false;
}

bool IceController::_is_pingable(IceConnection* conn) {
    // IceConnection可ping的条件
    //   1.要获取对端的ice_ufrag、ice_pwd
    const Candidate& remote = conn->remote_candidate();
    if (remote.username.empty() || remote.password.empty()) {
        RTC_LOG(LS_WARNING) << "remote ICE ufrag and pwd is empty, cannot ping.";
        return false;
    }

    //   2.Channel为weak状态
    if (_weak()) {
        return true;
    }

    return false;
}


PingResult IceController::select_connection_to_ping(int64_t _last_ping_sent_ms) {
    bool need_ping_more_at_weak = false;
    for (auto conn : _connections) {
        if (conn->num_pings_sent() < MIN_PINGS_AT_WEAK_PING_INTERVAL) {
            need_ping_more_at_weak =  true;
            break;
        }
    }

    // 1、确定ping的间隔
    int ping_interval = (_weak() || need_ping_more_at_weak) ? WEAK_PING_INTERVAL 
        : STRONG_PING_INTERVAL;

    // 2、确定ping的IceConnection
    int64_t now = rtc::TimeMillis();
    const IceConnection* conn = nullptr;
    if (now >= _last_ping_sent_ms + ping_interval) {
        conn = _find_next_pingable_connection(now);
    }

    return PingResult(conn, ping_interval);
}

const IceConnection* IceController::_find_next_pingable_connection(int64_t now) {
    if (_selected_connection && _selected_connection->writable() &&
            _is_connection_past_ping_interval(_selected_connection, now))
    {
        return _selected_connection;
    }

    return nullptr;
}

bool IceController::_is_connection_past_ping_interval(const IceConnection* conn,
        int64_t now)
{
    int interval = _get_connection_ping_interval(conn, now);
    return now >= conn->last_ping_sent() + interval;
}

int IceController::_get_connection_ping_interval(const IceConnection* conn,
        int64_t now)
{
    if (conn->num_pings_sent() < MIN_PINGS_AT_WEAK_PING_INTERVAL) {
        return WEAK_PING_INTERVAL;
    }

    if (_weak() || conn->stable(now)) {
        return STABLING_CONNECTION_PING_INTERVAL;
    }

    return STABLE_CONNECTION_PING_INTERVAL;
}


} // namespace xrtc


