#ifndef __SERVER__HPP__
#define __SERVER__HPP__
#include <list>
#include <memory>
#include <thread>
#include <mutex>
#include <functional>
#include <boost/asio.hpp>
#include <google/protobuf/message.h>
#include "client_item.hpp"
#include "config/config.hpp"
class Server{
public:
    Server(boost::asio::io_context& ioc, 
        std::shared_ptr<Config> config_in, 
        std::function<void(std::shared_ptr<ClientItem>, std::shared_ptr<::google::protobuf::Message>)> on_read_cb = std::function<void(std::shared_ptr<ClientItem>, std::shared_ptr<::google::protobuf::Message>)>());
    ~Server();
public:
    void Start();
    void Write(std::shared_ptr<::google::protobuf::Message> msg);
    std::list<std::shared_ptr<ClientItem>> GetClientList();
private:
    void OnAccept(std::shared_ptr<ClientItem> new_item,
      const boost::system::error_code& error);
    void OnReadMsg(std::shared_ptr<ClientItem> new_item,
      std::shared_ptr<::google::protobuf::Message> msg);
    void OnError(std::shared_ptr<ClientItem> item);
private:
    std::shared_ptr<Config> config;
    boost::asio::io_context& ioc;
    boost::asio::io_service::work work;
    boost::asio::ip::tcp::acceptor acceptor;
    std::mutex client_list_mutex;
    std::list<std::shared_ptr<ClientItem>> client_list;
    std::function<void(std::shared_ptr<ClientItem> new_item, std::shared_ptr<::google::protobuf::Message> msg)> on_read;
};
#endif