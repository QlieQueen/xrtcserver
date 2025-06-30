#include <rtc_base/helpers.h>

#include "ice/ice_def.h"
#include "ice/ice_credentials.h"


namespace xrtc {

IceParamters IceCredentials::create_random_ice_credentials() {
    return IceParamters(rtc::CreateRandomString(ICE_UFRAG_LENGTH),
            rtc::CreateRandomString(ICE_PWD_LENGTH));
}

} // namespace xrtc


