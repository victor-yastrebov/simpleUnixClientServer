#include<stdio.h>
#include<cstdlib>
#include<syslog.h>

#include"Daemon.h"
#include"UDSServer.h"

int main()
{
   Daemon daemon;
   if( false == daemon.Daemonise() )
   {
      return EXIT_SUCCESS;
   }

   const std::string s_path_to_db( "/tmp/kvd_db.txt" );
   UDSServer server;
   const int ret = server.startProcessing( s_path_to_db );

   syslog( LOG_NOTICE, "Start processing has finished" );

   return ret;
}