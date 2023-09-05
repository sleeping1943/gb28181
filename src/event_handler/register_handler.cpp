#include "register_handler.h"
#include <iostream>
#include "/usr/local/include/osipparser2/osip_message.h"
#include <osipparser2/headers/osip_header.h>
#include <string.h>
#include "../server.h"
#include "../utils/http_digest.h"
#include "../utils/log.h"
#include "../utils/helper.h"

namespace Xzm
{

RegisterHandler::RegisterHandler()
{

}

RegisterHandler::~RegisterHandler()
{

}

bool RegisterHandler::Process(eXosip_event_t *evtp, eXosip_t* sip_context_, int code)
{
    CLOGI(YELLOW, "register_handler Process!!!");
    // contact为null，不能这么获取
    //osip_contact_t *contact = nullptr;
    //osip_message_get_contact(evtp->request, 0, &contact);
    //std::string device = std::string(contact->url->username);
    if (MSG_IS_REGISTER(evtp->request)) {   // 注册客户端消息
        CLOGI(BLUE, "register msg!!");
        register_client(evtp, sip_context_);
    } else if (MSG_IS_MESSAGE(evtp->request)) { // 普通消息,包含心跳信息等
        response_message(evtp, sip_context_, 200);  // 响应消息,比如回复心跳keepalive
    } else if (MSG_IS_BYE(evtp->request)) {
        osip_body_t* body = nullptr;
        char CmdType[64] = {0};
        char DeviceID[64] = {0};
        // 获取sip协议中message消息body体xml数据并解析
        osip_message_get_body(evtp->request, 0, &body);
        if(body){
            parse_xml(body->body, "<CmdType>", false, "</CmdType>", false, CmdType);
            parse_xml(body->body, "<DeviceID>", false, "</DeviceID>", false, DeviceID);
        }
        if (Server::is_server_quit) {   // 服务退出,需要断开所有客户端连接
            Server::instance()->RemoveClient(DeviceID);
            std::cout << "\033[31m断开客户端[" << DeviceID << "]\033[0m" << std::endl;
        }
        request_bye(evtp, sip_context_);
        std::cout << "bye message msg!!" << std::endl;
    } else if (strncmp(evtp->request->sip_method, "BYE", 3) != 0) {
        std::cout << "bye" << std::endl;
    } else {
        std::cout << "unsuported msg!!" << std::endl;
    }
    return true;
}

bool RegisterHandler::register_client(eXosip_event_t *evtp, eXosip_t* sip_context_)
{
    osip_authorization_t *auth = nullptr;
    osip_message_get_authorization(evtp->request, 0, &auth);

    if (auth == nullptr || auth->username == nullptr) { // 无授权信息
        response_register_401unauthorized(evtp, sip_context_);
        return false;
    }

    osip_contact_t *contact = nullptr;
    osip_message_get_contact(evtp->request, 0, &contact);
    auto s_info = Server::instance()->GetServerInfo();
    ClientPtr client = std::make_shared<Client>( contact->url->host,
        atoi(contact->url->port), contact->url->username);
    osip_header_t *user_agent = nullptr;
    osip_message_get_user_agent(evtp->request, 0, &user_agent); // 获取代理类型,判断是否为摄像头
    do {
        if (!user_agent || !user_agent->hname || !user_agent->hvalue) {
            break;
        }
        LOGI("user_agent[name:%s value:%s]", user_agent->hname, user_agent->hvalue);
        std::string user_agent_type(user_agent->hvalue);
        if (user_agent_type.compare("IP Camera")) {
            break;
        }
        client->client_type = kClientIPC;
    } while (0);
    do {
        if (!check_ha1(evtp, auth)) {   // 鉴权没通过
            this->response_message_answer(evtp, sip_context_, 401);
            LOGI("Camera registration error, p=%s,port=%d,device=%s",client->ip.c_str(),client->port,client->device.c_str());
            break;
        }
        this->response_message_answer(evtp, sip_context_, 200);
        if (client->client_type != kClientIPC) {    // 只保存IP Camera类型客户端
            break;
        }
        LOGI("IP Camera registration succee,ip=%s,port=%d,device=%s",client->ip.c_str(),client->port,client->device.c_str());
        if (!Server::instance()->IsClientExist(client->device)) {   // 不存在该客户端
            Server::instance()->AddClient(client);
            this->request_device_query(sip_context_, client);
        }
        //request_invite(sip_context_, client);
    } while (0);
    return true;
}

void RegisterHandler::response_register_401unauthorized(eXosip_event_t *evtp, eXosip_t* sip_context_)
{
    char *dest = nullptr;
    osip_message_t * reg = nullptr;
    osip_www_authenticate_t * header = nullptr;
    auto s_info = Server::instance()->GetServerInfo();
    osip_www_authenticate_init(&header);
    osip_www_authenticate_set_auth_type (header, osip_strdup("Digest"));
    osip_www_authenticate_set_realm(header,osip_enquote(s_info.realm.c_str()));
    osip_www_authenticate_set_nonce(header,osip_enquote(s_info.nonce.c_str()));
    osip_www_authenticate_to_str(header, &dest);
    int ret = eXosip_message_build_answer (sip_context_, evtp->tid, 401, &reg);
    if ( ret == 0 && reg != nullptr ) {
        osip_message_set_www_authenticate(reg, dest);
        osip_message_set_content_type(reg, "Application/MANSCDP+xml");
        eXosip_lock(sip_context_);
        eXosip_message_send_answer (sip_context_, evtp->tid,401, reg);
        eXosip_unlock(sip_context_);
        std::cout << "response_register_401unauthorized success" << std::endl;
    }else {
        std::cout << "response_register_401unauthorized error" << std::endl;
    }

    osip_www_authenticate_free(header);
    osip_free(dest);

}

bool RegisterHandler::check_ha1(eXosip_event_t *evtp, osip_authorization_t *auth)
{
    bool ret = true;
    char *method = NULL, // REGISTER
    *algorithm = NULL, // MD5
    *username = NULL,// 340200000013200000024
    *realm = NULL, // sip服务器传给客户端，客户端携带并提交上来的sip服务域
    *nonce = NULL, //sip服务器传给客户端，客户端携带并提交上来的nonce
    *nonce_count = NULL,
    *uri = NULL; // sip:34020000002000000001@3402000000


    method = evtp->request->sip_method;
    char calc_response[HASHHEXLEN];
    HASHHEX HA1, HA2 = "", Response;

    SIP_STRDUP(algorithm);
    SIP_STRDUP(username);
    SIP_STRDUP(realm);
    SIP_STRDUP(nonce);
    SIP_STRDUP(nonce_count);
    SIP_STRDUP(uri);

    // 计算比较hash值,若sip协议中携带信息计算后ha1与服务器的信息计算后ha1相等，则验证授权通过
    auto s_info = Server::instance()->GetServerInfo();
    DigestCalcHA1(algorithm, username, realm,
     s_info.passwd.c_str(), nonce, nonce_count, HA1);
    DigestCalcResponse(HA1, nonce, nonce_count, auth->cnonce,
     auth->message_qop, 0, method, uri, HA2, Response);

    HASHHEX temp_HA1;
    HASHHEX temp_response;
    DigestCalcHA1("REGISTER", username, s_info.realm.c_str(),
     s_info.passwd.c_str(), s_info.nonce.c_str(), NULL, temp_HA1);
    DigestCalcResponse(temp_HA1, s_info.nonce.c_str(), NULL,
     NULL, NULL, 0, method, uri, NULL, temp_response);
    memcpy(calc_response, temp_response, HASHHEXLEN);

    if (memcmp(calc_response, Response, HASHHEXLEN)) {  // 验证失败
        ret = false;
    } 
    osip_free(algorithm);
    osip_free(username);
    osip_free(realm);
    osip_free(nonce);
    osip_free(nonce_count);
    osip_free(uri);
    return ret;
}
};