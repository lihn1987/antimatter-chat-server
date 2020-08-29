#include <functional>
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <boost/bind.hpp>
#include "server.hpp"
#include "log/log.hpp"
#include "msg.hpp"
#include "base.pb.h"
Server::Server(boost::asio::io_context& ioc, std::shared_ptr<Config> config_in,std::function<void(std::shared_ptr<ClientItem>, std::shared_ptr<::google::protobuf::Message>)> on_read_cb):
    config(config_in),
    ioc(ioc),
    work(ioc),
    acceptor(ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), config->net_config->lession_port), false),
    on_read(on_read_cb){
    LogDebug("Server create"<<this);
}

Server::~Server(){
    boost::system::error_code ec;
    acceptor.close(ec);
    {
        std::lock_guard<std::mutex> lk(client_list_mutex);
        for(auto client : client_list){
            client->Stop();
        }
    }
    LogDebug("Server desdroy"<<this);
}

void Server::Start(){
    std::shared_ptr<ClientItem> new_item = std::make_shared<ClientItem>(ioc, std::bind(&Server::OnError,this, std::placeholders::_1), std::bind(&Server::OnReadMsg, this, std::placeholders::_1, std::placeholders::_2));
    acceptor.async_accept(
        new_item->GetSocket(),
        std::bind(&Server::OnAccept, this, new_item,
        std::placeholders::_1));
}

void Server::Write(std::shared_ptr<::google::protobuf::Message> msg){
    std::lock_guard<std::mutex> lk(client_list_mutex);
    for(auto client_item : client_list){
        client_item->Write(msg);
    }
}

std::list<std::shared_ptr<ClientItem>> Server::GetClientList(){
    std::lock_guard<std::mutex> lk(client_list_mutex);
    return client_list;
}

void Server::OnAccept(std::shared_ptr<ClientItem> new_item, const boost::system::error_code& error){
    if(error)return;
    {
        std::lock_guard<std::mutex> lk(client_list_mutex);
        client_list.push_back(new_item);
        LogInfo("Receive a new connection, count "<<client_list.size())
    }
    new_item->Start();
    std::shared_ptr<ClientItem> item = std::make_shared<ClientItem>(ioc, std::bind(&Server::OnError,this, std::placeholders::_1), std::bind(&Server::OnReadMsg, this, std::placeholders::_1, std::placeholders::_2));
    acceptor.async_accept(
        item->GetSocket(),
        std::bind(&Server::OnAccept, this, item,
        std::placeholders::_1));
}
void Server::OnReadMsg(std::shared_ptr<ClientItem> new_item,
      std::shared_ptr<::google::protobuf::Message> msg){
    if(on_read)on_read(new_item, msg);
    if(msg->GetTypeName() == "net.Ping"){
        std::shared_ptr<net::Pong> pong = std::shared_ptr<net::Pong>(new net::Pong());
        pong->set_version(0x00000001);
        new_item->Write(pong);
    }
    
}
void Server::OnError(std::shared_ptr<ClientItem> item){
    {
        std::lock_guard<std::mutex> lk(client_list_mutex);
        for(auto iter = client_list.begin(); iter != client_list.end(); iter++){
            if(item == *iter){
                client_list.erase(iter);
                LogInfo("Remove a connection, count "<<client_list.size())
                break;
            }
        }
    }
}