#include "helper.h"

namespace Xzm {
namespace util {
    bool read_file(const std::string& file_path, std::string& content)
    {
        std::fstream f(file_path, std::ios::in);
        if (!f.is_open()) {
            return false;
        }
        std::copy(std::istream_iterator<unsigned char>(f),
         std::istream_iterator<unsigned char>(), std::back_inserter(content));
        return true;
    }
};
};