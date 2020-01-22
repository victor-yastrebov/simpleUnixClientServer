/**
 *
 * @author <va.yastrebov>
 * @description
 * <pre>
 *    Class is responsible for logging to syslog
  * </pre>
 *
 * @class SysLogger
 */

#pragma once

#include<sstream>
#include<iostream>
#include<syslog.h>

class SysLogger
{
public:
   SysLogger()
   {
      /* Open the log file */
      openlog( "kvdservice", LOG_PID, LOG_DAEMON );
   }

  ~SysLogger()
   {
     closelog();
   }

   template<typename...Ts>
   void Log( Ts&&... rest )
   {
      LogImpl( "[ksvdservice db] ", std::forward<Ts>( rest )... );
      std::string s_msg = ss.str();
      std::cout << s_msg << std::endl;
      syslog( LOG_NOTICE, s_msg.c_str() );
      ss.str( "" );
   }

private:
   std::stringstream ss;

   template <typename T>
   void LogImpl( T t )
   {
      ss << t;
   }

   // void LogToSyslog() {}

   template <typename T, typename...Ts>
   void LogImpl( T &&first, Ts&&... rest )
   {
       ss << first << " ";
       LogImpl( std::forward<Ts>( rest )... );
   }
};