/**
 * @file server.h
 * @author sleeping (csleeping@163.com)
 * @brief 国标28181-2016 服务端
 * @version 0.1
 * @date 2023-08-14
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once
#include <iostream>
#include <sstream>
#include "utils/singleton.h"
#include <atomic>
#include <thread>
#include <unordered_map>
#include "event_handler/handler.h"
#include <boost/thread/shared_mutex.hpp>

extern "C" {
#include <eXosip2/eXosip.h>
#include <osip2/osip_mt.h>
}
#include "xzm_defines.h"

#define LOG(x) std::cout << x << std::endl;

namespace Xzm
{

class Server : public util::Singleton<Server>
{
    friend class Singleton;
private:
    Server();

public:
    ~Server();
    inline void Test()
    {
        std::cout << "server Test" << std::endl;
    }

    bool Init(const std::string& conf_path);
    bool SetServerInfo(const std::string& json_str);
    inline ServerInfo GetServerInfo() { return s_info_; }
    inline struct eXosip_t* GetSipContext() { return sip_context_; }

    bool Start();
    bool Stop();
    bool IsClientExist(const std::string& device);
    ClientPtr FindClient(const std::string& device);
    bool AddClient(ClientPtr client);
    bool UpdateClientInfo(const std::string& device_id,
     std::unordered_map<std::string, ClientInfoPtr> client_infos);
    bool RemoveClient(const std::string& device);
    void ClearClient();
    std::unordered_map<std::string, ClientPtr> GetClients();
    int AddRequest(const ClientRequestPtr req_ptr);

public:
    static HandlerPtr kDefaultHandler;


private:
    bool init_sip_server();
    /**
     * @brief sip服务处理线程函数体
     * 
     * @return true 
     * @return false 
     */
    bool run();

    bool register_event_handler();
    /**
     * @brief 处理客户端的http请求
     * 
     * @return int 
     */
    int process_request();


public:
    static std::atomic_bool is_server_quit;
    static std::atomic_bool is_client_all_quit;

private:
    std::atomic_bool is_quit_;
    ServerInfo s_info_;
    struct eXosip_t *sip_context_;
    std::thread thread_;
    std::unordered_map<eXosip_event_type, HandlerPtr> event_map_; // 注册的事件处理函数体
    std::unordered_map<std::string, ClientPtr> clients_;  // 已注册的客户端 <device_id, std::shared_ptr<Client>>

    B_Lock client_mutex_;

    RequestQueue req_queue_;
    unsigned int max_request_num = 1000;
    std::mutex req_mutex_;
};
};