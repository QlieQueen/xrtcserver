#ifndef __SRTP_TRANSPORT_H_
#define __SRTP_TRANSPORT_H_

namespace xrtc {

class SrtpTransport {
public:
    SrtpTransport(bool rtcp_mux_enabled);
    virtual ~SrtpTransport() = default;

private:
    bool _rtcp_mux_enabled;
};

}

#endif // _SRTP_TRANSPORT_H_