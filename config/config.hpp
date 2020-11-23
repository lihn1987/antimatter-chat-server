#ifndef __CONFIG__HPP__
#define __CONFIG__HPP__

#include <string>
#include <memory>

class Config{
public:
    ~Config();
    static std::shared_ptr<Config> GetInstance(int argc = 0, char** argv = nullptr);
    static std::shared_ptr<Config> instance;
private:
    Config(int argc, char** argv);
public:
    uint16_t lession_port = 9987;
};
std::shared_ptr<Config> inline GetConfig(){
    return Config::GetInstance();
}
#endif
