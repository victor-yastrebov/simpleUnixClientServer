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
#include<unistd.h>
#include<signal.h>
#include<sys/stat.h>
#include<syslog.h>

#include"Daemon.h"

/**
 * CTOR
 */
Daemon::Daemon( std::shared_ptr<SysLogger> &p_logger ) :
   pLogger( p_logger )
{

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
      pLogger->Log( "Daemonise error: failed to fork for the first time" );
      return false;
   }

   // Let the parent terminate
   if( pid > 0 )
   {
      return false;
   }

   // The child process becomes session leader
   if( setsid() < 0 )
   {
      pLogger->Log( "Daemonise error: setsid failed" );
      return false;
   }

   // Catch, ignore and handle signals
   signal( SIGCHLD, SIG_IGN );
   signal( SIGHUP, SIG_IGN );

   // Fork off for the second time
   pid = fork();
   if( pid < 0 )
   {
      pLogger->Log( "Daemonise error: failed to fork for the second time" );
      return false;
   }

   // Let the parent terminate
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

   return true;
}