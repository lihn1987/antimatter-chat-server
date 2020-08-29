#ifndef __CLIENT__ITEM__HPP__
#define __CLIENT__ITEM__HPP__
#include <memory>
#include <string>
#include <mutex>
#include <functional>
#include <boost/asio.hpp>
#include <google/protobuf/message.h>

class ClientItem:public std::enable_shared_from_this<ClientItem>{
public:
    ClientItem(
        boost::asio::io_context& ioc, 
        std::function<void(std::shared_ptr<ClientItem>, const boost::system::error_code&)> on_error_cb = std::function<void(std::shared_ptr<ClientItem>, const boost::system::error_code&)>(),
        std::function<void(std::shared_ptr<ClientItem>, std::shared_ptr<::google::protobuf::Message>)> on_read_cb = std::function<void(std::shared_ptr<ClientItem>, std::shared_ptr<::google::protobuf::Message>)>(), 
        std::function<void(std::shared_ptr<ClientItem>)> on_write_cb = std::function<void(std::shared_ptr<ClientItem>)>(), 
        std::function<void(std::shared_ptr<ClientItem>)> on_connect_cb = std::function<void(std::shared_ptr<ClientItem>)>());
    ClientItem(boost::asio::io_context& ioc, 
        boost::asio::ip::tcp::endpoint ep,
        std::function<void(std::shared_ptr<ClientItem>, const boost::system::error_code&)> on_error_cb = std::function<void(std::shared_ptr<ClientItem>, const boost::system::error_code&)>(), 
        std::function<void(std::shared_ptr<ClientItem>, std::shared_ptr<::google::protobuf::Message>)> on_read_cb = std::function<void(std::shared_ptr<ClientItem>, std::shared_ptr<::google::protobuf::Message>)>(), 
        std::function<void(std::shared_ptr<ClientItem>)> on_write_cb = std::function<void(std::shared_ptr<ClientItem>)>(), 
        std::function<void(std::shared_ptr<ClientItem>)> on_connect_cb = std::function<void(std::shared_ptr<ClientItem>)>());
    ~ClientItem();
public:
    void Start();
    void Stop();
    void Write(std::shared_ptr<::google::protobuf::Message> msg);
    bool IsOpen();
public:
    boost::asio::ip::tcp::endpoint GetLocalEndpoint();
    boost::asio::ip::tcp::endpoint GetRemoteEndpoint();
    boost::asio::ip::tcp::socket& GetSocket();
private:
    void OnError(const boost::system::error_code& error);
    void OnRead(const boost::system::error_code& error, std::size_t bytes_transferred);
    void OnWrite(const boost::system::error_code& error, std::size_t bytes_transferred);
    void OnConnect(const boost::system::error_code& error);
private:
    boost::asio::io_context& ioc;
    boost::asio::ip::tcp::socket socket;
    std::string buffer_read;
    std::string buffer_read_tmp;
    std::string buffer_write_tmp;
    std::string buffer_write;
    std::mutex mutex_buffer_write;
private:
    std::function<void(std::shared_ptr<ClientItem>, const boost::system::error_code& error)> on_error;
    std::function<void(std::shared_ptr<ClientItem>, std::shared_ptr<::google::protobuf::Message>)> on_read;
    std::function<void(std::shared_ptr<ClientItem>)> on_write;
    std::function<void(std::shared_ptr<ClientItem>)> on_connect;
};

#endif
