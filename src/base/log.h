#ifndef __BASE_LOG_H_
#define __BASE_LOG_H_

#include <queue>
#include <mutex>
#include <thread>
#include <fstream>

#include <rtc_base/logging.h>

namespace xrtc {

class XrtcLog : public rtc::LogSink {
public:
    XrtcLog(const std::string& log_dir,
            const std::string& log_name,
            const std::string& log_level);
    ~XrtcLog() override;

    int init();
    void set_log_to_stderr(bool on);
    bool start();
    void stop();
    void join();

    void OnLogMessage(const std::string& message, rtc::LoggingSeverity serverity) override;
    void OnLogMessage(const std::string& message) override;

private:
    static void write_log_to_log_file(std::ofstream& log_file,
        std::queue<std::string>& queue, std::mutex& mutex);

private:
    std::string             _log_dir;
    std::string             _log_name;
    std::string             _log_level;
    std::string             _log_file;
    std::string             _log_file_wf;

    std::ofstream           _out_file;         // 正常log的写入文件
    std::ofstream           _out_file_wf;      // 错误log的写入文件

    std::queue<std::string> _log_queue;
    std::mutex              _mtx;

    std::queue<std::string> _log_queue_wf;
    std::mutex              _mtx_wf;

    std::thread*            _thread = nullptr;
    std::atomic<bool>       _running{false};
};


} // namespace xrtc

#endif