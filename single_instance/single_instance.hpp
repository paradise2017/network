#include <memory>
#include <mutex>
#include <iostream>
using namespace std;
template <typename T>
class Singleton
{
protected:
    Singleton() = default;
    Singleton(const Singleton<T> &) = delete;
    Singleton &operator=(const Singleton<T> &st) = delete;
    static std::shared_ptr<T> _instance;

public:
    static std::shared_ptr<T> GetInstance()
    {
        // 只会被初始化一次
        static std::once_flag s_flag;
        // 检测是否被调用
        std::call_once(s_flag, [&]()
                       { _instance = shared_ptr<T>(new T); });
        return _instance;
    }
    void PrintAddress()
    {
        std::cout << _instance.get() << endl;
    }
    ~Singleton()
    {
        std::cout << "this is singleton destruct" << std::endl;
    }
};
template <typename T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;