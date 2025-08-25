#include "pc/srtp_session.h"
#include "rtc_base/logging.h"
#include "srtp2/srtp.h"

namespace xrtc {

SrtpSession::SrtpSession() {

}

SrtpSession::~SrtpSession() {
    
}

bool SrtpSession::set_send(int cs, const uint8_t* key, size_t key_len,
        const std::vector<int>& extension_ids)
{
    return _set_key(ssrc_any_outbound, cs, key, key_len, extension_ids);
}

bool SrtpSession::_increment_libsrtp_usage_count_and_maybe_init() {
    return true;
}

bool SrtpSession::_set_key(int type, int cs, const uint8_t* key, size_t key_len,
        const std::vector<int>& extension_ids)
{
    if (_session) {
        RTC_LOG(LS_WARNING) << "Failed to create session: "
            << "SRTP session already created";
        return false;
    }

    if (_increment_libsrtp_usage_count_and_maybe_init()) {
        _inited = true;
    } else {
        return false;
    }

    return true;
}

} // namespace xrtc