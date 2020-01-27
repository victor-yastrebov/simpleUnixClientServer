/**
 *
 * @author <va.yastrebov>
 * @description
 * <pre>
 *    Class is responsible for quering data from remote DB
  * </pre>
 *
 * @class UDSClient
 */

#include<vector>
#include<iostream>

#include"UDS.h"
#include"UDSClient.h"
#include"AppProtocol.h"

/**
 * CTOR
 */
UDSClient::UDSClient( const std::string &s_sock_file ) :
   sSockFile( s_sock_file ),
   mSocket( ioService, sSockFile )
{

}

/**
 * DTOR
 */
UDSClient::~UDSClient()
{
   asio::error_code ec;
   mSocket.shutdown( local_str_proto::socket::shutdown_both, ec);
   mSocket.close( ec );
}

/**
 * Try to connect to remote server
 */
int UDSClient::Connect( const std::string &s_server_sock_file )
{
   std::remove( sSockFile.c_str() );

   asio::error_code ec;
   mSocket.connect(
      local_str_proto::endpoint( s_server_sock_file.c_str() ), ec );
   if( ec )
   {
      std::cout << "Connection to server failed: " << ec << std::endl;
      return 0;
   }

   return 1;
}

/**
 * Start query session to DB
 */
int UDSClient::ProcSession( const std::string &s_query )
{
   AppProtocol app_protocol;
   std::vector<BYTE> v_query =
      app_protocol.EncodeMsg( s_query );

   asio::write( mSocket,
      asio::buffer( v_query, v_query.size() ) );

   const size_t buf_size = 4;
   std::vector<BYTE> v_buf;
   v_buf.resize( buf_size );

   size_t bytes_read = 0;

   std::vector<BYTE> v_full_msg;

   asio::error_code ec;
   while( ! ec )
   {
      bytes_read = asio::read( mSocket, asio::buffer( v_buf ), ec );
      v_full_msg.insert( v_full_msg.end(),
         v_buf.begin(), v_buf.begin() + bytes_read );
   }

   bool status_ok = false;
   const std::string s_ans = app_protocol.DecodeMsg(
      v_full_msg, status_ok );

   int ret_code = 0;
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

   return ret_code;
}
