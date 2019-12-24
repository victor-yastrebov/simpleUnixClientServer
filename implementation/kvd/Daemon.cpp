/**
 *
 * @author <va.yastrebov>
 * @description
 * <pre>
 *    Class is responsible for demonisation of application
  * </pre>
 *
 * @class Daemon
 */

#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<sys/stat.h>
#include<syslog.h>

#include"Daemon.h"

/**
 * CTOR
 */
Daemon::Daemon() :
   isDaemon( false )
{

}

/**
 * DTOR
 */
Daemon::~Daemon()
{
   if( isDaemon )
   {
      // syslog (LOG_NOTICE, "KVD service terminated");
      // closelog();
   }
}

/**
 * Run applicaton in daemon state
 */
bool Daemon::Daemonise()
{
   pid_t pid;

   // Fork off the parent process
   pid = fork();
   if( pid < 0 )
   {
      std::cout << "Error: failed to fork for the first time" << std::endl;
      return false;
   }

   // On success let the parent terminate
   if( pid > 0 )
   {
      return false;
   }

   // On success the child process becomes session leader
   if( setsid() < 0 )
   {
      std::cout << "Error: setsid failed" << std::endl;
      return false;
   }

   // For right now we just ignore the signals
   signal( SIGCHLD, SIG_IGN );
   signal( SIGHUP, SIG_IGN );

   // Fork off for the second time
   pid = fork();
   if( pid < 0 )
   {
      std::cout << "Error: failed to fork for the second time" << std::endl;
      return false;
   }

   // On success let the parent terminate
   if( pid > 0 )
   {
      return false;
   }

   // Set new file permissions
   umask( 0 );

   // Change the working directory to the root directory
   chdir( "/" );

   // Close all open file descriptors
   int x;
   for( x = sysconf( _SC_OPEN_MAX ); x >= 0; x-- )
   {
      close( x );
   }

   // Open the log file
   openlog( "kvdservice", LOG_PID, LOG_DAEMON );

   isDaemon = true;
   return true;
}