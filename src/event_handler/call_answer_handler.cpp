#include "call_answer_handler.h"
#include "../utils/log.h"

namespace Xzm
{

CallAnswerHandler::CallAnswerHandler()
{

}

CallAnswerHandler::~CallAnswerHandler()
{

}

bool CallAnswerHandler::Process(eXosip_event_t *evtp, eXosip_t* sip_context_, int code)
{
    osip_message_t* msg = nullptr;
    int ret = eXosip_call_build_ack(sip_context_, evtp->did, &msg);
    if (!ret && msg) {
        eXosip_call_send_ack(sip_context_, evtp->did, msg);
        LOGI("eXosip_call_send_ack OK");
    } else {
        LOGE("eXosip_call_send_ack error=%d", ret);
    }
    return true;
}
};