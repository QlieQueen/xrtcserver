#ifndef __BASE_CONF_H_
#define __BASE_CONF_H_

#include <string>

namespace xrtc {

struct GeneralConf {
    std::string log_dir;
    std::string log_name;
    std::string log_level;
    bool log_to_stderr;
};

int load_general_conf(const char * filename, GeneralConf* conf);

}  // namespace xrtc

#endif // __BASE_CONF_H_