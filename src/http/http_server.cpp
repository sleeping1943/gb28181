#include "http_server.h"
#include "../include/rapidjson/document.h"
#include "../utils/json_helper.h"
#include "../utils/helper.h"
#include <hv/HttpServer.h>

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

    router.GET("/query_device", [] (HttpRequest* req, HttpResponse* resp) {
        return resp->String(
        "{" \
        "    \"device\":["  \
        "        {" \
        "            \"name\":\"3420000100000001\"" \
        "        }" \
        "    ]" \
        "}");
    });


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
};