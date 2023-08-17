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

    virtual bool Process(eXosip_event_t *evtp, int code) override;
};
};