#include<stdio.h>
#include<cstdlib>

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

   return EXIT_SUCCESS;
}