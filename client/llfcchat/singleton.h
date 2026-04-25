#ifndef SINGLETON_H
#define SINGLETON_H
/******************************************************************************
 *
 * @file       singleton.h
 * @brief       单例模板类
 *
 * @author     恋恋风辰
 * @date       2024/02/28
 * @history
 *****************************************************************************/
#include <memory>
#include <mutex>
#include <iostream>
#include "global.h"

using namespace std;
template <typename T>
class Singleton {
protected:
    Singleton() = default;
    Singleton(const Singleton<T>&) = delete;
    Singleton& operator=(const Singleton<T>& st) = delete;

    static std::shared_ptr<T> _instance;
public:
    static std::shared_ptr<T> GetInstance() {
        static std::once_flag s_flag;
        std::call_once(s_flag, [&]() {
            _instance = shared_ptr<T>(new T);
            });

        return _instance;
    }
    void PrintAddress() {
        std::cout << _instance.get() << endl;
    }
    ~Singleton() {
        std::cout << "this is singleton destruct" << std::endl;
    }
};

template <typename T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;
#endif // SINGLETON_H
