#include <ifaddrs.h>

#include <rtc_base/logging.h>

#include "base/network.h"
#include "rtc_base/ip_address.h"

namespace xrtc {

NetWorkManager::NetWorkManager() = default;

NetWorkManager::~NetWorkManager() {
    for (auto network : _network_lists) {
        delete network;
    }
    _network_lists.clear();
}

int NetWorkManager::create_networks() {
//因为云服务器没有公网IP对应的网卡，所以getifaddrs无法获取公网IP，
//目前直接强制写死，后续可以部署stun或turn服务，实现公网IP获取。
#define CANT_GET_FROM_IFADDRS
#ifndef CANT_GET_FROM_IFADDRS
    struct ifaddrs* interface;
    int err = getifaddrs(&interface);
    if (err != 0) {
        RTC_LOG(LS_WARNING) << "getifaddrs error: " << strerror(errno) 
            << ", errno: " << errno;
        return -1;
    }

    for (auto cur = interface; cur != NULL; cur = cur->ifa_next) {
        if (cur->ifa_addr->sa_family != AF_INET) continue; 

        struct sockaddr_in* addr = (struct sockaddr_in*)(cur->ifa_addr);
        rtc::IPAddress ip_address(addr->sin_addr);

        if (rtc::IPIsLoopback(ip_address)) {
            continue;
        }

        Network* network = new Network(cur->ifa_name, ip_address);

        RTC_LOG(LS_INFO) << "gathered network interface: " << network->to_string();

        _network_lists.push_back(network);

    }

    freeifaddrs(interface);
#else
    rtc::IPAddress ip_address(htonl(inet_addr("120.76.197.143")));
    Network* network = new Network("eth0", ip_address);
    RTC_LOG(LS_INFO) << "gathered network interface: " << network->to_string();

    _network_lists.push_back(network);
#endif
#undef CANT_GET_FROM_IFADDRS
    return 0;
}


} // namespace xrtc