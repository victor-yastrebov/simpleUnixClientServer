/**
 *
 * @author <va.yastrebov>
 * @description
 * <pre>
 *    Class is responsible logging to syslog
  * </pre>
 *
 * @class SysLogger
 */

#pragma once

#include<syslog.h>
#include<sstream>
#include<iostream>

class SysLogger
{
public:
   SysLogger() = default;
  ~SysLogger();

   template<typename...Ts>
   void LogToSyslog( Ts&&... rest )
   {
      LogToSyslogImpl( "[ksvdservice db] ", std::forward<Ts>( rest )... );
      std::string s_msg = ss.str();
      std::cout << s_msg << std::endl;
      syslog( LOG_NOTICE, s_msg.c_str() );
      ss.str( "" );
   }

private:
   std::stringstream ss;

   template <typename T>
   void LogToSyslogImpl( T t )
   {
      ss << t;
   }

   template <typename T, typename...Ts>
   void LogToSyslogImpl( T &&first, Ts&&... rest )
   {
       ss << first << " ";
       LogToSyslogImpl( std::forward<Ts>( rest )... );
   }
};
