#include <unistd.h>

#include <rtc_base/zmalloc.h>
#include <rtc_base/logging.h>

#include "base/xhead.h"
#include "xrtc_server_def.h"
#include "base/socket.h"
#include "server/rtc_server.h"
#include "server/tcp_connection.h"
#include "server/signaling_worker.h"

extern xrtc::RtcServer* g_rtc_server;

namespace xrtc {

void signaling_worker_recv_notify(EventLoop* /*el*/, IOWatcher* /*w*/, int fd,
    int /*events*/, void *data)
{
    int msg;
    if (read(fd, &msg, sizeof(int)) != sizeof(int)) {
        RTC_LOG(LS_WARNING) << "read from pipe error: " << strerror(errno)
            << ", errno: " << errno;
        return;    
    }

    SignalingWorker* worker = (SignalingWorker*)data;
    worker->_process_notify(msg);
}

SignalingWorker::SignalingWorker(int worker_id, const SignalingServerOptions& options) :
    _worker_id(worker_id),
    _options(options),
    _el(new EventLoop(this))
{
}

SignalingWorker::~SignalingWorker() {
    for (auto c : _conns) {
        if (c) {
            _close_conn(c);
        }
    }

    _conns.clear();

    if (_el) {
        delete _el;
        _el = nullptr;
    }

    if (_thread) {
        delete _thread;
        _thread = nullptr;
    }
}

int SignalingWorker::init() {
    int fds[2];
    if (pipe(fds)) {
        RTC_LOG(LS_WARNING) << "create pipe error: " << strerror(errno)
            << ", errno: " << errno;
        return -1;
    }

    _notify_recv_fd = fds[0];
    _notify_send_fd = fds[1];

    _pipe_watcher = _el->create_io_event(signaling_worker_recv_notify, this);
    _el->start_io_event(_pipe_watcher, _notify_recv_fd, EventLoop::READ);

    return 0;
}

bool SignalingWorker::start() {
    if (_thread) {
        RTC_LOG(LS_WARNING) << "signaling worker already start, worker_d:" << _worker_id;
        return false;
    }    

    _thread = new std::thread([=]() {
        RTC_LOG(LS_INFO) << "signaling worker event loop start, worker_id: " << _worker_id;
        _el->start();
        RTC_LOG(LS_INFO) << "signaling worker event loop stop, worker_id: " << _worker_id;
    });

    return true;
}

void SignalingWorker::stop() {
    notify(SignalingWorker::QUIT);
}

int SignalingWorker::notify(int msg) {
    int written = write(_notify_send_fd, &msg, sizeof(int));
    return written == sizeof(int) ? 0 : -1;
}

void SignalingWorker::join() {
    if (_thread && _thread->joinable()) {
        _thread->join();
    }
}

void SignalingWorker::_stop() {
    if (!_thread) {
        RTC_LOG(LS_WARNING) << "signaling worker not running, worker_id: " << _worker_id;
        return;
    }

    _el->delete_io_event(_pipe_watcher);
    _el->stop();

    close(_notify_recv_fd);
    close(_notify_send_fd);
}

void conn_io_cb(EventLoop* /*el*/, IOWatcher* /*w*/, int fd, int events, void* data) {
    SignalingWorker* worker = (SignalingWorker*)data;

    if (events & EventLoop::READ) {
        worker->_read_query(fd);
    }

    if (events & EventLoop::WRITE) {
        worker->_write_reply(fd);
    }
}

void conn_timer_cb(EventLoop* el, TimerWatcher* /*w*/, void* data) {
    SignalingWorker* worker = (SignalingWorker*)(el->owner());
    TcpConnection* c = (TcpConnection*)data;
    worker->_process_timeout(c);

}

void SignalingWorker::_write_reply(int fd) {
    if (fd <= 0 || (size_t)(fd) >= _conns.size()) {
        return;
    }

    TcpConnection* c = _conns[fd];
    if (!c) {
        return;
    }

    while (!c->reply_list.empty()) {
        rtc::Slice reply = c->reply_list.front();
        int nwritten = sock_write_data(c->fd, reply.data() + c->cur_resp_pos, 
            reply.size() - c->cur_resp_pos);
        if (-1 == nwritten) {
            _close_conn(c);
            return;
        } else if (0 == nwritten) {
            RTC_LOG(LS_WARNING) << "write zero bytes, fd: " << c->fd
                << ", worker_id: " << _worker_id;
        } else if ((nwritten + c->cur_resp_pos) >= reply.size()) {
            // 写入完成
            c->reply_list.pop_front();
            zfree((void*)reply.data());
            c->cur_resp_pos = 0;
            RTC_LOG(LS_INFO) << "write finish, fd: " << c->fd
                << ", worker_id: " << _worker_id;
        } else {
            c->cur_resp_pos += nwritten;
        }
    }
    c->last_interaction = _el->now();
    if (c->reply_list.empty()) {
        _el->stop_io_event(c->io_watcher, c->fd, EventLoop::WRITE);
        RTC_LOG(LS_INFO) << "stop write event, fd: " << c->fd
            << ", worker_id: " << _worker_id;
    }
}

void SignalingWorker::_process_timeout(TcpConnection* c) {
    if (_el->now() - c->last_interaction >= (unsigned long)(_options.connection_timeout)) {
        RTC_LOG(LS_INFO) << "connection timeout, fd: " << c->fd;
        _close_conn(c);
    } 

}

void SignalingWorker::_new_conn(int fd) {
    RTC_LOG(LS_INFO) << "signaling worker " << _worker_id << ", receive fd: " << fd;

    if (fd <= 0) {
        RTC_LOG(WARNING) << "invalid fd: " << fd;
        return;
    }

    sock_setnonblock(fd);
    sock_setnodelay(fd);

    TcpConnection* c = new TcpConnection(fd);
    sock_peer_to_str(fd, c->ip, &(c->port));
    c->io_watcher = _el->create_io_event(conn_io_cb, this);
    _el->start_io_event(c->io_watcher, fd, EventLoop::READ);

    c->timer_watcher = _el->create_timer(conn_timer_cb, c, true);
    _el->start_timer(c->timer_watcher, 100000); // 100ms

    c->last_interaction = _el->now();

    if ((size_t)fd > _conns.size()) {
        _conns.resize(fd * 2);
    }

    _conns[fd] = c;

}

void SignalingWorker::_read_query(int fd) {
    RTC_LOG(LS_INFO) << "signaling worker " << _worker_id
        << " receive read event, fd: " << fd;
    
    if (fd < 0 || (size_t)fd >= _conns.size()) {
        RTC_LOG(LS_WARNING) << "invalid fd: " << fd;
        return;
    }

    TcpConnection* c = _conns[fd];
    int nread = 0;
    int read_len = c->bytes_expected;
    int qb_len = sdslen(c->querybuf);
    c->querybuf = sdsMakeRoomFor(c->querybuf, read_len);
    nread = sock_read_data(fd, c->querybuf + qb_len, read_len);
    
    c->last_interaction = _el->now();

    RTC_LOG(LS_INFO) << "sock read data, len: " << nread;
    
    if (-1 == nread) {
        _close_conn(c);
        return;
    } else if (nread > 0) {
        sdsIncrLen(c->querybuf, nread);  
    }

    int ret = _process_query_buffer(c);
    if (ret != 0) {
        _close_conn(c);
        return;
    }

}

void SignalingWorker::_close_conn(TcpConnection* c) {
    RTC_LOG(INFO) << "close connection, fd: " << c->fd;
    close(c->fd);
    _remove_conn(c);
}

void SignalingWorker::_remove_conn(TcpConnection* c) {
    _el->delete_timer(c->timer_watcher);
    _el->delete_io_event(c->io_watcher);
    _conns[c->fd] = nullptr;
    delete c;
}

int SignalingWorker::_process_query_buffer(TcpConnection* c) {
    while (sdslen(c->querybuf) >= c->bytes_processed + c->bytes_expected) {
        xhead_t* head = (xhead_t*)(c->querybuf);
        if (TcpConnection::STATE_HEAD == c->current_state) {
            if (XHEAD_MAGIC_NUM != head->magic_num) {
                RTC_LOG(LS_WARNING) << "invalid data, fd: " << c->fd;
                return -1;
            }
            c->current_state = TcpConnection::STATE_BODY;
            c->bytes_processed = XHEAD_SIZE;
            c->bytes_expected = head->body_len;
        } else {
            rtc::Slice header(c->querybuf, XHEAD_SIZE);
            rtc::Slice body(c->querybuf + XHEAD_SIZE, head->body_len);

            int ret = _process_request(c, header, body);
            if (ret != 0) {
                return -1;
            }

            // 短链接处理
            c->bytes_processed = 65535;
        }
    }

    return 0;
}

int SignalingWorker::_process_request(TcpConnection* c,
    const rtc::Slice& header,
    const rtc::Slice& body)
{
    RTC_LOG(LS_INFO) << "receive body: " << body.data();

    xhead_t* xh = (xhead_t*)(header.data());

    int ret = 0;
    int cmdno;
    json root;
    try {
        root = json::parse(body.data());
        cmdno = root["cmdno"];

        RTC_LOG(LS_INFO) << "cmdno: " << cmdno;
    } catch (json::parse_error& e) {
        RTC_LOG(LS_WARNING) << "parse error: " << e.what()
            << ", pos: " << e.byte 
            << ", fd: " << c->fd
            << ", log_id: " << xh->log_id;
        return -1;
    }

    
    switch(cmdno) {
        case CMDNO_PUSH:
            return _process_push(cmdno, c, root, xh->log_id);
            break;

        case CMDNO_ANSWER:
            ret = _process_answer(cmdno, c, root, xh->log_id);
            break;

        default:
            ret = -1;
            RTC_LOG(LS_WARNING) << "unknown cmdno: " << cmdno << ", log_id: " << xh->log_id;
            break;
    }

    // 返回处理结果
    char* buf = (char*)zmalloc(XHEAD_SIZE + MAX_RES_BUF);
    xhead_t* res_xh = (xhead_t*)buf;
    memcpy(res_xh, header.data(), header.size());

    json res_root;
    if (0 ==ret) {
        res_root["err_no"] = 0;
        res_root["err_msg"] = "success";
    } else {
        res_root["err_no"] = -1;
        res_root["err_msg"] = "process error";
    }

    std::string json_data = res_root.dump();
    RTC_LOG(LS_INFO) << "response body: " << json_data;

    res_xh->body_len = json_data.size();
    snprintf(buf + XHEAD_SIZE, MAX_RES_BUF, "%s", json_data.c_str());

    rtc::Slice reply(buf, XHEAD_SIZE + res_xh->body_len);
    _add_reply(c, reply);

    return 0;
}

int SignalingWorker::_process_push(int cmdno, TcpConnection* c, 
    const json& root, uint32_t log_id) 
{
    uint64_t uid;
    std::string stream_name;
    int audio;
    int video;

    try {
        uid = root.at("uid");
        stream_name = root.at("stream_name");
        audio = root.at("audio");
        video = root.at("video");

    } catch (const json::out_of_range& e) {
        RTC_LOG(LS_WARNING) << "parse error: " << e.what()
            << ", fd: " << c->fd
            << ", log_id: " << log_id;
        return -1;
    }

    RTC_LOG(LS_INFO) << "cmdno[" << cmdno
        << "] uid[" << uid
        << "] stream_name[" << stream_name
        << "] audio[" << audio
        << "] video[" << video
        << "] signaling server push request";

    std::shared_ptr<RtcMsg> msg = std::make_shared<RtcMsg>();
    msg->cmdno = cmdno;
    msg->uid = uid;
    msg->stream_name = stream_name;
    msg->audio = audio;
    msg->video = video;
    msg->log_id = log_id;
    msg->worker = this;
    msg->conn = c;
    msg->fd = c->fd;

    return g_rtc_server->send_rtc_msg(msg);
}


int SignalingWorker::_process_answer(int cmdno, TcpConnection* c, 
    const json& root, uint32_t log_id) 
{
    uint64_t uid;
    std::string stream_name;
    std::string answer;
    std::string stream_type;

    try {
        uid = root.at("uid");
        stream_name = root.at("stream_name");
        answer = root.at("answer");
        stream_type = root.at("type");

    } catch (const json::out_of_range& e) {
        RTC_LOG(LS_WARNING) << "parse error: " << e.what()
            << ", fd: " << c->fd
            << ", log_id: " << log_id;
        return -1;
    }

    RTC_LOG(LS_INFO) << "cmdno[" << cmdno
        << "] uid[" << uid
        << "] stream_name[" << stream_name
        << "] answer[" << answer
        << "] stream_type[" << stream_type
        << "] signaling server send answer request";

    std::shared_ptr<RtcMsg> msg = std::make_shared<RtcMsg>();
    msg->cmdno = cmdno;
    msg->uid = uid;
    msg->stream_name = stream_name;
    msg->sdp = answer;
    msg->stream_type = stream_type;
    msg->log_id = log_id;

    return g_rtc_server->send_rtc_msg(msg);
}




void SignalingWorker::_response_server_offer(std::shared_ptr<RtcMsg> msg) {
    TcpConnection* c = (TcpConnection*)msg->conn;
    if (!c) {
        return;
    }

    // 此处使用msg中的fd而不是使用msg->conn->fd在SignalingWorkers中的_conns查找
    // 是因为msg->conn可能已经因为fd超时断开或者客户端主动断开而变成野指针.
    int fd = msg->fd;
    if (fd < 0 || (size_t)fd >= _conns.size()) {
        return;
    }

    // 因为查找到的TcpConnection可能是新连接，所以需要跟msg中的conn进行比较, 确认
    // 是否是原本的连接
    if (_conns[fd] != c) {
        return;
    }

    // 构造响应头
    xhead_t* xh = (xhead_t*)(c->querybuf);
    rtc::Slice header(c->querybuf, XHEAD_SIZE);
    char* buf = (char*)zmalloc(XHEAD_SIZE + MAX_RES_BUF);
    if (!buf) {
        RTC_LOG(LS_WARNING) << "zmalloc error, log_id: " << xh->log_id;
        return;
    }

    // 构造response的头部
    // 复用request的头部（只需要在后续修改body_size）
    memcpy(buf, header.data(), header.size());
    xhead_t* res_xh = (xhead_t*)buf;

    json res_root;
    res_root["err_no"] = msg->err_no;
    if (msg->err_no != 0) {
        res_root["err_msg"] = "process error";
        res_root["offer"] = "";
    } else {
        res_root["err_msg"] = "success";
        res_root["offer"] = msg->sdp;
    }

    std::string json_data = res_root.dump();
    RTC_LOG(LS_INFO) << "response body: " << json_data;

    res_xh->body_len = json_data.size();
    // 构造response的body
    snprintf(buf + XHEAD_SIZE, MAX_RES_BUF, "%s", json_data.c_str());

    rtc::Slice reply(buf, XHEAD_SIZE + res_xh->body_len);
    _add_reply(c, reply);

}

void SignalingWorker::_add_reply(TcpConnection* c, const rtc::Slice& reply) {
    c->reply_list.push_back(reply);
    _el->start_io_event(c->io_watcher, c->fd, EventLoop::WRITE);
}

void SignalingWorker::_process_rtc_msg() {
    std::shared_ptr<RtcMsg> msg = pop_msg();
    if (!msg) {
        return;
    }

    switch (msg->cmdno)
    {
    case CMDNO_PUSH:
        _response_server_offer(msg);
        break;
    
    default:
        RTC_LOG(LS_WARNING) << "unknown cmdno: " << msg->cmdno << ", log_id: " << msg->log_id;
        break;
    }
}

void SignalingWorker::_process_notify(int msg) {
    switch(msg) {
        case QUIT:
            _stop();
            break;
        case NEW_CONN:
            int fd;
            if (_q_conn.consume(&fd)) {
                _new_conn(fd);
            }
            break;

        case RTC_MSG:
            _process_rtc_msg();
            break;

        default:
            RTC_LOG(LS_WARNING) << "unknown msg: " << msg;
            break;
    }
}

int SignalingWorker::notify_new_conn(int fd) {
    _q_conn.produce(fd);
    return notify(SignalingWorker::NEW_CONN);
}


void SignalingWorker::push_msg(std::shared_ptr<RtcMsg> msg) {
    std::unique_lock<std::mutex> lock(_q_msg_mtx);
    _q_msg.push(msg);
}


std::shared_ptr<RtcMsg> SignalingWorker::pop_msg() {
    std::unique_lock<std::mutex> lock(_q_msg_mtx);
    if (_q_msg.empty()) {
        return nullptr;
    }

    std::shared_ptr<RtcMsg> msg = _q_msg.front();
    _q_msg.pop();
    return msg;
}

int SignalingWorker::send_rtc_msg(std::shared_ptr<RtcMsg> msg) {
    push_msg(msg);
    return notify(RTC_MSG);
}

}