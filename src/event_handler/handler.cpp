#include "handler.h"
#include "../utils/log.h"
#include <osipparser2/osip_message.h>
#include <osipparser2/osip_parser.h>
#include <ostream>
#include <string.h>
#include "../server.h"
#include "../utils/helper.h"
#include "../utils/tinyxml2.h"

using tinyxml2::XMLDocument;
using tinyxml2::XMLError;
using tinyxml2::XMLElement;

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
    this->dump_request(evtp);
    this->dump_response(evtp);
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
        this->parse_device_xml(body->body);
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

int Handler::request_device_query(eXosip_t *sip_context, ClientPtr client)
{
    if (!sip_context || !client) {
        return -1;
    }
    char str_from[512] = {0};
    char str_to[512] = {0};
    char str_body[2048] = {0};
    auto s_info = Server::instance()->GetServerInfo();
    sprintf(str_from, "sip:%s@%s:%d", s_info.sip_id.c_str(), s_info.ip.c_str(), s_info.port);
    sprintf(str_to, "sip:%s@%s:%d", client->device.c_str(), client->ip.c_str(), client->port);
    snprintf(str_body, 2048,
    "<?xml version=\"1.0\"?>"\
    "<Query>"   \
    "<CmdType>Catalog</CmdType>"    \
    /*"<SN>248</SN>"  \*/
    "<DeviceID>%s</DeviceID>" \
    "</Query>", client->device.c_str()
    );

    osip_message_t *message = nullptr;
    eXosip_message_build_request(sip_context, &message, "MESSAGE", str_to, str_from, nullptr);
    osip_message_set_body(message, str_body, strlen(str_body));
    osip_message_set_content_type(message, "Application/MANSCDP+xml");
    eXosip_lock(sip_context);
    int ret = eXosip_message_send_request(sip_context, message);
    CLOGI(RED, "send device query ret:%d", ret);
    eXosip_unlock(sip_context);
    return 0;
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

/**
<Response>
<CmdType>Catalog</CmdType>
<SN>0</SN>
<DeviceID>34020000002000001001</DeviceID>
<SumNum>2</SumNum>
<DeviceList Num="2">
<Item>
<DeviceID>34020000001310000001</DeviceID>
<Name>200w</Name>
<Manufacturer>GBT28181</Manufacturer>
<Model>IP Camera</Model>
<Owner>Owner</Owner>
<CivilCode>3402000000</CivilCode>
<Address>Address</Address>
<Parental>0</Parental>
<ParentID>34020000001310000001</ParentID>
<SafetyWay>0</SafetyWay>
<RegisterWay>1</RegisterWay>
<Secrecy>0</Secrecy>
<Status>ON</Status>
</Item>
<Item>
<DeviceID>34020000001370000001</DeviceID>
<Name>AudioOut</Name>
<Manufacturer>GBT28181</Manufacturer>
<Model>AudioOut</Model>
<Owner>Owner</Owner>
<CivilCode>3402000000</CivilCode>
<Address>Address</Address>
<Parental>0</Parental>
<ParentID>34020000002000001001</ParentID>
<SafetyWay>0</SafetyWay>
<RegisterWay>1</RegisterWay>
<Secrecy>0</Secrecy>
<Status>ON</Status>
</Item>
</DeviceList>
</Response>
 */
int Handler::parse_device_xml(const std::string& xml_str)
{
    XMLDocument doc;
    auto ret = doc.Parse(xml_str.c_str());
    if (ret != XMLError::XML_SUCCESS) {
        LOGE("parse device xml error!");
        return -1;
    }
    // 根元素
    XMLElement *root = doc.RootElement();
    // 指定名字的第一个子元素
    XMLElement *node_device_id = root->FirstChildElement("DeviceID");
    if (!node_device_id) {
        LOGE("parse device_id error!");
        return -2;
    }
    std::string device_id = node_device_id->GetText();
    XMLElement *node_device_list = root->FirstChildElement("DeviceList");
    if (!node_device_list) {
        LOGE("parse device list error!");
        return -3;
    }
    XMLElement *node_device_item = node_device_list->FirstChildElement("Item");
    int index = 0;
    std::string temp_str;
    std::stringstream ss;
    std::unordered_map<std::string, ClientInfoPtr> client_infos;   // <device_id, client_info>
    do {
        ClientInfoPtr client_info = std::make_shared<ClientInfo>();
        client_info->device_id = node_device_item->FirstChildElement("DeviceID")->GetText();
        client_info->name = node_device_item->FirstChildElement("Name")->GetText();
        client_info->manufacturer = node_device_item->FirstChildElement("Manufacturer")->GetText();
        client_info->model = node_device_item->FirstChildElement("Model")->GetText();
        client_info->owner = node_device_item->FirstChildElement("Owner")->GetText();
        client_info->civil_code = node_device_item->FirstChildElement("CivilCode")->GetText();
        client_info->address = node_device_item->FirstChildElement("Address")->GetText();
        temp_str = node_device_item->FirstChildElement("Parental")->GetText();
        client_info->parental = std::stoi(temp_str);
        client_info->parent_id = node_device_item->FirstChildElement("ParentID")->GetText();
        temp_str = node_device_item->FirstChildElement("SafetyWay")->GetText();
        client_info->safety_way = std::stoi(temp_str);
        temp_str = node_device_item->FirstChildElement("RegisterWay")->GetText();
        client_info->register_way = std::stoi(temp_str);
        temp_str = node_device_item->FirstChildElement("Secrecy")->GetText();
        client_info->secrecy = std::stoi(temp_str);
        temp_str = node_device_item->FirstChildElement("Status")->GetText();
        client_info->status = (temp_str == "ON") ? 1 : 0;
        client_infos[client_info->device_id] = client_info;
        node_device_item = node_device_item->NextSiblingElement("Item");
        ss << "index[" << index++ << "]:" << std::endl
        << "DeviceID    :" << client_info->device_id << std::endl
        << "Name        :" << client_info->name << std::endl
        << "Manufacturer:" << client_info->manufacturer << std::endl
        << "Model       :" << client_info->model << std::endl
        << "Owner       :" << client_info->owner << std::endl
        << "CivilCode   :" << client_info->civil_code << std::endl
        << "Address     :" << client_info->address << std::endl
        << "Parental    :" << client_info->parental << std::endl
        << "ParentID    :" << client_info->parent_id << std::endl
        << "RegisterWay :" << client_info->register_way << std::endl
        << "secrecy     :" << client_info->secrecy << std::endl
        << "status:" << client_info->status << std::endl;
        CLOGI(RED, "%s", ss.str().c_str());
        ss.str("");
    } while (node_device_item);
    Server::instance()->UpdateClientInfo(device_id, client_infos);
    return 0;
}

void Handler::dump_request(eXosip_event_t *evtp)
{
    char *s;
    size_t len;
    osip_message_to_str(evtp->request, &s, &len);
    CLOGI(YELLOW, "\n********************print request start\ttype=%d********************\n%s\n********************print request end********************\n",evtp->type,s);
}

void Handler::dump_response(eXosip_event_t *evtp)
{
    char *s;
    size_t len;
    osip_message_to_str(evtp->response, &s, &len);
    CLOGI(BLUE, "\n********************print response start\ttype=%d********************\n%s\n********************print response end********************\n",evtp->type,s);
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