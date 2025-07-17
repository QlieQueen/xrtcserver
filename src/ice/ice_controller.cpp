#include <cstdint>
#include <rtc_base/logging.h>
#include <rtc_base/time_utils.h>
#include <absl/algorithm/container.h>

#include "ice/ice_controller.h"
#include "ice/ice_def.h"
#include "ice/udp_port.h"

namespace xrtc {

const int k_min_improvement = 10; // 单位：10ms
const int a_is_better = 1;
const int b_is_better = -1;

void IceController::add_connection(IceConnection* conn) {
    _connections.push_back(conn);
    _unpinged_connections.insert(conn);
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

    bool has_pingable = false;
    for (auto conn : _unpinged_connections) {
        if (_is_pingable(conn)) {
            has_pingable = true;
            break;
        }
    }

    if (!has_pingable) {
        _unpinged_connections.insert(_pinged_connections.begin(),
            _pinged_connections.end());
        _pinged_connections.clear();
    }

    IceConnection* find_conn = nullptr;
    for (auto conn : _unpinged_connections) {
        // 优先选择最久没有发送ping请求的connection
        if (_more_pingable(conn, find_conn)) {
            find_conn = conn;
        }
    }

    return find_conn;
}

bool IceController::_more_pingable(IceConnection* conn1, IceConnection* conn2) {
    if (!conn2) {
        return true;
    }

    if (!conn1) {
        return false;
    }

    if (conn1->last_ping_sent() < conn2->last_ping_sent()) {
        return true;
    }

    if (conn1->last_ping_sent() > conn2->last_ping_sent()) {
        return false;
    }
    
    return false;
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

int IceController::_compare_connections(IceConnection* a, IceConnection* b) {
    if (a->writable() && !b->writable()) {
        return a_is_better;
    }

    if (!a->writable() && b->writable()) {
        return b_is_better;
    }

    if (a->write_state() < b->write_state()) {
        return a_is_better;
    }

    if (a->write_state() > b->write_state()) {
        return b_is_better;
    }

    if (a->receiving() && !b->receiving()) {
        return a_is_better;
    }

    if (!a->receiving() && b->receiving()) {
        return b_is_better;
    }

    if (a->priority() > b->priority()) {
        return a_is_better;
    }

    if (a->priority() < b->priority()) {
        return b_is_better;
    }

    return 0;
}

bool IceController::_ready_to_send(IceConnection* conn) {
    return conn && (conn->writable() || conn->write_state() 
                    == IceConnection::STATE_WRITE_UNRELIABLE);
}

IceConnection* IceController::sort_and_switch_connection() {
    absl::c_stable_sort(_connections, [this](IceConnection* conn1, IceConnection* conn2){
        int cmp = _compare_connections(conn1, conn2);
        if (cmp != 0) {
            return cmp > 0;
        }

        return conn1->rtt() < conn2->rtt();
    });

    RTC_LOG(LS_INFO) << "Sort " << _connections.size() << " available connections: ";
    for (auto conn : _connections) {
        RTC_LOG(LS_INFO) << conn->to_string();
    }

    IceConnection* top_connection = _connections.empty() ? nullptr : _connections[0];
    if (!_ready_to_send(top_connection) || _selected_connection == top_connection) {
        return nullptr;
    }

    // 未选出selected_connection
    if (!_selected_connection)  {
        return top_connection;
    }

    if (top_connection->rtt() <= _selected_connection->rtt() - k_min_improvement) {
        return top_connection;
    }

    return nullptr;
}


} // namespace xrtc


