#include <chrono>
#include <iostream>
#include <ratio>
#include <thread>
#include "server.h"
#include "utils/helper.h"
#include <signal.h>

const static std::string kConfPath = "./conf/config.json";

void quit_server(int)
{
    Xzm::Server::is_server_quit.store(true);
    std::cout << "\033[31mready to quit server!!\033[0m" << std::endl;
}

int main(int argc, char** argv)
{
    signal(SIGINT, quit_server);
    std::cout << "hello world!" << std::endl;
    Xzm::Server::instance()->Test();
    std::string content;
    if (!Xzm::Server::instance()->Init(kConfPath)) {
        std::cout << "init server error!" << std::endl;
        return -1;
    }
    if (!Xzm::Server::instance()->Start()) {
        std::cout << "start server error!" << std::endl;
        return -1;
    }
    std::cout << "s_info_:" << Xzm::Server::instance()->GetServerInfo().str() << std::endl;

    unsigned int interval = 5;
    while (!Xzm::Server::is_server_quit && !Xzm::Server::is_client_all_quit) {
        std::cout << "\033[32mwait for server quit[" << interval << "s interval]...\033[0m" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(interval * 1000));
    }
    Xzm::Server::instance()->Stop();
    std::cout << "\033[32mquit server gracefully!!\033[0m" << std::endl;
    return 0;
}
