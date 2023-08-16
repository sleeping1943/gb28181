#include "server.h"
#include "include/rapidjson/document.h"
#include "utils/json_helper.h"

namespace Xzm
{
Server::Server()
{

}

Server::~Server()
{

}

bool Server::SetServerInfo(const std::string& json_str)
{
    rapidjson::Document doc;
    JSON_PARSE_BOOL(doc, json_str.c_str());
    JSON_VALUE_REQUIRE_STRING(doc, "ua", s_info_.ua);
    JSON_VALUE_REQUIRE_STRING(doc, "nonce", s_info_.nonce);
    JSON_VALUE_REQUIRE_STRING(doc, "ip", s_info_.ip);
    JSON_VALUE_REQUIRE_INT(doc, "port", s_info_.port);
    JSON_VALUE_REQUIRE_INT(doc, "rtpPort", s_info_.rtp_port);
    JSON_VALUE_REQUIRE_STRING(doc, "sipId", s_info_.sid_id);
    JSON_VALUE_REQUIRE_STRING(doc, "sipRealm", s_info_.realm);
    JSON_VALUE_REQUIRE_STRING(doc, "sipPass", s_info_.passwd);
    JSON_VALUE_REQUIRE_INT(doc, "sipTimeout", s_info_.timeout);
    JSON_VALUE_REQUIRE_INT(doc, "sipExpiry", s_info_.valid_time);
    return true;
}
};