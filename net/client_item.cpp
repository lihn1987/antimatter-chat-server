#include "client_item.hpp"
#include "log/log.hpp"
#include "tool/string_tools.hpp"
#include "msg.hpp"
ClientItem::ClientItem(
    boost::asio::io_context& ioc, 
    std::function<void(std::shared_ptr<ClientItem>, const boost::system::error_code&)> on_error_cb,
    std::function<void(std::shared_ptr<ClientItem>, std::shared_ptr<::google::protobuf::Message>)> on_read_cb, 
    std::function<void(std::shared_ptr<ClientItem>)> on_write_cb, 
    std::function<void(std::shared_ptr<ClientItem>)> on_connect_cb):
    ioc(ioc), socket(ioc),on_error(on_error_cb),on_read(on_read_cb),on_write(on_write_cb),on_connect(on_connect_cb){

}

ClientItem::ClientItem(
    boost::asio::io_context& ioc, 
    boost::asio::ip::tcp::endpoint ep, 
    std::function<void(std::shared_ptr<ClientItem>, const boost::system::error_code&)> on_error_cb, 
    std::function<void(std::shared_ptr<ClientItem>, std::shared_ptr<::google::protobuf::Message>)> on_read_cb, 
    std::function<void(std::shared_ptr<ClientItem>)> on_write_cb, 
    std::function<void(std::shared_ptr<ClientItem>)> on_connect_cb):ioc(ioc), socket(ioc),on_error(on_error_cb),on_read(on_read_cb),on_write(on_write_cb),on_connect(on_connect_cb){
        socket.async_connect(ep, std::bind(&ClientItem::OnConnect, this, std::placeholders::_1));
}

ClientItem::~ClientItem(){
    Stop();
}

void ClientItem::Start(){
    buffer_read_tmp.resize(1024*1024*10);
    buffer_write_tmp.resize(1024*1024*10);
    std::cout<<shared_from_this().get()<<std::endl;
    socket.async_read_some(boost::asio::buffer(buffer_read_tmp), std::bind(&ClientItem::OnRead, shared_from_this(),  std::placeholders::_1, std::placeholders::_2));
    //boost::asio::buffer dd;
}

void ClientItem::Stop(){
    LogDebug("client stop");
    try{
        socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
        socket.close();
    }catch(...){}
}

void ClientItem::Write(std::shared_ptr<::google::protobuf::Message> msg){
    std::lock_guard<std::mutex> lk(mutex_buffer_write);
    std::string buf = Msg::EncodeProtobuf2String(msg);
    buffer_write.append(buf);
    if(buf.size() == buffer_write.size()){
        socket.async_write_some(boost::asio::buffer(buf), std::bind(&ClientItem::OnWrite, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }
}

bool ClientItem::IsOpen(){
    return socket.is_open();
}

boost::asio::ip::tcp::endpoint ClientItem::GetLocalEndpoint(){
    return socket.local_endpoint();
}

boost::asio::ip::tcp::endpoint ClientItem::GetRemoteEndpoint(){
    return socket.remote_endpoint();
}

boost::asio::ip::tcp::socket& ClientItem::GetSocket(){
    return socket;
}
void ClientItem::OnError(const boost::system::error_code& error){

}

void ClientItem::OnRead(const boost::system::error_code& error, std::size_t bytes_transferred){
    if(error){
        LogWarning("OnReadError:"+ error.message());
        if(on_error){
            on_error(shared_from_this(), error);
        }
        return;
    }else{
        LogDebug("Readed "<<bytes_transferred<<" bytes")
        buffer_read.append(buffer_read_tmp, 0, bytes_transferred);
        while(auto msg = Msg::DecodeString2Protobuf(buffer_read)){
            if(on_read) on_read(shared_from_this(), msg);
        }
        //if(on_read) on_read(shared_from_this(), std::string(buffer_read_tmp, 0, bytes_transferred));
        socket.async_read_some(boost::asio::buffer(buffer_read_tmp), std::bind(&ClientItem::OnRead, shared_from_this(),  std::placeholders::_1, std::placeholders::_2));
        LogDebug("Readed "<<bytes_transferred<<" bytes");
    }
}

void ClientItem::OnWrite(const boost::system::error_code& error, std::size_t bytes_transferred){
    if(!error){
        std::lock_guard<std::mutex> lk(mutex_buffer_write);
        buffer_write.erase(0, bytes_transferred);
        if(buffer_write.size()){
            socket.async_write_some(boost::asio::buffer(buffer_write), std::bind(&ClientItem::OnWrite, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
        }
    }else{
        if(on_error)on_error(shared_from_this(), error);
    }
}
void ClientItem::OnConnect(const boost::system::error_code& error){
    if(error ){
        if(on_error)on_error(shared_from_this(), error);
        LogWarning("Socket connect error! "<<error.message());
    }else{
        if(on_connect)on_connect(shared_from_this());
        LogInfo("Socket connected! "<<error.message());
        Start();
    }
}