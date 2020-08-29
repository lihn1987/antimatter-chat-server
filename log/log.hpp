#ifndef __LOG__HPP__
#define __LOG__HPP__
#include <boost/log/trivial.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <string>
extern bool lob_inited;
void InitLog();
inline void LogTrace(const std::string& log){
    InitLog();
    BOOST_LOG_TRIVIAL(trace) << log;
}

#define LogDebug(log) \
    InitLog();\
    BOOST_LOG_TRIVIAL(debug) <<"["<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<"]:"<<log;


#define LogInfo(log)\
    InitLog();\
    BOOST_LOG_TRIVIAL(info) << "["<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<"]:"<<log;


#define LogWarning(log)\
    InitLog();\
    BOOST_LOG_TRIVIAL(warning) << "["<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<"]:"<<log;


#define LogError(log)\
    InitLog();\
    BOOST_LOG_TRIVIAL(error) << "["<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<"]:"<<log;


#define LogFatal(log)\
    InitLog();\
    BOOST_LOG_TRIVIAL(fatal) << "["<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<"]:"<<log;

#endif