#include "helper.h"
#include <sstream>
#include <algorithm>
#include <random>

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


    /*
    y字段(ssrc)：由10位十进制整数组成的字符串，表示SSRC值
    第一位为0代表实况，为1则代表回放；
    第二位至第六位由监控域ID的第4位到第8位组成；
    最后4位为不重复的4个整数
    */
    std::string build_ssrc(bool is_alive, const std::string& realm)
    {
        if (realm.length() < 8) {
            return "";
        }
        static std::string str = "0123456789";
        std::stringstream ss;
        ss << (is_alive ? 0 : 1);
        ss << realm.substr(3, 5);
        std::mt19937 gen(std::random_device{}());
        std::shuffle(str.begin(), str.end(), gen);
        ss << str.substr(0, 4);
        return ss.str();
    }

    // 转换10进制为16进制
    std::string convert10to16(const int num)
    {
        std::string out;
        std::stringstream ss;
        ss << std::hex << num;
        ss >> out;
        std::transform(out.begin(), out.end(), out.begin(), ::toupper);
        return out;
    }

    std::string convert10to16(const std::string& num)
    {
        int index = 0;
        auto iter = num.begin();
        std::stringstream ss;
        while (iter != num.end() && *iter == '0') {
            ss << *iter;
            iter++;
            index++;
        }
        auto num_left = num.substr(index, num.length() - index);
        int num_10 = std::stoi(num_left);
        ss << convert10to16(num_10);
        return ss.str();
    }

    std::string get_rtsp_addr(const std::string& mip, const std::string& ssrc)
    {
        std::stringstream ss;
        ss << "rtsp://" << mip << "/rtp/" << ssrc;
        return ss.str();
    }
};
};