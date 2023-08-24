/**
 * @file log.h
 * @author sleeping (csleeping@163.com)
 * @brief 日志记录
 * @version 0.1
 * @date 2023-08-17
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once
#include <time.h>
#include <string>

#define BLACK   "\033[30m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define PURPLE  "\033[35m"
#define CYAN    "\033[36m"

static std::string getTime() {
    const char* time_fmt = "%Y-%m-%d %H:%M:%S";
    time_t t = time(nullptr);
    char time_str[64];
    strftime(time_str, sizeof(time_str), time_fmt, localtime(&t));

    return time_str;
}
//  __FILE__ 获取源文件的相对路径和名字
//  __LINE__ 获取该行代码在文件中的行号
//  __func__ 或 __FUNCTION__ 获取函数名

#define CLOGI(color, format, ...)  fprintf(stderr,"%s[INFO]%s [%s:%d %s()] " format "\033[0m\n", color, getTime().data(),__FILE__,__LINE__,__func__ ,##__VA_ARGS__)
#define CLOGE(color, format, ...)  fprintf(stderr,"%s[ERROR]%s [%s:%d %s()] " format "\033[0m]\n", color, getTime().data(),__FILE__,__LINE__,__func__ ,##__VA_ARGS__)

#define LOGI(format, ...)  fprintf(stderr,"\033[32m[INFO]%s [%s:%d %s()] " format "\033[0m\n", getTime().data(),__FILE__,__LINE__,__func__ ,##__VA_ARGS__)
#define LOGE(format, ...)  fprintf(stderr,"\033[31m[ERROR]%s [%s:%d %s()] " format "\033[0m]\n",getTime().data(),__FILE__,__LINE__,__func__ ,##__VA_ARGS__)