#include <iostream>
#include <boost/asio.hpp>
#include <functional>
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <boost/bind.hpp>
#include "net/net.hpp"
#include "config/config.hpp"
#include "net/client_item.hpp"

int main(int argc, char* argv[]){
    auto config = Config::GetInstance(argc, argv);
    boost::asio::io_context ioc;
    AntNet net(ioc, config);
    net.Start();
    std::vector<std::shared_ptr<std::thread>> thread_list;
    for(auto i = 0; i < std::thread::hardware_concurrency(); i++){
        thread_list.push_back(std::make_shared<std::thread>(boost::bind(&boost::asio::io_context::run, &ioc)));
    }
    getchar();
    ioc.stop();
    for(auto i = 0; i < std::thread::hardware_concurrency(); i++){
        thread_list[i]->join();
    }
    
    return 0;
}