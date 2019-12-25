/**
 *
 * @author <va.yastrebov>
 * @description
 * <pre>
 *    Class is responsible for quering data from DB
  * </pre>
 *
 * @class UDSClient
 */

#include<sys/socket.h>
#include<sys/un.h>

#include<unistd.h>
#include<iostream>
#include<iomanip>
#include<sstream>

#include"UDS.h"
#include"UDSClient.h"
#include"AppProtocol.h"

/**
 * CTOR
 */
UDSClient::UDSClient() :
   socketId( -1 )
{

}

/**
 * DTOR
 */
UDSClient::~UDSClient()
{
   if( socketId > 0 )
   {
      close( socketId );
   }
}

/**
 * Listen and process incoming connections
 */
int UDSClient::Connect()
{
   socketId = socket( PF_UNIX, SOCK_STREAM, 0 );
   if( socketId < 0 )
   {
       std::cout << "Error: failed to create socket" << std::endl;
       return 0;
   }

   struct sockaddr_un addr;
   memset( &addr, 0, sizeof( addr ) );
   addr.sun_family = AF_UNIX;
   strcpy( addr.sun_path, UDS::sClientSockFile.c_str() );
   unlink( UDS::sClientSockFile.c_str() );

   if( bind( socketId, ( struct sockaddr * )&addr, sizeof( addr ) ) < 0 )
   {
      std::cout << "Error: failed to bind socket" << std::endl;
      return 0;
   }

   memset( &addr, 0, sizeof( addr ) );
   addr.sun_family = AF_UNIX;
   strcpy( addr.sun_path, UDS::sServerSockFile.c_str() );
   if( connect( socketId, ( struct sockaddr * )&addr, sizeof( addr ) ) == -1 )
   {
      std::cout << "Error: failed to connect to remote socket" << std::endl;
      return 0;
   }

   return 1;
}

/**
 * Start query session to DB
 */
int UDSClient::Query( const std::string &s_query ) const
{
   AppProtocol app_protocol;

   const size_t buf_size = 8192;
   char buf[buf_size];

   std::vector<BYTE> v_query = app_protocol.encodeMsg( s_query );

   if( send( socketId, v_query.data(), v_query.size(), 0 ) < 0 )
   {
      std::cout << "Error: failed to send msg due to network error" << std::endl;
      return 1;
   }

   int bytes_read;
   if( ( bytes_read = recv( socketId, buf, buf_size, 0 ) ) < 0 )
   {
      std::cout << "Error: failed to recv msg due to network error" << std::endl;
      return 1;
   }

   bool status_ok = false;
   const std::string s_answer = app_protocol.decodeMsg(
      std::vector<BYTE>( buf, buf + bytes_read ), status_ok );

   std::stringstream ss;
   ss << "msg is " << std::hex;
   for ( int i=0; i<bytes_read; ++i )
   {
      ss << "0x" << std::uppercase << std::hex << std::setfill( '0' ) << std::setw( 2 )
               << static_cast<int>( buf[i] ) << " ";
   }
   ss << std::dec;

   // std::cout << ss.str() << std::endl;

   if( ! status_ok )
   {
      std::cout << "Received msg is not full" << std::endl;
   }
   else
   {
      if( s_answer != "Empty response")
      {
         std::cout << s_answer;
      }
   }

   return 0;
}