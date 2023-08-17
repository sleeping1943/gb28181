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
#include <string>
#include <iostream>
#include <memory>
#include <sstream>

namespace Xzm
{

#define BEGIN_REGISTER_EVENT_HANDLER    \
    event_map_ = std::unordered_map<eXosip_event_type, HandlerPtr>{
#define REGISTER_EVENT_HANDLER(event, handler)  \
    {event, std::make_shared<handler>()}
#define END_REGISTER_EVENT_HANDLER };

/* sip服务器配置信息 */
struct ServerInfo
{
    std::string ua;     //服务器名称
    std::string nonce;  //随机数值
    std::string ip;
    unsigned short port;
    unsigned short  rtp_port;
    std::string sid_id;         //sip服务器ID
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
            << std::endl << "\trtp_port:\t" <<  rtp_port
            << std::endl << "\tsid_id:\t" << sid_id
            << std::endl << "\trealm:\t" << realm
            << std::endl << "\tpasswd:\t" << passwd
            << std::endl << "\ttimeout:\t" << timeout
            << std::endl << "\tvalid_time:\t" << valid_time;
        return ss.str();
    }
};
using ServerInfoPtr = std::shared_ptr<ServerInfo>;

// 客户端信息
struct Client
{
    Client(const std::string& _ip, unsigned short _port,
    const std::string& _device, bool _is_reg, unsigned short _rtp_port):
    ip(_ip),port(_port),device(_device),is_reg(_is_reg),rtp_port(_rtp_port)
    {

    }

    ~Client() = default;

    std::string ip;
    unsigned short port = 0;
    std::string device;
    bool is_reg = false;
    unsigned short rtp_port = 10000;
};
using ClientPtr = std::shared_ptr<Client>;

};