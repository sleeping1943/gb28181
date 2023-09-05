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
#include "../xzm_defines.h"

namespace Xzm
{
class Handler {
public:
    Handler();
    virtual ~Handler();
    virtual bool Process(eXosip_event_t *evtp, eXosip_t* sip_context_, int code);

    /**
     * @brief 结束会话
     * 
     * @param evtp 
     * @param sip_context_ 
     * @return int 
     */
    int request_bye(eXosip_event_t *evtp, eXosip_t *sip_context_);
    void response_message(eXosip_event_t *evtp, eXosip_t * sip_context_, int code);
    void response_message_answer(eXosip_event_t *evtp, eXosip_t * sip_context_, int code);
    int request_invite(eXosip_t *sip_context, ClientPtr client);
    int request_device_query(eXosip_t *sip_context, ClientPtr client);
    int parse_xml(const char *data, const char *s_mark, bool with_s_make,
        const char *e_mark, bool with_e_make, char *dest);

    int parse_device_xml(const std::string& xml_str);

    void dump_request(eXosip_event_t *evtp);
    void dump_response(eXosip_event_t *evtp);
};
using HandlerPtr = std::shared_ptr<Handler>;
};