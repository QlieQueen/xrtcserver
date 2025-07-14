#include <rtc_base/logging.h>

#include "ice/ice_controller.h"

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
    const Candidate& remote = conn->remote_candidate();
    if (remote.username.empty() || remote.password.empty()) {
        RTC_LOG(LS_WARNING) << "remote ICE ufrag and pwd is empty, cannot ping.";
        return false;
    }

    if (_weak()) {
        return true;
    }

    return false;
}

} // namespace xrtc


