#include <iostream>

#include "base/conf.h"

xrtc::GeneralConf* g_conf = nullptr;

int init_general_conf(const char* filename) {
    if (!filename) {
        fprintf(stderr, "filename is nullptr\n");
        return -1;
    }

    g_conf = new xrtc::GeneralConf();

    int ret = xrtc::load_general_conf(filename, g_conf);
    if (ret != 0) {
        fprintf(stderr, "load %s config file failed\n", filename);
        return -1;
    }

    return 0;
}

int main() {
    std::cout << "hello world" << std::endl;

    int ret = init_general_conf("./conf/general.yaml");
    if (ret != 0) {
        fprintf(stderr, "init config file failed\n");
        return -1;
    }

    return 0;
}