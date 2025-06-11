#ifndef __BASE_LOG_H_
#define __BASE_LOG_H_

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

    void OnLogMessage(const std::string& message, rtc::LoggingSeverity serverity) override;
    void OnLogMessage(const std::string& message) override;

private:
    std::string _log_dir;
    std::string _log_name;
    std::string _log_level;
};


} // namespace xrtc

#endif