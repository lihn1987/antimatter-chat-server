#ifndef __CONFIG__HPP__
#define __CONFIG__HPP__

#include <string>
#include <memory>

struct NetConfig{
    uint16_t lession_port = 9987;
    std::string peer_addr = "";
};
class Config{
public:
    Config();
    ~Config();
    static std::shared_ptr<Config> GetInstance(int argc = 0, char** argv = nullptr);
private:
    Config(int argc, char** argv);
public:
    std::shared_ptr<NetConfig> net_config;
    static std::shared_ptr<Config> instance;
};

#endif
