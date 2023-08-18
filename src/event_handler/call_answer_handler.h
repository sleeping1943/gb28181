/**
 * @file call_answer_handler.h
 * @author sleeping (csleeping@163.com)
 * @brief 处理invite调用的响应,表示会话开始,若不处理此消息,流媒体服务器会报错(illegal connection)
 * @version 0.1
 * @date 2023-08-18
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once
#include "handler.h"

namespace Xzm
{
class CallAnswerHandler : public Handler
{
public:
    CallAnswerHandler();
    ~CallAnswerHandler();

    virtual bool Process(eXosip_event_t *evtp, eXosip_t* sip_context_, int code) override;
};
};