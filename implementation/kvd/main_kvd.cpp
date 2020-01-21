#include<iostream>
#include<algorithm>

#include"asio.hpp"
#include"UDSServer.h"
#include"DataBase.h"

#if defined(ASIO_HAS_LOCAL_SOCKETS)

int main()
{
   try
   {
      const std::string s_socket_file( "/tmp/server.sock" );
      std::remove( s_socket_file.c_str() );

      UDSServer s( s_socket_file );
      s.Run();
   }
   catch( std::exception& e )
   {
      std::cout << "Exception: " << e.what() << std::endl;
   }
   catch( ... )
   {
      std::cout << "Unknown exception" << std::endl;
   }

   return 0;
}

#else // defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
# error Local sockets not available on this platform.
#endif // defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)