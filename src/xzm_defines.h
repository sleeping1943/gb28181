/**
 * @file xzm_defines.h
 * @author sleeping (csleeping@163.com)
 * @brief 一些类型定义
 * @version 0.1
 * @date 2023-08-17
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once
#include <boost/thread/pthread/shared_mutex.hpp>
#include <string>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <sstream>
#include <queue>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

namespace Xzm
{

#define BEGIN_REGISTER_EVENT_HANDLER    \
    event_map_ = std::unordered_map<eXosip_event_type, HandlerPtr>{
#define REGISTER_EVENT_HANDLER(event, handler)  \
    {event, std::make_shared<handler>()}
#define END_REGISTER_EVENT_HANDLER };

#define SIP_STRDUP(field) if (auth->field) (field) = osip_strdup_without_quote(auth->field)

/* 读写锁定义 */
typedef boost::shared_mutex B_Lock;
typedef boost::unique_lock<B_Lock> WriteLock;
typedef boost::shared_lock<B_Lock> ReadLock;

// 客户端代理类型
enum XClientType
{
    kClientNone = 0,    // 未知
    kClientIPC,         // 网络摄像头
    kClientMax,         // 无
};

// 客户端请求类型
enum RequestType
{
    kRequestTypeNone = 0,
    kRequestTypeInvite, // 建立会话请求
    kRequestTypeMax = 9999,

};
/* sip服务器配置信息 */
struct ServerInfo
{
    std::string ua;     //服务器名称
    std::string nonce;  //随机数值
    std::string ip;
    unsigned short port;
    std::string rtp_ip;
    unsigned short  rtp_port;
    std::string sip_id;         //sip服务器ID
    std::string realm;          // sip域
    std::string passwd;         // 服务器密码
    unsigned int timeout;       // 超时间隔
    unsigned int valid_time;    // 有效时长

    std::string str()
    {
        std::stringstream ss;
        ss << std::endl << "\tua:\t" << ua
            << std::endl << "\tnonce:\t" << nonce
            << std::endl << "\tip:\t" << ip
            << std::endl << "\tport:\t" << port
            << std::endl << "\trtp_ip:\t" <<  rtp_ip
            << std::endl << "\trtp_port:\t" <<  rtp_port
            << std::endl << "\tsip_id:\t" << sip_id
            << std::endl << "\trealm:\t" << realm
            << std::endl << "\tpasswd:\t" << passwd
            << std::endl << "\ttimeout:\t" << timeout
            << std::endl << "\tvalid_time:\t" << valid_time;
        return ss.str();
    }
};
using ServerInfoPtr = std::shared_ptr<ServerInfo>;

struct HttpServerInfo
{
    std::string ip;
    unsigned short port;
};

struct ClientInfo
{
    std::string device_id;
    std::string name;
    std::string manufacturer;   // 厂商
    std::string model;          //IP Camera
    std::string owner;
    std::string civil_code;     // 3402000000
    std::string address;
    int parental = 0;
    std::string parent_id;      // parent_id和client中的device_id相同时，该设备具有语音输出功能
    int register_way = 1;
    int safety_way = 0;
    int secrecy = 0;
    int status = 1;
};
using ClientInfoPtr = std::shared_ptr<ClientInfo>;

// 客户端信息
struct Client
{
    Client(const std::string& _ip, unsigned short _port,
    const std::string& _device): ip(_ip),port(_port),device(_device)
    {

    }

    Client(const std::string& _ip, unsigned short _port,
    const std::string& _device, bool _is_reg, unsigned short _rtp_port, XClientType _client_type):
    ip(_ip),port(_port),device(_device),is_reg(_is_reg),rtp_port(_rtp_port),client_type(_client_type)
    {

    }

    ~Client() = default;

    std::string ip;
    unsigned short port = 0;
    std::string device;
    bool is_reg = false;
    unsigned short rtp_port = 10000;
    std::string ssrc;   // 10进制
    std::string rtsp_url;
    XClientType client_type = kClientNone;
    std::unordered_map<std::string, ClientInfoPtr> client_infos_;   // 每个设备有多个信息，如摄像头有视频和音频2个设备信息
};
using ClientPtr = std::shared_ptr<Client>;

struct ClientRequest
{
    ClientPtr client_ptr;
    RequestType req_type;
};
using ClientRequestPtr = std::shared_ptr<ClientRequest>;
using RequestQueue = std::queue<ClientRequestPtr>;

};