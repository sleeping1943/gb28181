#include "http_server.h"
#include "../include/rapidjson/document.h"
#include "../utils/json_helper.h"
#include "../utils/helper.h"
#include "../utils/log.h"
#include <functional>
#include <hv/HttpServer.h>
#include <mutex>
#include "../server.h"

namespace Xzm
{
XHttpServer::XHttpServer():is_quit_(false)
{

}

XHttpServer::~XHttpServer()
{

}

bool XHttpServer::Init(const std::string& conf_path)
{
    std::string content;
    if (!Xzm::util::read_file(conf_path, content)) {
        return false;
    }
    rapidjson::Document doc;
    JSON_PARSE_BOOL(doc, content.c_str());
    if (!doc.HasMember("http_config") || !doc["http_config"].IsObject()) {
        return false;
    }
    auto& http_config = doc["http_config"];
    JSON_VALUE_REQUIRE_STRING(http_config, "ip", s_info_.ip);
    JSON_VALUE_REQUIRE_INT(http_config, "port", s_info_.port);

    server_.service = &router;
    server_.port = s_info_.port;

    router.GET("/query_device",
     std::bind(&XHttpServer::query_device_list, this, std::placeholders::_1, std::placeholders::_2));
    router.GET("/start_rtsp_publish",
     std::bind(&XHttpServer::start_rtsp_publish, this, std::placeholders::_1, std::placeholders::_2));
    router.GET("/stop_rtsp_publish",
     std::bind(&XHttpServer::stop_rtsp_publish, this, std::placeholders::_1, std::placeholders::_2));
    router.POST("/on_publish",
    std::bind(&XHttpServer::on_publish, this, std::placeholders::_1, std::placeholders::_2));
    router.POST("/on_play",
    std::bind(&XHttpServer::on_play, this, std::placeholders::_1, std::placeholders::_2));
    return true;
}

bool XHttpServer::Start()
{
    Run();
    return true;
}

bool XHttpServer::Stop()
{
    http_server_stop(&server_);
    is_quit_ = true;
    return true;
}

bool XHttpServer::Run()
{
    thread_ = std::thread([this]() {
        http_server_run(&server_);
    });
    return true;
}

int XHttpServer::query_device_list(HttpRequest* req, HttpResponse* resp)
{
    std::string device_list;
    rapidjson::Document doc(rapidjson::kObjectType);    // doc.SetObject();
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
    std::vector<ClientPtr> vec_device;
    const auto& clients = Server::instance()->GetClients();
    rapidjson::Value arr_device(rapidjson::kArrayType);
    for (const auto& iter : clients) {
        ClientPtr device = iter.second;
        rapidjson::Value value(rapidjson::kObjectType);
        rapidjson::Value d_name(rapidjson::kStringType);
        rapidjson::Value d_ip(rapidjson::kStringType);
        rapidjson::Value d_port(rapidjson::kNumberType);
        rapidjson::Value d_ssrc(rapidjson::kStringType);
        rapidjson::Value d_rtsp_url(rapidjson::kStringType);
        d_name.SetString(device->device.c_str(), allocator);
        d_ip.SetString(device->ip.c_str(), allocator);
        d_port.SetInt(device->port);
        d_ssrc.SetString(device->ssrc.c_str(), allocator);
        d_rtsp_url.SetString(device->rtsp_url.c_str(), allocator);
        //value.AddMember("name", device->device.c_str(), allocator);
        value.AddMember("name", d_name, allocator);
        value.AddMember("ip", d_ip, allocator);
        value.AddMember("port", d_port, allocator);
        value.AddMember("ssrc", d_ssrc, allocator);
        value.AddMember("rtsp", d_rtsp_url, allocator);
        arr_device.PushBack(value, allocator);
    }
    doc.AddMember("device_list", arr_device, allocator);
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    device_list = buffer.GetString();
    CLOGI(BLUE, "device_list:%s", device_list.c_str());
    return resp->String(device_list);
}

int XHttpServer::start_rtsp_publish(HttpRequest* req, HttpResponse* resp)
{
    std::string device = req->GetParam("device");
    if (device.empty()) {
        return resp->String(get_simple_info(400, "can not find param device!"));
    }
    auto client_ptr = Server::instance()->FindClient(device);
    if (!client_ptr) {
        return resp->String(get_simple_info(101, "can not find the device client"));
    }
    auto req_ptr = std::make_shared<ClientRequest>();
    req_ptr->client_ptr = client_ptr;
    req_ptr->req_type = kRequestTypeInvite;
    Server::instance()->AddRequest(req_ptr);
    return resp->String(get_simple_info(200, "ok"));
}

int XHttpServer::stop_rtsp_publish(HttpRequest* req, HttpResponse* resp)
{
    std::string device_list;
    rapidjson::Document doc(rapidjson::kObjectType);    // doc.SetObject();
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
    std::vector<ClientPtr> vec_device;
    const auto& clients = Server::instance()->GetClients();
    rapidjson::Value arr_device(rapidjson::kArrayType);
    for (const auto& iter : clients) {
        ClientPtr device = iter.second;
        rapidjson::Value value(rapidjson::kObjectType);
        rapidjson::Value d_name(rapidjson::kStringType);
        rapidjson::Value d_ip(rapidjson::kStringType);
        rapidjson::Value d_port(rapidjson::kNumberType);
        rapidjson::Value d_ssrc(rapidjson::kStringType);
        rapidjson::Value d_rtsp_url(rapidjson::kStringType);
        d_name.SetString(device->device.c_str(), allocator);
        d_ip.SetString(device->ip.c_str(), allocator);
        d_port.SetInt(device->port);
        d_ssrc.SetString(device->ssrc.c_str(), allocator);
        d_rtsp_url.SetString(device->rtsp_url.c_str(), allocator);
        //value.AddMember("name", device->device.c_str(), allocator);
        value.AddMember("name", d_name, allocator);
        value.AddMember("ip", d_ip, allocator);
        value.AddMember("port", d_port, allocator);
        value.AddMember("ssrc", d_ssrc, allocator);
        value.AddMember("rtsp", d_rtsp_url, allocator);
        arr_device.PushBack(value, allocator);
    }
    doc.AddMember("device_list", arr_device, allocator);
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    device_list = buffer.GetString();
    CLOGI(BLUE, "device_list:%s", device_list.c_str());
    return resp->String(device_list);
}

int XHttpServer::on_publish(HttpRequest* req, HttpResponse* resp)
{
    req->query_params;
    CLOGI(CYAN, "http on publish!!!-------------------------------------------------------------------");
    return 0;
}


int XHttpServer::on_play(HttpRequest* req, HttpResponse* resp)
{
    CLOGI(RED, "------------------------------------------http on play---------------------------------");
    resp->json["code"] = 0; // 鉴权成功
    resp->json["msg"] = "success";
    return 200; // http调用成功
}
};