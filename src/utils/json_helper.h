/**
 * @file json_helper.h
 * @author sleeping (csleeping@163.com)
 * @brief json解析帮助类
 * @version 0.1
 * @date 2023-08-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once
#include "../include/rapidjson/document.h"
#include "../include/rapidjson/writer.h"
#include "../include/rapidjson/stringbuffer.h"

namespace Xzm {
namespace util{

#define JSON_PARSE_BOOL(doc, json_str)  \
    if (doc.Parse(json_str).HasParseError()) {  \
        return false;                   \
    }

#define JSON_VALUE_REQUIRE_INT(doc, from, to)           \
    if (!doc.HasMember(from) || !doc[from].IsInt()) {   \
        return false;                                   \
    } else {                                            \
        to = doc[from].GetInt();                        \
    }

#define JSON_VALUE_OPTION_INT(doc, from, to, default_value) \
    to = default_value;                                     \
    if (doc.HasMember(from) && doc[from].IsInt()) {         \
        to = doc[from].GetInt();                            \
    }

#define JSON_VALUE_REQUIRE_STRING(doc, from, to)            \
    if (!doc.HasMember(from) || !doc[from].IsString()) {    \
        return false;                                       \
    } else {                                                \
        to = doc[from].GetString();                         \
    }

#define JSON_VALUE_OPTION_STRING(doc, from, to, default_value)  \
    to = default_value;                                         \
    if (doc.HasMember(from) && doc[from].IsString()) {          \
        to = doc[from].GetString();                             \
    }

#define JSON_VALUE_REQUIRE_BOOL(doc, from, to)                  \
    if (!doc.HasMember(from) || !doc[from].IsBool()) {          \
        return false;                                           \
    } else {                                                    \
        to = doc[from].GetBool();                               \
    }

#define JSON_VALUE_OPTION_BOOL(doc, from, to, default_value)    \
    to = default_value;                                         \
    if (doc.HasMember(from) && doc[from].IsBool()) {            \
        to = doc[from].GetBool();                               \
    }

};
};