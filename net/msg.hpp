#include <stdint.h>
#include <string>
#include <memory>
#include <google/protobuf/message.h>
#include <unordered_map>
/*
    uint32_t len;
    uint32_t msg_type;
    std::string content;
*/
enum{
    MSG_START = 0x00000000,
    MSG_PING = 0x00000001,
    MSG_PONG = 0x00000002,
};
class Msg{
public:
    void static InitMsgType();
    std::string static EncodeProtobuf2String(std::shared_ptr<::google::protobuf::Message> msg);
    std::shared_ptr<::google::protobuf::Message> static DecodeString2Protobuf(std::string& msg);
    uint32_t static GetMsgType(const std::string& msg_type);
    std::string static GetMsgType(uint32_t msg_type);
public:
    static std::unordered_map<std::string, int> msgtype_string2int_map;
    static std::unordered_map<int, std::string> msgtype_int2string_map;
};