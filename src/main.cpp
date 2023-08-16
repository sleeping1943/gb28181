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
        if (!Xzm::util::read_file(kConfPath, content)) {
            content = "read config error!";
            std::cerr << content << std::endl;
            break;
        }
        std::cout << content << std::endl;
        Xzm::Server::instance()->SetServerInfo(content);
        std::cout << "s_info_:" << Xzm::Server::instance()->GetServerInfo().str() << std::endl;
    } while (0);
    return 0;
}
