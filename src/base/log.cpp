#include <iostream>

#include <errno.h>
#include <sys/stat.h>

#include "base/log.h"

namespace xrtc {

XrtcLog::XrtcLog(const std::string& log_dir,
        const std::string& log_name,
        const std::string& log_level) :
    _log_dir(log_dir),
    _log_name(log_name),
    _log_level(log_level),
    _log_file(log_dir + "/" + log_name + ".log"),
    _log_file_wf(log_dir + "/" + log_name + ".log.wf")
{
    
}

XrtcLog::~XrtcLog() {
    stop();

    _out_file.close();
    _out_file_wf.close();
}

void XrtcLog::OnLogMessage(const std::string& message, rtc::LoggingSeverity serverity)
{
    if (serverity >= rtc::LS_WARNING) {
        std::unique_lock<std::mutex> lock(_mtx_wf);
        _log_queue_wf.push(message);
    } else {
        std::unique_lock<std::mutex> lock(_mtx);
        _log_queue.push(message);
    }
}

void XrtcLog::OnLogMessage(const std::string& /*message*/) 
{
    // 不需要有逻辑
}

static rtc::LoggingSeverity get_log_serverity(const std::string& level) {
    if ("verbose" == level) {
        return rtc::LS_VERBOSE;
    } else if ("info" == level) {
        return rtc::LS_INFO;
    } else if ("warning" == level) {
        return rtc::LS_WARNING;
    } else if ("error" == level) {
        return rtc::LS_ERROR;
    } else if ("none" == level) {
        return rtc::LS_NONE;
    }
    return rtc::LS_NONE;
}

int XrtcLog::init() {
    rtc::LogMessage::ConfigureLogging("thread tstamp");
    rtc::LogMessage::SetLogPathPrefix("/src");
    rtc::LogMessage::AddLogToStream(this, get_log_serverity(_log_level));

    int ret = mkdir(_log_dir.c_str(), 0755);
    if (ret != 0 && errno != EEXIST) {
        fprintf(stderr, "create log_dir[%s] failed\n", _log_dir.c_str());
        return -1;
    }

    // 打开文件
    _out_file.open(_log_file, std::ios::app);
    if (!_out_file.is_open()) {
        fprintf(stderr, "open log_file[%s] failed\n", _log_file.c_str());
        return -1;
    }

    _out_file_wf.open(_log_file_wf, std::ios::app);
    if (!_out_file_wf.is_open()) {
        fprintf(stderr, "open log_file_wf[%s] failed\n", _log_file_wf.c_str());
        return -1;
    }

    return 0;
}

bool XrtcLog::start() {
    if (_running) {
        fprintf(stderr, "log thread already running\n");
        return false;
    }

    _running = true;

    _thread = new std::thread([=]() {
        struct stat stat_data;
        static int counter = 0;
        while (_running) {
            // 检查日志文件是否被删除或者移动
            if (++counter == 100) {
                counter = 0;
                if (stat(_log_file.c_str(), &stat_data) < 0) {
                    _out_file.close();
                    _out_file.open(_log_file, std::ios::app);
                }

                if (stat(_log_file_wf.c_str(), &stat_data) < 0) {
                    _out_file_wf.close();
                    _out_file_wf.open(_log_file_wf, std::ios::app);
                }
            }

            write_log_to_log_file(_out_file, _log_queue, _mtx);
            write_log_to_log_file(_out_file_wf, _log_queue_wf, _mtx_wf);

            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
    });

    return true;
}

void XrtcLog::stop() {
    _running = false;

    if (_thread) {
        if (_thread->joinable()) {
            _thread->join();
        }

        delete _thread;
        _thread = nullptr;
    }
}

void XrtcLog::join() {
    if (_thread && _thread->joinable()) {
        _thread->join();
    }
}

void XrtcLog::write_log_to_log_file(std::ofstream& log_file,
    std::queue<std::string>& queue,
    std::mutex& mutex)
{
    std::queue<std::string> tmp;
    {
        std::unique_lock<std::mutex> lock(mutex);
        if (!queue.empty()) {
            tmp = std::move(queue);
        }
    }
    while (!tmp.empty()) {
        log_file << tmp.front();
        tmp.pop();
    }
    log_file.flush();
}

void XrtcLog::set_log_to_stderr(bool on) {
    rtc::LogMessage::SetLogToStderr(on);
}

} // namespace xrtc