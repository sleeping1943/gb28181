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
    Handler();
    virtual ~Handler();
    virtual bool Process(eXosip_event_t *evtp, eXosip_t* sip_context_, int code);

    void response_message(eXosip_event_t *evtp, eXosip_t * sip_context_, int code);
    void response_message_answer(eXosip_event_t *evtp, eXosip_t * sip_context_, int code);
    int request_invite(eXosip_t *sip_context, const std::string& device, const std::string& user_ip, unsigned short user_port);

    int parse_xml(const char *data, const char *s_mark, bool with_s_make,
        const char *e_mark, bool with_e_make, char *dest);
};
using HandlerPtr = std::shared_ptr<Handler>;
};