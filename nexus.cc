#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>

#include "../include/nexus_conf.hpp"
#include "../include/nexus_utils.hpp"

int main() {
    Config& config = Config::getInstance();
    if (config.Load("../nexus.conf") == false) {
        printf("配置文件读取失败\n");
        _exit(1);
    }
    printf("%d,%s", config.getInt("ListenPort", 0), config.getString("DBInfo"));

    return 0;
}
