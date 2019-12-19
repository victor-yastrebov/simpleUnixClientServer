#include <stdio.h>
#include <cstdlib>

#include "Daemon.h"

int main()
{
   Daemon daemon;
   if( false == daemon.Daemonise() )
   {
      return EXIT_SUCCESS;
   }

   daemon.StartProcessing();

   return EXIT_SUCCESS;
}

