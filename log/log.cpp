#include "log.hpp"
bool lob_inited = false;
void InitLog(){
    if(!lob_inited){
        boost::log::add_file_log
        (
            boost::log::keywords::file_name = "sample_%N.log",
            boost::log::keywords::rotation_size = 10 * 1024 * 1024,
            boost::log::keywords::format = "[%TimeStamp%]: %Message%"
        );
        boost::log::add_console_log(std::clog, boost::log::keywords::format = "%TimeStamp%: %Message%");
        boost::log::add_common_attributes();
        lob_inited = true;
    }
}