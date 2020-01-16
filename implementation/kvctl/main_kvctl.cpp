#include<cstdlib>
#include<cstring>
#include<iostream>

#include"asio.hpp"
#include"AppProtocol.h"

#if defined(ASIO_HAS_LOCAL_SOCKETS)

using asio::local::stream_protocol;

enum { max_length = 1024 };

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 3)
    {
      std::cerr << "Usage: stream_client <client_sock> <server_sock>\n";
      return 1;
    }

    asio::io_service io_service;

    std::remove(argv[1]);
    stream_protocol::socket s(io_service, stream_protocol::endpoint( argv[1] ));

    s.connect(stream_protocol::endpoint(argv[2]));

    using namespace std; // For strlen.
    std::cout << "Enter message: ";
    std::string s_query;

    std::getline( std::cin, s_query );

    AppProtocol app_protocol;
    std::vector<BYTE> v_query =
       app_protocol.encodeMsg( s_query );

    asio::write(s, asio::buffer(v_query, v_query.size()));

    char reply[max_length];
    asio::error_code ec;
    size_t bytes_read = 0;

    std::vector<byte> v_ans;

    while( !ec )
    {
       bytes_read = asio::read(s,
          asio::buffer(reply, max_length), ec);
       // append to v_ans
    }

    bool status_ok = false;
    const std::string s_ans = app_protocol.decodeMsg(
       std::vector<BYTE>( reply, reply  + bytes_read ), status_ok );

    if(status_ok)
    {
       std::cout << s_ans << std::endl;
    }
    else
    {
       std::cout << "Recv wrong message" << std::endl;
    }

  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}

#else // defined(ASIO_HAS_LOCAL_SOCKETS)
# error Local sockets not available on this platform.
#endif // defined(ASIO_HAS_LOCAL_SOCKETS)
