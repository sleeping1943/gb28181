/**
 * @file register_handler.h
 * @author sleeping (csleeping@163.com)
 * @brief 注册处理类
 * @version 0.1
 * @date 2023-08-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once
#include "handler.h"
#include "../xzm_defines.h"


namespace Xzm {

class RegisterHandler : public Handler
{
public:
    RegisterHandler();
    ~RegisterHandler();

    virtual bool Process(eXosip_event_t *evtp, eXosip_t* sip_context_, int code) override;

private:
    /**
     * @brief 注册新客户端
     * 
     * @param evtp 
     * @param sip_context_ 
     * @return true 
     * @return false 
     */
    bool register_client(eXosip_event_t *evtp, eXosip_t* sip_context_);
    /**
     * @brief 无授权信息返回401
     * 
     * @param evtp 
     * @param sip_context_ 
     */
    void response_register_401unauthorized(eXosip_event_t *evtp, eXosip_t* sip_context_);
    /**
     * @brief 验证授权信息
     * 
     * @param evtp 
     * @param auth 
     * @return true 
     * @return false 
     */
    bool check_ha1(eXosip_event_t *evtp, osip_authorization_t *auth);
};
};