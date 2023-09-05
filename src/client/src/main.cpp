#include <iostream>
#include <string>
#include <string.h>
#include "../../utils/log.h"
#include <arpa/inet.h>
#include <thread>


extern "C" {
#include <eXosip2/eXosip.h>
#include <osip2/osip_mt.h>
}

eXosip_t * sip_context_ = nullptr;
const char *server_uac = "XZM_SipServer";
const char *server_ip = "10.23.132.54";
unsigned short server_port = 15060;

bool init_sip();
void Run();
int request_invite(const char *server_uac, const char *server_ip, int server_port);
void dump_request(eXosip_event_t *evtp);
void dump_response(eXosip_event_t *evtp);

int main()
{
    std::cout << "hellp world" << std::endl;
    if (!init_sip()) {
        LOGE("init sip error!");
        return -1;
    }
    // 直接发送一个invite请求
    // invite......
    request_invite(server_uac, server_ip, server_port);

    std::thread t(&Run);
    if (t.joinable()) {
        t.join();
    }
    return 0;
}

bool init_sip()
{
    //register_event_handler();
    /* 
    step 1 
    申请结构体内存
    */
    sip_context_ = eXosip_malloc();
    if (!sip_context_) {
        LOGI("eXosip_malloc error!");
        return false;
    }
    /*
     step 2
     初始化
    */
    if (eXosip_init(sip_context_)) {
        LOGI("eXosip_init error");
        return false;
    }
    /*
      step 3
      开始监听
     */
    if (eXosip_listen_addr(sip_context_, IPPROTO_UDP, nullptr, 25060, AF_INET, 0)) {
        LOGI("eXosip_listen_addr error");
        return false;
    }

     eXosip_set_user_agent(sip_context_, "XZM_SipClient_test");
     /**
      step 4
      添加授权信息
      */
    //if (eXosip_add_authentication_info(sip_context_, s_info_.sip_id.c_str(),
    //   s_info_.sip_id.c_str(), s_info_.passwd.c_str(), nullptr, s_info_.realm.c_str())) {
    //    LOGI("eXosip_add_authentication_info error");
    //    return false;
    //}
    return true;
}

void Run()
{
    while (true) {
        if (!sip_context_) { break; }
        eXosip_event_t *evtp = eXosip_event_wait(sip_context_, 0, 20);  // 接受时间20ms超时
        if (!evtp) {
            eXosip_automatic_action(sip_context_);  // 执行一些自动操作
            osip_usleep(100000);
            continue;
        }
        eXosip_automatic_action(sip_context_);  // 执行一些自动操作
        /*
         handler the event here
         应该有个默认的处理函数体,即使没有注册处理函数，也不会没有响应
         */
        switch(evtp->type) {
        case EXOSIP_MESSAGE_NEW:    // 新客户端发送请求
        case EXOSIP_CALL_MESSAGE_NEW:
        case EXOSIP_CALL_CLOSED:
        case EXOSIP_CALL_RELEASED:
        case EXOSIP_MESSAGE_ANSWERED:
        case EXOSIP_MESSAGE_REQUESTFAILURE:
        case EXOSIP_CALL_INVITE:
        case EXOSIP_CALL_PROCEEDING:
        case EXOSIP_CALL_ANSWERED:
        case EXOSIP_CALL_SERVERFAILURE:
        case EXOSIP_IN_SUBSCRIPTION_NEW:
        default:
            dump_request(evtp);
            dump_response(evtp);
        break;
        }
        eXosip_event_free(evtp);    // 释放事件所占资源
    }
    return;
}

void dump_request(eXosip_event_t *evtp)
{
    char *s;
    size_t len;
    osip_message_to_str(evtp->request, &s, &len);
    LOGI("\nprint request start\ntype=%d\n%s\nprint request end\n",evtp->type,s);
}

void dump_response(eXosip_event_t *evtp)
{
    char *s;
    size_t len;
    osip_message_to_str(evtp->response, &s, &len);
    LOGI("\nprint response start\ntype=%d\n%s\nprint response end\n",evtp->type,s);
}


int request_invite(const char *server_uac, const char *server_ip, int server_port)
{
    LOGI("INVITE");
    char session_exp[1024] = { 0 };
    osip_message_t *msg = nullptr;
    char from[1024] = {0};
    char to[1024] = {0};
    char contact[1024] = {0};
    char sdp[2048] = {0};
    char head[1024] = {0};

    char sip_id[] = "34020000002000000001";
    char local_ip[] = "10.23.132.54";
    unsigned short local_port = 25060;
    unsigned short rtp_server_port = 10000;

    sprintf(from, "sip:%s@%s:%d", "XZM_SipClient", local_ip, local_port);
    sprintf(contact, "sip:%s@%s:%d", "XZM_SipClient", local_ip, local_port);
    sprintf(to, "sip:%s@%s:%d", sip_id, server_ip, server_port);
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
              "y=0100000001\r\n"
              "f=\r\n", "XZM_SipClient", local_ip, local_ip, rtp_server_port);

    int ret = eXosip_call_build_initial_invite(sip_context_, &msg, to, from,  nullptr, nullptr);
    if (ret) {
        LOGE( "eXosip_call_build_initial_invite error: %s %s ret:%d", from, to, ret);
        return -1;
    }

    osip_message_set_body(msg, sdp, strlen(sdp));
    osip_message_set_content_type(msg, "application/sdp");
    snprintf(session_exp, sizeof(session_exp)-1, "%i;refresher=uac", 1800);
    osip_message_set_header(msg, "Session-Expires", session_exp);
    osip_message_set_supported(msg, "timer");

    int call_id = eXosip_call_send_initial_invite(sip_context_, msg);

    if (call_id > 0) {
        LOGI("eXosip_call_send_initial_invite success: call_id=%d",call_id);
    }else{
        LOGE("eXosip_call_send_initial_invite error: call_id=%d",call_id);
    }
    return ret;
}