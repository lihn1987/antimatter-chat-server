#include "string_tools.hpp" 
std::string String2Hex(std::string input, uint32_t len){
    std::string rtn_str;
    uint32_t str_size = (len == 0? input.size():len);
    for (uint32_t i = 0; i < str_size; ++i)
    {
        char tmp_value[4] = { 0 };
        sprintf(tmp_value, "%02X ", (unsigned char)(input.c_str()[i]));
        rtn_str += tmp_value;
    }
    return rtn_str;
}