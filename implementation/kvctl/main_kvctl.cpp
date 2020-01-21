#include<cstdlib>
#include<cstring>
#include<iostream>
#include<vector>

#include"asio.hpp"
#include"AppProtocol.h"

#if defined(ASIO_HAS_LOCAL_SOCKETS)

using asio::local::stream_protocol;

/**
 * Print application usage
 */
void printHelp()
{
   std::cout << "Client for database by va.yastrebov." << std::endl;
   std::cout << "Usage examples: " << std::endl;
   std::cout << " - PUT <key> <value>" << std::endl;
   std::cout << " - GET <key>" << std::endl;
   std::cout << " - ERASE [key]" << std::endl;
   std::cout << " - LIST [prefix]" << std::endl;
}

/**
 * Create query string from input CLI args
 */
std::string getQuery( const int argc, const char *const argv[] )
{
   std::stringstream ss;
   for( int i = 1; i < argc; ++i)
   {
      ss << argv[i] << " ";
   }

   std::string s_query = ss.str();
   if( argc > 1 )
   {
      // s_query.pop_back();
      s_query.resize( s_query.size() - 1 );
   }

   return s_query;
}

int main( int argc, char* argv[] )
{
  int ret_code = 0;

  try
  {
    const std::string s_query = getQuery( argc, argv );

    if( s_query.empty() )
    {
       printHelp();
       return 1;
    }

    asio::io_service io_service;
    const std::string s_server_sock_file( "/tmp/server.sock" );
    const std::string s_client_sock_file( "/tmp/client4.sock" );

    std::remove( s_client_sock_file.c_str() );
    stream_protocol::socket s( io_service, stream_protocol::endpoint( s_client_sock_file ) );

    asio::error_code ec;
    s.connect( stream_protocol::endpoint( s_server_sock_file.c_str() ), ec );
    if( ec )
    {
       std::cout << "Connection to server failed: " << ec << std::endl;
       return 1;
    }

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
       v_full_msg.insert( v_full_msg.end(),
          v_buf.begin(), v_buf.begin() + bytes_read );
    }

    bool status_ok = false;
    const std::string s_ans = app_protocol.decodeMsg(
       v_full_msg, status_ok );

    if( status_ok )
    {
       // it will be more accurate to pass query status in AppProtocol msg
       const std::string s_dummy_ok_pattern = "Query OK";
       if( s_ans != s_dummy_ok_pattern ) std::cout << s_ans << std::endl;

       const std::string s_failure_pattern = "kvctl:";
       const bool has_failure_pattern =
          ( 0 == s_ans.compare( 0, s_failure_pattern.size(), s_failure_pattern ) );

       ret_code = has_failure_pattern;
    }
    else
    {
       std::cout << "Recv wrong message" << std::endl;
       ret_code = 1;
    }

  }
  catch( std::exception& e )
  {
    std::cout << "Exception: " << e.what() << std::endl;
    ret_code = 1;
  }
  catch( ... )
  {
     std::cout << "Unknow exception type" << std::endl;
     ret_code = 1;
  }

  return ret_code;
}

#else // defined(ASIO_HAS_LOCAL_SOCKETS)
# error Local sockets not available on this platform.
#endif // defined(ASIO_HAS_LOCAL_SOCKETS)
