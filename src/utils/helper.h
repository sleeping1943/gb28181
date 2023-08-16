/**
 * @file helper.h
 * @author sleeping (csleeping@163.com)
 * @brief 一些工具函数
 * @version 0.1
 * @date 2023-08-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include <iterator>
#include <string>
#include <fstream>

namespace Xzm {
namespace util {
    bool read_file(const std::string& file_path, std::string& content);
};
};