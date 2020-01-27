/**
 *
 * @author <va.yastrebov>
 * @description
 * <pre>
 *    Class is responsible for logging to syslog and to cout
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
   /**
    * CTOR
    */
   SysLogger()
   {
      openlog( "kvdservice", LOG_PID, LOG_DAEMON );
   }

   /**
    * DTOR
    */
   ~SysLogger()
   {
      closelog();
   }

   /**
    * Do log both to std::cout and syslog
    */
   template<typename...Ts>
   void Log( Ts&&... params )
   {
      ss.str( "" );

      (( ss << params << " " ), ... );
      const std::string s_msg = ss.str();

      std::cout << s_msg << std::endl;
      syslog( LOG_NOTICE, s_msg.c_str() );
   }

private:
   std::stringstream ss;
};
