#include "msg.hpp"
#include "base.pb.h"

std::unordered_map<std::string, int> Msg::msgtype_string2int_map;
std::unordered_map<int, std::string> Msg::msgtype_int2string_map;

inline void Msg::InitMsgType(){
    if(msgtype_string2int_map.size())return;
    msgtype_string2int_map = {
        {"net.Ping", MSG_PING},
        {"net.Pong", MSG_PONG}
    };
    for(auto item : msgtype_string2int_map){
        msgtype_int2string_map[item.second] = item.first;
    }
}

std::string Msg::EncodeProtobuf2String(std::shared_ptr<::google::protobuf::Message> msg){
    std::string rtn;
    uint32_t len;
    uint32_t type;
    std::string content;
    InitMsgType();
    msg->SerializeToString(&content);
    len = content.size()+sizeof(type);
    type = GetMsgType(msg->GetTypeName());
    if(type==0){
        return rtn;
    }

    rtn.resize(len+4);
    void* ptr = memcpy((void*)rtn.c_str(), &len, sizeof(len));
    ptr = (uint8_t*)ptr + sizeof(len);
    memcpy(ptr, &type, sizeof(type));
    ptr = (uint8_t*)ptr + sizeof(type);
    memcpy(ptr, content.c_str(),content.size());
    return rtn;
}

std::shared_ptr<::google::protobuf::Message> Msg::DecodeString2Protobuf(std::string& msg){
    InitMsgType();
    if(msg.size() < 8){
        return std::shared_ptr<::google::protobuf::Message>();
    }
    uint32_t len;
    uint32_t type;
    memcpy(&len, msg.c_str(), sizeof(len));
    memcpy(&type, msg.c_str()+sizeof(len), sizeof(type));
    if(msg.size()-4 < len){
        return std::shared_ptr<::google::protobuf::Message>();
    }
    std::shared_ptr<::google::protobuf::Message> rtn;
    switch(type){
    case MSG_PING:
        rtn = std::shared_ptr<net::Ping>(new net::Ping());
        if(!rtn->ParseFromString(msg.substr(8, len-8))){
            msg.erase(0, sizeof(len)+len);
            return std::shared_ptr<::google::protobuf::Message>();
        }
        msg.erase(0, sizeof(len)+len);
        break;
    case MSG_PONG:
        rtn = std::shared_ptr<net::Pong>(new net::Pong());
        if(!rtn->ParseFromString(msg.substr(8, len-8))){
            msg.erase(0, sizeof(len)+len);
            return std::shared_ptr<::google::protobuf::Message>();
        }
        msg.erase(0, sizeof(len)+len);
        break;
    default:
        break;
    }
    return rtn;
    
    
}

uint32_t Msg::GetMsgType(const std::string& msg_type){
    InitMsgType();
    if(msgtype_string2int_map.find(msg_type) != msgtype_string2int_map.end()){
        return msgtype_string2int_map[msg_type];
    }
    return 0;
}

std::string Msg::GetMsgType(uint32_t msg_type){
    InitMsgType();
    if(msgtype_int2string_map.find(msg_type) != msgtype_int2string_map.end()){
        return msgtype_int2string_map[msg_type];
    }
    return "";
}
