#ifndef __STUN_REQUEST_H_
#define __STUN_REQUEST_H_

#include <map>

#include <rtc_base/third_party/sigslot/sigslot.h>

#include "ice/stun.h"

namespace xrtc {

class StunRequest;

class StunRequestManager {
public:
    StunRequestManager() = default;
    ~StunRequestManager() = default;

    void send(StunRequest* request);

public:
    sigslot::signal3<StunRequest*, const char*, size_t> signal_send_packet;

private:
    typedef std::map<std::string, StunRequest*> RequestMap;
    RequestMap _requests;
};

class StunRequest {
public:
    StunRequest(StunMessage* request);
    virtual ~StunRequest();

    const std::string& id() { return _msg->transaction_id(); }
    void set_manager(StunRequestManager* manager) { _manager = manager; }
    void construct();
    void send();

protected:
    virtual void prepare(StunMessage*) { }

private:
    StunMessage* _msg;
    StunRequestManager* _manager = nullptr;
};

} // namespace xrtc


#endif