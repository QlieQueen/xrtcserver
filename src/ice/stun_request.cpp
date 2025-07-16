#include <rtc_base/helpers.h>

#include "ice/stun_request.h"

namespace xrtc {

void StunRequestManager::send(StunRequest* request) {
    request->construct();
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

} // namespace xrtc