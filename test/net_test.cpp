#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/asio.hpp>
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <boost/bind.hpp>
#include "net/server.hpp"
#include "log/log.hpp"
#include "net/msg.hpp"
#include "base.pb.h"
BOOST_AUTO_TEST_SUITE(NetTest)

BOOST_AUTO_TEST_CASE(SampleTest){
    LogInfo("========================>");
    LogInfo("Start sample test");
    boost::asio::io_context ioc;
    std::shared_ptr<Config> server_config = std::make_shared<Config>();
    server_config->net_config->lession_port=1111;
    std::shared_ptr<Server> server = std::make_shared<Server>(ioc, server_config);
    std::vector<std::shared_ptr<std::thread>> thread_list;
    for(auto i = 0; i < std::thread::hardware_concurrency(); i++){
        thread_list.push_back(std::make_shared<std::thread>(boost::bind(&boost::asio::io_context::run, &ioc)));
    }
    ioc.stop();
    for(auto i = 0; i < std::thread::hardware_concurrency(); i++){
        thread_list[i]->join();
    }
    LogInfo("End sample test");
    BOOST_CHECK(1);
}

BOOST_AUTO_TEST_CASE(ConnectTest){
    LogInfo("=====================>");
    LogInfo("Start connect test");
    boost::asio::io_context ioc;

    //init server
    std::shared_ptr<Config> server_config = std::make_shared<Config>();
    server_config->net_config->lession_port=1111;
    std::shared_ptr<Server> server = std::make_shared<Server>(ioc, server_config);
    server->Start();

    //init clilent
    uint32_t socket_count = 10;
    std::list<std::shared_ptr<ClientItem>> item_list;
    for(auto i = 0; i < socket_count; i++){
        auto socket_item = std::make_shared<ClientItem>(ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), server_config->net_config->lession_port));
        item_list.push_back(socket_item);
    }
    std::vector<std::shared_ptr<std::thread>> thread_list;
    
    for(auto i = 0; i < std::thread::hardware_concurrency(); i++){
        thread_list.push_back(std::make_shared<std::thread>(boost::bind(&boost::asio::io_context::run, &ioc)));
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    //check client
    for(auto item : item_list){
        BOOST_CHECK(item->IsOpen() == true);
    }
    //check server
    std::list<std::shared_ptr<ClientItem>> server_client_list = server->GetClientList();
    for(auto item : server_client_list){
        BOOST_CHECK(item->IsOpen() == true);
    }
    for(auto item : item_list){
        item->Stop();
    }
    BOOST_CHECK(server_client_list.size() == socket_count);
    ioc.stop();
    for(auto i = 0; i < std::thread::hardware_concurrency(); i++){
        thread_list[i]->join();
    }
    LogInfo("End connect test");
}

BOOST_AUTO_TEST_CASE(ConnectAndDisconnectTest){
    LogInfo("====================>");
    LogInfo("Start connect test");
    boost::asio::io_context ioc;

    //init server
    std::shared_ptr<Config> server_config = std::make_shared<Config>();
    server_config->net_config->lession_port=1111;
    std::shared_ptr<Server> server = std::make_shared<Server>(ioc, server_config);
    server->Start();

    //init clilent
    uint32_t socket_count = 10;
    uint32_t socket_disconnect_count = 5;
    std::vector<std::shared_ptr<ClientItem>> item_list;
    for(auto i = 0; i < socket_count; i++){
        auto socket_item = std::make_shared<ClientItem>(ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), server_config->net_config->lession_port));
        item_list.push_back(socket_item);
    }

    for(auto i = 0; i < socket_disconnect_count; i++){
        item_list[i]->Stop();
    }
    
    std::vector<std::shared_ptr<std::thread>> thread_list;
    
    for(auto i = 0; i < std::thread::hardware_concurrency(); i++){
        thread_list.push_back(std::make_shared<std::thread>(boost::bind(&boost::asio::io_context::run, &ioc)));
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    //check client
    for(auto i = 0; i < socket_count; i++){
        if(i < socket_disconnect_count){
            BOOST_CHECK(!item_list[i]->IsOpen());
        }else{
            BOOST_CHECK(item_list[i]->IsOpen());
        }
    }
    //check server
    std::list<std::shared_ptr<ClientItem>> server_client_list = server->GetClientList();
    BOOST_CHECK(server_client_list.size() == (socket_count - socket_disconnect_count));

    for(auto item : item_list){
        item->Stop();
    }

    ioc.stop();
    for(auto i = 0; i < std::thread::hardware_concurrency(); i++){
        thread_list[i]->join();
    }
    LogInfo("End connect test");
}


BOOST_AUTO_TEST_CASE(ReadWriteTest){
    LogInfo("===========================>");
    LogInfo("Start connectiong rw test");
    boost::asio::io_context ioc;
    bool client_ping_readed = false;
    bool client_pong_readed = false;

    //init server
    std::shared_ptr<Config> server_config = std::make_shared<Config>();
    server_config->net_config->lession_port=1111;
    std::shared_ptr<Server> server = std::make_shared<Server>(ioc, server_config);
    server->Start();
    //init client
    auto socket_item = std::make_shared<ClientItem>(
        ioc, 
        boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), server_config->net_config->lession_port),
        [](std::shared_ptr<ClientItem>, const boost::system::error_code&){},
        [&client_ping_readed, &client_pong_readed](std::shared_ptr<ClientItem> client, std::shared_ptr<::google::protobuf::Message> msg){
            LogInfo("Received a message: "<<msg->GetTypeName());
            switch(Msg::GetMsgType(msg->GetTypeName())){
            case MSG_PING:
                client_ping_readed = true;
                break;
            case MSG_PONG:
                client_pong_readed = true;
                break;
            }
        });

    //start io_context
    std::vector<std::shared_ptr<std::thread>> thread_list;
    for(auto i = 0; i < std::thread::hardware_concurrency(); i++){
        thread_list.push_back(std::make_shared<std::thread>(boost::bind(&boost::asio::io_context::run, &ioc)));
    }
    //start test
    LogInfo("Start deal connection");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    LogInfo("Start message send");
    std::shared_ptr<net::Ping> ping = std::shared_ptr<net::Ping>(new net::Ping());
    ping->set_version(0x00000001);
    socket_item->Write(ping);
    server->Write(ping);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    LogInfo("End message send");
    BOOST_CHECK(client_ping_readed);
    BOOST_CHECK(client_pong_readed);
    socket_item->Stop();
    
    //end io_context
    ioc.stop();
    for(auto i = 0; i < std::thread::hardware_concurrency(); i++){
        thread_list[i]->join();
    }
    LogInfo("End connectiong rw");
}

BOOST_AUTO_TEST_SUITE_END()