#ifndef __PEER_CONNECTION_H_
#define __PEER_CONNECTION_H_

#include <memory>
#include <string>
#include "base/event_loop.h"
#include "pc/session_description.h"


namespace xrtc {

struct RTCOfferAnswerOptions {
    bool send_audio = true;
    bool send_video = true;
    bool recv_audio = true;
    bool recv_video = true;
    bool use_rtp_mux = true; // bundle
    bool use_rtcp_mux = true;
};

class PeerConnection {
public:
    PeerConnection(EventLoop* el);
    ~PeerConnection();

    std::string create_offer(const RTCOfferAnswerOptions& options);

private:
    EventLoop* _el;

    std::unique_ptr<SessionDescription> _local_desc;
};

} // namespace xrtc


#endif