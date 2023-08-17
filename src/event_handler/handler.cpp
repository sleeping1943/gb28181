#include "handler.h"
#include "../utils/log.h"
#include <string.h>

namespace Xzm {

Handler::Handler()
{

}

Handler::~Handler()
{

}

bool Handler::Process(eXosip_event_t *evtp, eXosip_t* sip_context_, int code)
{
    std::cout << "Handler Process!!" << std::endl;
    return true;
}

void Handler::response_message(eXosip_event_t *evtp, eXosip_t * sip_context_, int code)
{
    osip_body_t* body = nullptr;
    char CmdType[64] = {0};
    char DeviceID[64] = {0};
    // 获取sip协议中message消息body体xml数据并解析
    osip_message_get_body(evtp->request, 0, &body);
    if(body){
        parse_xml(body->body, "<CmdType>", false, "</CmdType>", false, CmdType);
        parse_xml(body->body, "<DeviceID>", false, "</DeviceID>", false, DeviceID);
    }
    LOGI("CmdType=%s,DeviceID=%s", CmdType,DeviceID);

    if(!strcmp(CmdType, "Catalog")) {
        this->response_message_answer(evtp, sip_context_, 200);
        // 需要根据对方的Catelog请求，做一些相应的应答请求
    } else if(!strcmp(CmdType, "Keepalive")){   // 心跳消息
        this->response_message_answer(evtp, sip_context_, 200);
    }else{
        this->response_message_answer(evtp, sip_context_, 200);
    }
}

void Handler::response_message_answer(eXosip_event_t *evtp, eXosip_t * sip_context_, int code)
{
    int returnCode = 0 ;
    osip_message_t * pRegister = nullptr;
    returnCode = eXosip_message_build_answer (sip_context_,evtp->tid,code,&pRegister);
    bool bRegister = false;
    if(pRegister){
        bRegister = true;
    }
    if (returnCode == 0 && bRegister)
    {
        eXosip_lock(sip_context_);
        eXosip_message_send_answer (sip_context_,evtp->tid,code,pRegister);
        eXosip_unlock(sip_context_);
    }
    else{
        LOGE("code=%d,returnCode=%d,bRegister=%d",code,returnCode,bRegister);
    }

}

int Handler::parse_xml(const char *data, const char *s_mark, bool with_s_make, const char *e_mark, bool with_e_make, char *dest) {
    const char* satrt = strstr( data, s_mark );

    if(satrt != NULL) {
        const char* end = strstr(satrt, e_mark);

        if(end != NULL){
            int s_pos = with_s_make ? 0 : strlen(s_mark);
            int e_pos = with_e_make ? strlen(e_mark) : 0;

            strncpy( dest, satrt+s_pos, (end+e_pos) - (satrt+s_pos) );
        }
        return 0;
    }
    return -1;

}
};