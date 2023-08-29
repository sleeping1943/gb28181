#include <chrono>
#include <iostream>
#include <ratio>
#include <thread>
#include "server.h"
#include "utils/helper.h"
#include <signal.h>
#include "utils/log.h"
#include "http/http_server.h"
#include "xzm_defines.h"

const static std::string kConfPath = "./conf/config.json";

void quit_server(int)
{
    Xzm::Server::is_server_quit.store(true);
    CLOGI(RED, "ready to quit server!!");
}

int main(int argc, char** argv)
{
    signal(SIGINT, quit_server);
    Xzm::Server::instance()->Test();
    std::string content;
    // 启动sip服务
    if (!Xzm::Server::instance()->Init(kConfPath)) {
        LOGE("init server error!");
        return -1;
    }
    if (!Xzm::Server::instance()->Start()) {
        LOGE("start server error!");
        return -1;
    }
    CLOGI(BLUE, "s_info_:%s", Xzm::Server::instance()->GetServerInfo().str().c_str());
    if (!Xzm::XHttpServer::instance()->Init(kConfPath)) {
        LOGE("init http_server error!");
        return -2;
    }
    // 启动http服务
    if (!Xzm::XHttpServer::instance()->Start()) {
        LOGE("start http_server error!");
        return -2;
    }
    // 等待服务退出
    unsigned int interval = 5;
    while (!Xzm::Server::is_server_quit && !Xzm::Server::is_client_all_quit) {
        CLOGI(YELLOW, "wait for server quit[%ds interval]...", interval);
        std::this_thread::sleep_for(std::chrono::milliseconds(interval * 1000));
    }
    // 关闭sip和http服务
    Xzm::Server::instance()->Stop();
    Xzm::XHttpServer::instance()->Stop();
    CLOGE(RED, "quit server gracefully!!");
    return 0;
}
