#ifndef __ICE_CONNECTION_H_
#define __ICE_CONNECTION_H_

#include "base/event_loop.h"
#include "ice/candidate.h"
#include "ice/ice_credentials.h"
#include "ice/stun.h"
#include "ice/stun_request.h"
#include "ice/udp_port.h"

namespace xrtc {

class UDPPort;
class IceConnection;

class ConnectionRequest : public StunRequest {
public:
    ConnectionRequest(IceConnection* conn);

protected:
    void prepare(StunMessage* msg) override;
    void on_response(StunMessage* msg) override;
    void on_error_response(StunMessage* msg) override;

private:
    IceConnection* _connection;
};

class IceConnection : public sigslot::has_slots<> {
public:
    enum WriteState {
        STATE_WRITABLE = 0, // 最近发送的ping都能收到相应
        STATE_WRITE_UNRELIABLE = 1, 
        STATE_WRITE_INIT = 2, // 初始状态
        STATE_WRITE_TIMEOUT = 3,
    };

    struct SentPing {
        SentPing(const std::string& id, int64_t ts) :
            id(id), sent_time(ts) {}
        std::string id;
        int64_t sent_time;
    };

    IceConnection(EventLoop* el, UDPPort* port, const Candidate& remote_candidate);
    ~IceConnection();

    const Candidate& remote_candidate() const { return _remote_candidate; }
    const Candidate& local_candidate() const;
    UDPPort* port() { return _port; }

    void handle_stun_binding_request(StunMessage* stun_msg);
    void send_stun_binding_response(StunMessage* stun_msg);
    void send_response_message(const StunMessage& response);
    void on_read_packet(const char* buf, size_t len, int64_t ts);
    void on_connection_response(ConnectionRequest* request, StunMessage* msg);
    void on_connection_error_response(ConnectionRequest* request, StunMessage* msg);
    void maybe_set_remote_ice_params(const IceParamters& ice_params);

    bool writable() { return _write_state == STATE_WRITABLE; }
    bool receiving() { return _receiving; }
    // 当IceConnection处于不可写或者不可读的状态，即为weak状态
    bool weak() { return !(writable() && receiving()); }
    bool active() { return _write_state != STATE_WRITE_TIMEOUT; }
    bool stable(int64_t now) const;
    void ping(int64_t now);

    int64_t last_ping_sent() const { return _last_ping_sent; }
    int num_pings_sent() const { return _nums_pings_sent; }

    std::string to_string();

private:
    void _on_stun_send_packet(StunRequest* request, const char* buf, size_t len);

private:
    EventLoop* _el;
    UDPPort* _port;
    Candidate _remote_candidate;

    WriteState _write_state = STATE_WRITE_INIT;
    bool _receiving = false; // 可读状态只有两种

    int64_t _last_ping_sent = 0;
    int _nums_pings_sent = 0;
    std::vector<SentPing> _pings_since_last_responses;
    StunRequestManager _request_manager;
};

}

#endif