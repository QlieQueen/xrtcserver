#include "base/log.h"

#include <iostream>

namespace xrtc {

XrtcLog::XrtcLog(const std::string& log_dir,
        const std::string& log_name,
        const std::string& log_level) :
    _log_dir(log_dir),
    _log_name(log_name),
    _log_level(log_level)
{
    
}

XrtcLog::~XrtcLog() {}

void XrtcLog::OnLogMessage(const std::string& message, rtc::LoggingSeverity serverity)
{
    std::cout << "==========OnLogMessage " << message;
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
    return 0;
}

void XrtcLog::set_log_to_stderr(bool on) {
    rtc::LogMessage::SetLogToStderr(on);
}

} // namespace xrtc