#ifndef __SRTP_SESSION_H_
#define __SRTP_SESSION_H_

#include <vector>
#include <string>
#include <srtp2/srtp.h>

namespace xrtc {

class SrtpSession {
public:
    SrtpSession();
    ~SrtpSession();

    bool set_send(int cs, const uint8_t* key, size_t key_len,
            const std::vector<int>& extension_ids);

private:
    bool _set_key(int type, int cs, const uint8_t* key, size_t key_len,
        const std::vector<int>& extension_ids);
    static bool _increment_libsrtp_usage_count_and_maybe_init();
    static void _event_handle_thunk(srtp_event_data_t* ev); 
    void _handle_event(srtp_event_data_t* ev);
    bool _do_set_key(int type, int cs, const uint8_t* key, size_t key_len,
        const std::vector<int>& extension_ids);
private:
    srtp_ctx_t* _session = nullptr;
    bool _inited = false;
    int _rtp_auth_tag_len = 0;
    int _rtcp_auth_tag_len = 0;
};

}

#endif