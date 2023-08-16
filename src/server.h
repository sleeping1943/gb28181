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

namespace Xzm
{

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

    bool SetServerInfo(const std::string& json_str);
    inline ServerInfo GetServerInfo() {
        return s_info_;
    }

private:
    ServerInfo s_info_;
};
};