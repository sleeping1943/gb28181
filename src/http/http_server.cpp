#include "http_server.h"
#include "../include/rapidjson/document.h"
#include "../utils/json_helper.h"
#include "../utils/helper.h"
#include "../utils/log.h"
#include <functional>
#include <hv/HttpServer.h>
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
    /*
        [] () {
        return resp->String(
        "{" \
        "    \"device\":["  \
        "        {" \
        "            \"name\":\"3420000100000001\"" \
        "        }" \
        "    ]" \
        "}");
    });
    */

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
        d_name.SetString(device->device.c_str(), allocator);
        //value.AddMember("name", device->device.c_str(), allocator);
        value.AddMember("name", d_name, allocator);
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

};