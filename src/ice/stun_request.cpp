#include "ice/stun_request.h"
#include "ice/stun.h"

namespace xrtc {

void StunRequestManager::send(StunRequest* request) {
    request->construct();
}

StunRequest::StunRequest(StunMessage* msg) :
    _msg(msg)
{

}

StunRequest::~StunRequest() {

}

void StunRequest::construct() {
    prepare(_msg);
}

} // namespace xrtc