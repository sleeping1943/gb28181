/**
 * @file handler.h
 * @author sleeping (csleeping@163.com)
 * @brief 事件处理基类
 * @version 0.1
 * @date 2023-08-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include <iostream>
#include "eXosip2/eXosip.h"
#include <memory>

namespace Xzm
{
class Handler {
public:
    Handler() {

    }
    virtual ~Handler() {

    }

    virtual bool Process(eXosip_event_t *evtp, int code)
    {
        std::cout << "Handler Process!!" << std::endl;
        return true;
    }
};
using HandlerPtr = std::shared_ptr<Handler>;
};