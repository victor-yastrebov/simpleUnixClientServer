#include<iostream>
#include<algorithm>

#include"asio.hpp"
#include"UDSServer.h"

// #include<cstdio>
// #include<array>
// #include<memory>

#if defined(ASIO_HAS_LOCAL_SOCKETS)

int main(int argc, char* argv[])
{
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
