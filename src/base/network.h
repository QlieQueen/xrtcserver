#ifndef __BASE_NETWORK_H_
#define __BASE_NETWORK_H_

#include <vector>
#include <rtc_base/ip_address.h>

namespace xrtc {

class Network {
public:
    Network(const std::string& name, const rtc::IPAddress& ip) :
        _name(name), _ip(ip) {}
    ~Network() = default;

    const std::string name() { return _name; }
    const rtc::IPAddress ip() { return _ip; }

    std::string to_string() {
        return _name + ":" + _ip.ToString();
    }


private:
    std::string _name;
    rtc::IPAddress _ip;
};

class NetWorkManager {
public:
    NetWorkManager();
    ~NetWorkManager();

    const std::vector<Network*>& get_networks() { return _network_lists; }
    int create_networks();

private:
    std::vector<Network*> _network_lists;

};

} // namespace xrtc

#endif // __BASE_NETWORK_H_