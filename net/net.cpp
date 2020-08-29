#include "net.hpp"
#include "log/log.hpp"
AntNet::AntNet(boost::asio::io_context& ioc, std::shared_ptr<Config> config_in):ioc(ioc), config(config_in),server(ioc, config){}
AntNet::~AntNet(){}
void AntNet::Start(){
    server.Start();
}

