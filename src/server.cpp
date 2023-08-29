#include "server.h"
#include "include/rapidjson/document.h"
#include "utils/json_helper.h"
#include "utils/helper.h"
#include "utils/log.h"

#include <arpa/inet.h>
#include <functional>
#include <mutex>
#include "event_handler/register_handler.h"
#include "event_handler/call_answer_handler.h"



namespace Xzm
{

std::atomic_bool Server::is_server_quit(false);
std::atomic_bool Server::is_client_all_quit(false);

Server::Server():is_quit_(false)
{

}

Server::~Server()
{
    if (thread_.joinable()) {
        thread_.join();
    }
}

bool Server::Init(const std::string& conf_path)
{
    std::string content;
    if (!Xzm::util::read_file(conf_path, content)) {
        content = "read config error!";
        std::cerr << content << std::endl;
        return false;
    }
    std::cout << content << std::endl;
    if (!Xzm::Server::instance()->SetServerInfo(content)) {
        std::cerr << "json parse error!" << std::endl;
        return false;
    }
    return true;
}

bool Server::SetServerInfo(const std::string& json_str)
{
    rapidjson::Document doc;
    JSON_PARSE_BOOL(doc, json_str.c_str());

    if (!doc.HasMember("sip_config") || !doc["sip_config"].IsObject()) {
        return false;
    }
    auto& sip_config = doc["sip_config"];
    JSON_VALUE_REQUIRE_STRING(sip_config, "ua", s_info_.ua);
    JSON_VALUE_REQUIRE_STRING(sip_config, "nonce", s_info_.nonce);
    JSON_VALUE_REQUIRE_STRING(sip_config, "ip", s_info_.ip);
    JSON_VALUE_REQUIRE_INT(sip_config, "port", s_info_.port);
    JSON_VALUE_REQUIRE_STRING(sip_config, "rtp_ip", s_info_.rtp_ip);
    JSON_VALUE_REQUIRE_INT(sip_config, "rtpPort", s_info_.rtp_port);
    JSON_VALUE_REQUIRE_STRING(sip_config, "sipId", s_info_.sip_id);
    JSON_VALUE_REQUIRE_STRING(sip_config, "sipRealm", s_info_.realm);
    JSON_VALUE_REQUIRE_STRING(sip_config, "sipPass", s_info_.passwd);
    JSON_VALUE_REQUIRE_INT(sip_config, "sipTimeout", s_info_.timeout);
    JSON_VALUE_REQUIRE_INT(sip_config, "sipExpiry", s_info_.valid_time);
    return true;
}

bool Server::init_sip_server()
{
    register_event_handler();
    /* 
    step 1 
    申请结构体内存
    */
    sip_context_ = eXosip_malloc();
    if (!sip_context_) {
        LOG("eXosip_malloc error!");
        return false;
    }
    /*
     step 2
     初始化
    */
    if (eXosip_init(sip_context_)) {
        LOG("eXosip_init error");
        return false;
    }
    /*
      step 3
      开始监听
     */
    if (eXosip_listen_addr(sip_context_, IPPROTO_UDP, nullptr, s_info_.port, AF_INET, 0)) {
        LOG("eXosip_listen_addr error");
        return false;
    }

     eXosip_set_user_agent(sip_context_, s_info_.ua.c_str());
     /**
      step 4
      添加授权信息
      */
    if (eXosip_add_authentication_info(sip_context_, s_info_.sip_id.c_str(),
       s_info_.sip_id.c_str(), s_info_.passwd.c_str(), nullptr, s_info_.realm.c_str())) {
        LOG("eXosip_add_authentication_info error");
        return false;
    }
    return true;
}

bool Server::Start()
{
    if (!init_sip_server()) {
        return false;
    }
    thread_ = std::thread(std::bind(&Server::run, this));
    return true;
}

bool Server::Stop()
{
    is_quit_.store(true);
    return true;
}

bool Server::IsClientExist(const std::string& device)
{
    ReadLock _lock(client_mutex_);
    if (this->clients_.count(device) > 0) {
        return true;
    }
    return false;
}

bool Server::AddClient(ClientPtr client)
{
    WriteLock _lock(client_mutex_);
    if (clients_.count(client->device) > 0) {
        return false;
    }
    clients_[client->device] = client;
    return true;
}

bool Server::RemoveClient(const std::string& device)
{
    WriteLock _lock(client_mutex_);
    if (clients_.count(device) <= 0) {
        return false;
    }
    clients_.erase(device);
    return true;
}

void Server::ClearClient()
{
    ReadLock _lock(client_mutex_);
    clients_.clear();
    LOGI("already quit all clients...");
    return;
}

std::unordered_map<std::string, ClientPtr> Server::GetClients()
{
    ReadLock _lock(client_mutex_);
    decltype(clients_) ret_value = clients_;
    return ret_value;
}

bool Server::run()
{
    while (true) {
        {
            ReadLock _lock(client_mutex_);
            if (is_quit_ && clients_.empty()) {
                is_client_all_quit = true;
                LOGI("aleady quit all clients......");
                break;
            }
        }
        eXosip_event_t *evtp = eXosip_event_wait(sip_context_, 0, 20);  // 接受时间20ms超时
        if (!evtp) {
            eXosip_automatic_action(sip_context_);  // 执行一些自动操作
            osip_usleep(100000);
            continue;
        }
        eXosip_automatic_action(sip_context_);  // 执行一些自动操作
        /*
         handler the event here
         应该有个默认的处理函数体,即使没有注册处理函数，也不会没有响应
         */
        std::shared_ptr<Handler> handler_ptr;
        if (event_map_.count(evtp->type) > 0) {
            handler_ptr = event_map_[evtp->type];
        } else {
            handler_ptr = std::make_shared<Handler>();
        }
        if (handler_ptr) {
            handler_ptr->Process(evtp, sip_context_,200);
        }
        eXosip_event_free(evtp);    // 释放事件所占资源
    }
    
    //ClearClient();
    return true;
}

bool Server::register_event_handler()
{
    BEGIN_REGISTER_EVENT_HANDLER 
        REGISTER_EVENT_HANDLER(EXOSIP_MESSAGE_NEW, RegisterHandler), // 新客户端发送请求
        REGISTER_EVENT_HANDLER(EXOSIP_CALL_ANSWERED, CallAnswerHandler), // 宣布通话开始
    END_REGISTER_EVENT_HANDLER 
        //{ EXOSIP_MESSAGE_NEW, std::make_shared<RegisterHandler>()}, // 新客户端发送请求
        //{ EXOSIP_CALL_MESSAGE_NEW, nullptr},
        //{ EXOSIP_CALL_CLOSED, nullptr},
        //{ EXOSIP_CALL_RELEASED, nullptr},
        //{ EXOSIP_MESSAGE_NEW, std::make_shared<RegisterHandler>()}, // 新客户端发送请求
        //{ EXOSIP_MESSAGE_ANSWERED, nullptr},
        //{ EXOSIP_MESSAGE_REQUESTFAILURE, nullptr},
        //{ EXOSIP_CALL_INVITE, nullptr},
        //{ EXOSIP_CALL_PROCEEDING, nullptr},
        //{ EXOSIP_CALL_ANSWERED, nullptr},
        //{ EXOSIP_CALL_SERVERFAILURE, nullptr},
        //{ EXOSIP_IN_SUBSCRIPTION_NEW, nullptr},
    return true;
}
};