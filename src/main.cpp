#include <iostream>
#include "server.h"
#include "utils/helper.h"

const static std::string kConfPath = "./conf/config.json";

int main(int argc, char** argv)
{
    std::cout << "hello world!" << std::endl;
    Xzm::Server::instance()->Test();
    std::string content;
    do {
        std::cout << "s_info_:" << Xzm::Server::instance()->GetServerInfo().str() << std::endl;
    } while (0);

    std::cout << "ready to quit server!!" << std::endl;
    return 0;
}
