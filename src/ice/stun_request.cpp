#include <rtc_base/helpers.h>

#include "ice/stun_request.h"
#include "rtc_base/byte_buffer.h"

namespace xrtc {

void StunRequestManager::send(StunRequest* request) {
    request->set_manager(this);
    request->construct();
    _requests[request->id()] = request;
    request->send();
}

StunRequest::StunRequest(StunMessage* msg) :
    _msg(msg)
{
    _msg->set_transaction_id(rtc::CreateRandomString(k_stun_transaction_id_length));
}

StunRequest::~StunRequest() {

}

void StunRequest::construct() {
    prepare(_msg);
}

void StunRequest::send() {
    rtc::ByteBufferWriter buf;
    if (!_msg->write(&buf)) {
        return;
    }

    _manager->signal_send_packet(this, buf.Data(), buf.Length());
}

} // namespace xrtc