// #include<iostream>
#include<algorithm>

#include"asio.hpp"
#include"UDSServer.h"
#include"DataBase.h"
#include"Daemon.h"
#include"SysLogger.h"

#if defined(ASIO_HAS_LOCAL_SOCKETS)

int main()
{
   std::shared_ptr<SysLogger> p_logger;

   try
   {
      p_logger = std::make_shared<SysLogger>();

      Daemon daemon( p_logger );
      if( false == daemon.Daemonise() )
      {
         return 0;
      }

      const std::string s_socket_file( "/tmp/server.sock" );
      std::remove( s_socket_file.c_str() );

      UDSServer s( s_socket_file, p_logger );
      s.Run();
   }
   catch( std::exception& e )
   {
      p_logger->Log( "Exception caught: ", e.what() );
   }
   catch( ... )
   {
      p_logger->Log( "Unknown exception caught" );
   }

   p_logger->Log( "Server is stopped" );

   return 0;
}

#else // defined(ASIO_HAS_LOCAL_SOCKETS)
# error Local sockets are not available
#endif // defined(ASIO_HAS_LOCAL_SOCKETS)