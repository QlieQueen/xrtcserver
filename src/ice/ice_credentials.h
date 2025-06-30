#ifndef __ICG_CREDENTIALS_H_
#define __ICG_CREDENTIALS_H_

#include <string>

namespace xrtc {

struct IceParamters {
    IceParamters() = default;
    IceParamters(const std::string& ufrag, const std::string& pwd) :
        ice_ufrag(ufrag), ice_pwd(pwd) {}

    std::string ice_ufrag;
    std::string ice_pwd;
};

class IceCredentials {
public:
    static IceParamters create_random_ice_credentials();
};

} // namespace xrtc

#endif