/**
 * @file singleton.h
 * @author sleeping (csleepign@163.com)
 * @brief 单例
 * @version 0.1
 * @date 2023-08-14
 * 
 * @copyright Copyright (c) 2023
 * 
 */


#include <mutex>

namespace Xzm {
namespace util {

//extern std::once_flag oflag;

template<typename T>
class Singleton
{
public:
    Singleton()
    {

    }
    virtual ~Singleton()
    {

    }
    Singleton(Singleton&) = delete;
    Singleton(Singleton&&) = delete;

    static T* instance()
    {
        static T instance_;
        return &instance_;
    }
    
};
};
};