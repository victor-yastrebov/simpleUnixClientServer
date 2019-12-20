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

   UDSServer server;
   server.StartProcessing();

   syslog( LOG_NOTICE, "Before end of main" );

   return EXIT_SUCCESS;
}