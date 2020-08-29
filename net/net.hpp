#ifndef __NET__HPP__
#define __NET__HPP__
#include "server.hpp"
#include "config/config.hpp"
class AntNet{
public:
    AntNet(boost::asio::io_context& ioc ,std::shared_ptr<Config> config_in);
    ~AntNet();
public:
    void Start();
private:
    std::shared_ptr<Config> config;
    boost::asio::io_context& ioc;
    Server server;
};
#endif