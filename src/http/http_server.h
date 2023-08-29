/**
 * @file http_server.h
 * @author sleeping (csleeping@163.com)
 * @brief 一个简单的基于libhv的http服务器
 * @version 0.1
 * @date 2023-08-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once
#include "../xzm_defines.h"
#include "../utils/singleton.h"
#include "hv/HttpServer.h"
#include <thread>
#include <atomic>

namespace Xzm
{
class XHttpServer : public util::Singleton<XHttpServer>
{
    friend class Singleton;
public:
    ~XHttpServer();
    bool Init(const std::string& conf_path);
    bool Start();
    bool Stop();
    bool Run();

private:
    XHttpServer();
    int query_device_list(HttpRequest* req, HttpResponse* resp);


private:
    hv::HttpService router;
    std::thread thread_;
    std::atomic_bool is_quit_;
    HttpServerInfo s_info_;
    http_server_t server_;
};
};