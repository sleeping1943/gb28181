#include "handler.h"
#include "../utils/log.h"
#include <osipparser2/osip_parser.h>
#include <string.h>
#include "../server.h"
#include "../utils/helper.h"

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
    this->response_message(evtp, sip_context_, code);
    return true;
}

int Handler::request_bye(eXosip_event_t *evtp, eXosip_t *sip_context_)
{
    eXosip_lock(sip_context_);
    int ret = eXosip_call_terminate(sip_context_, evtp->cid, evtp->did);
    eXosip_unlock(sip_context_);
    return 0;
}

void Handler::response_message(eXosip_event_t *evtp, eXosip_t * sip_context_, int code)
{
    if (evtp == nullptr || evtp->request == nullptr) {
        LOGE("evtp or evtp->requets is nullptr!");
        return;
    }
    osip_body_t* body = nullptr;
    char CmdType[64] = {0};
    char DeviceID[64] = {0};
    // 获取sip协议中message消息body体xml数据并解析
    osip_message_get_body(evtp->request, 0, &body);
    if(body){
        parse_xml(body->body, "<CmdType>", false, "</CmdType>", false, CmdType);
        parse_xml(body->body, "<DeviceID>", false, "</DeviceID>", false, DeviceID);
        CLOGI(YELLOW, "%s", body->body);
    }
    
    if (!Server::instance()->IsClientExist(DeviceID)) {  // 服务器没有此客户端信息,断开连接
        request_bye(evtp, sip_context_);
        return;
    } else if (Server::is_server_quit) {    // 已经开始关闭服务,删除该客户端,发送bye
        Server::instance()->RemoveClient(DeviceID);
        request_bye(evtp, sip_context_);
        return;
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
    return;
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

int Handler::request_invite(eXosip_t *sip_context, ClientPtr client)
{
    char session_exp[1024] = { 0 };
    osip_message_t *msg = nullptr;
    char from[1024] = {0};
    char to[1024] = {0};
    char contact[1024] = {0};
    char sdp[2048] = {0};
    char head[1024] = {0};

    auto s_info = Server::instance()->GetServerInfo();
    client->ssrc = Xzm::util::build_ssrc(true, s_info.realm);
    auto ssrc = Xzm::util::convert10to16(client->ssrc);
    client->rtsp_url = Xzm::util::get_rtsp_addr(s_info.rtp_ip, ssrc);
    
    CLOGI(RED, "addr:%s", client->rtsp_url.c_str());
    sprintf(from, "sip:%s@%s:%d", s_info.sip_id.c_str(),s_info.ip.c_str(), s_info.port);
    sprintf(contact, "sip:%s@%s:%d", s_info.sip_id.c_str(),s_info.ip.c_str(), s_info.port);
    sprintf(to, "sip:%s@%s:%d", client->device.c_str(), client->ip.c_str(), client->port);
    snprintf (sdp, 2048,
              "v=0\r\n"
              "o=%s 0 0 IN IP4 %s\r\n"
              "s=Play\r\n"
              "c=IN IP4 %s\r\n"
              "t=0 0\r\n"
              "m=video %d TCP/RTP/AVP 96 98 97\r\n"
              "a=recvonly\r\n"
              "a=rtpmap:96 PS/90000\r\n"
              "a=rtpmap:98 H264/90000\r\n"
              "a=rtpmap:97 MPEG4/90000\r\n"
              "a=setup:passive\r\n"
              "a=connection:new\r\n"
              "y=%s\r\n"
              "f=\r\n", client->device.c_str(),s_info.rtp_ip.c_str(), s_info.rtp_ip.c_str(), s_info.rtp_port, client->ssrc.c_str());
              //"y=0100000001\r\n"
              //"f=\r\n", s_info.sip_id.c_str(),s_info.ip.c_str(), s_info.rtp_ip.c_str(), s_info.rtp_port);

    int ret = eXosip_call_build_initial_invite(sip_context, &msg, to, from,  nullptr, nullptr);
    if (ret) {
        LOGE( "eXosip_call_build_initial_invite error: %s %s ret:%d", from, to, ret);
        return -1;
    }

    osip_message_set_body(msg, sdp, strlen(sdp));
    osip_message_set_content_type(msg, "application/sdp");
    snprintf(session_exp, sizeof(session_exp)-1, "%i;refresher=uac", s_info.timeout);
    osip_message_set_header(msg, "Session-Expires", session_exp);
    osip_message_set_supported(msg, "timer");

    int call_id = eXosip_call_send_initial_invite(sip_context, msg);

    if (call_id > 0) {
        LOGI("eXosip_call_send_initial_invite success: call_id=%d",call_id);
    }else{
        LOGE("eXosip_call_send_initial_invite error: call_id=%d",call_id);
    }
    return ret;
}

int Handler::parse_xml(const char *data, const char *s_mark, bool with_s_make, const char *e_mark, bool with_e_make, char *dest)
{
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

/*
"v=0\r\n
o=34020000002000000001 0 0 IN IP4 10.23.132.27\r\n
s=Play\r\n
c=IN IP4 10.23.132.27\r\n
t=0 0\r\n
m=video 10000 TCP/RTP/AVP 96 98 97\r\n
a=recvonly\r\n
a=rtpmap:96 PS/90000\r\n
a=rtpmap:98 H264/90000\r\n
a=rtpmap:97 MPEG4/90000\r\n
a=setup:passive\r\n
a=connection:new\r\n
y=0200002495\r\n
f=\r\n"
*/