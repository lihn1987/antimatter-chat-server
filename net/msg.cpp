#include "msg.hpp"
#include "base.pb.h"

std::unordered_map<std::string, int> Msg::msgtype_string2int_map;
std::unordered_map<int, std::string> Msg::msgtype_int2string_map;
std::unordered_map<int, std::function<std::shared_ptr<::google::protobuf::Message>()>> Msg::msgtype_int2func_map;
#define CreateInt2Func(arg) \
{\
    MSG_##arg,\
    std::bind([]()->std::shared_ptr<::google::protobuf::Message>{\
            return std::shared_ptr<net::arg>(new net::arg());\
        })\
}



inline void Msg::InitMsgType(){
    if(msgtype_string2int_map.size())return;
    msgtype_string2int_map = {
        {"net.Ping", MSG_Ping},
        {"net.Pong", MSG_Pong},
        {"net.LoginRequest", MSG_LoginRequest},
        {"net.LoginResponse", MSG_LoginResponse},
        {"net.TextMessageRequest", MSG_TextMessageRequest},
        {"net.TextMessageresponse", MSG_TextMessageresponse}
    };
    for(auto item : msgtype_string2int_map){
        msgtype_int2string_map[item.second] = item.first;
    }

    msgtype_int2func_map = {
        CreateInt2Func(Ping),
        CreateInt2Func(Pong),
        CreateInt2Func(LoginRequest),
        CreateInt2Func(LoginResponse),
        CreateInt2Func(TextMessageRequest),
        CreateInt2Func(TextMessageresponse)
    };
         
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

    if(msgtype_int2func_map.find(type) != msgtype_int2func_map.end()){
        rtn = msgtype_int2func_map[type]();
        if(!rtn->ParseFromString(msg.substr(sizeof(len)+sizeof(type), len - sizeof(type)))){
            msg.erase(0, sizeof(len)+len);
            return std::shared_ptr<::google::protobuf::Message>();
        }
    }
    msg.erase(0, sizeof(len)+len);
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
