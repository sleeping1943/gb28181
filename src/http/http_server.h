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

#define GERNERATE_ERROR_INFO(err_code, msg) \
    "{\"code\":err_code, msg:""}"
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
    /**
     * @brief 查询已注册设备信息
     * 
     * @param req 
     * @param resp 
     * @return int 
     */
    int query_device_list(HttpRequest* req, HttpResponse* resp);
    /**
     * @brief 请求指定设备开启推流
     * 
     * @param req 
     * @param resp 
     * @return int 
     */
    int start_rtsp_publish(HttpRequest* req, HttpResponse* resp);
    /**
     * @brief 使指定设备结束推流
     * 
     * @param req 
     * @param resp 
     * @return int 
     */
    int stop_rtsp_publish(HttpRequest* req, HttpResponse* resp);
    /**
     * @brief 指定设备开始对讲
     * 
     * @param req 
     * @param resp 
     * @return int 
     */
    int start_invite_talk(HttpRequest* req, HttpResponse* resp);
    /**
     * @brief 指定设备结束对讲
     * 
     * @param req 
     * @param resp 
     * @return int 
     */
    int stop_talk(HttpRequest* req, HttpResponse* resp);

    /**
     * @brief 广播对讲消息
     * 
     * @param req 
     * @param resp 
     * @return int 
     */
    int start_talk_broadcast(HttpRequest* req, HttpResponse* resp);

    /**
     * @brief 国标设备开始推送的回调及鉴权函数
     * 
     * @param req 
     * @param resp 
     * @return int 
     */
    int on_publish(HttpRequest* req, HttpResponse* resp);
    /**
     * @brief 客户端请求播放流的回调及鉴权函数
     * 
     * @param req 
     * @param resp 
     * @return int 
     */
    int on_play(HttpRequest* req, HttpResponse* resp);

    inline std::string get_simple_info(int code, const std::string& msg)
    {
        std::stringstream ss;
        ss << "{"
        << "\"code\":" << code << ","
        << "\"msg\":" << msg
        << "}";
        return ss.str();
    }

private:
    hv::HttpService router;
    std::thread thread_;
    std::atomic_bool is_quit_;
    HttpServerInfo s_info_;
    http_server_t server_;
};
};