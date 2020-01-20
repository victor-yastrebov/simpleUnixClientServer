#include<cstdlib>
#include<cstring>
#include<iostream>
#include<vector>

#include"asio.hpp"
#include"AppProtocol.h"

#if defined(ASIO_HAS_LOCAL_SOCKETS)

using asio::local::stream_protocol;

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

    std::remove( argv[1] );
    stream_protocol::socket s( io_service, stream_protocol::endpoint( argv[1] ) );

    asio::error_code ec;
    s.connect(stream_protocol::endpoint(argv[2]), ec);
    if( ec )
    {
       std::cout << "Connection to server failed. Try again later" << std::endl;
       return 1;
    }

    std::cout << "Enter message: ";
    std::string s_query;

    std::getline( std::cin, s_query );

    AppProtocol app_protocol;
    std::vector<BYTE> v_query =
       app_protocol.encodeMsg( s_query );

    asio::write( s, asio::buffer( v_query, v_query.size() ) );

    const size_t buf_size = 4;
    std::vector<BYTE> v_buf;
    v_buf.resize( buf_size );

    size_t bytes_read = 0;

    std::vector<BYTE> v_full_msg;

    while( !ec )
    {
       bytes_read = asio::read( s, asio::buffer( v_buf ), ec );
       // std::cout << "After read: " << bytes_read << " bytes" << std::endl;
       v_full_msg.insert( v_full_msg.end(),
          v_buf.begin(), v_buf.begin() + bytes_read );
    }

    bool status_ok = false;
    const std::string s_ans = app_protocol.decodeMsg(
       v_full_msg, status_ok );

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
