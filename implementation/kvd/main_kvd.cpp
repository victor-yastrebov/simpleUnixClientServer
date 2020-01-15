#include<iostream>
#include<algorithm>

#include"asio.hpp"
#include"UDSServer.h"
#include"DataBase.h"

// #include<cstdio>
// #include<array>
// #include<memory>

#if defined(ASIO_HAS_LOCAL_SOCKETS)

int main(int argc, char* argv[])
{
/*
  DataBase db;
  std::string s_result;
  db.ProcessQuery( "put key2 valueIsSuperFastAgain" );
  db.ProcessQuery( "put key3 value3" );
  db.ProcessQuery( "put mykey4 value4" );
  std::cout << "Result is: " << 
     db.ProcessQuery( "list ke" ) << std::endl; 
  std::cout << "Result is: " << 
     db.ProcessQuery( "list" ) << std::endl; 

  return 1;
*/

/*/
  db.ProcessQuery( "put key2 valueIsSuperFastAgain" );
  s_result = db.ProcessQuery( "get key2" );
  std::cout << "Result is: " << s_result << std::endl; 

  db.ProcessQuery( "erase key2" );

  s_result = db.ProcessQuery( "get key2" );
  std::cout << "Result is: " << s_result << std::endl; 

  return  1;
*/

  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: stream_server <file>\n";
      std::cerr << "*** WARNING: existing file is removed ***\n";
      return 1;
    }

    asio::io_service io_service;

    std::remove( argv[1] );
    UDSServer s( io_service, argv[1] );

    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}

#else // defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
# error Local sockets not available on this platform.
#endif // defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
