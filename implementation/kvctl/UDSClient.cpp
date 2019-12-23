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

#include"UDS.h"
#include"UDSClient.h"

UDSClient::UDSClient() :
   socketId( -1 )
{

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
   strcpy( addr.sun_path, CLIENT_SOCK_FILE );
   unlink( CLIENT_SOCK_FILE );

   if( bind( socketId, ( struct sockaddr * )&addr, sizeof( addr ) ) < 0 )
   {
      std::cout << "Error: failed to bind socket" << std::endl;
      return 0;
   }

   memset( &addr, 0, sizeof( addr ) );
   addr.sun_family = AF_UNIX;
   strcpy( addr.sun_path, SERVER_SOCK_FILE );
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
   const size_t buf_size = 8192;
   char buf[buf_size];

   std::cout << s_query << std::endl;

   if( send( socketId, s_query.c_str(), s_query.size(), 0 ) < 0 )
   {
      std::cout << "Error: failed to send msg due to network error" << std::endl;
      return 1;
   }

   int len;
   if( ( len = recv( socketId, buf, buf_size, 0 ) ) < 0 )
   {
      std::cout << "Error: failed to recv msg due to network error" << std::endl;
      return 1;
   }
   std::string s_ans( buf, len );
   std::cout << s_ans << std::endl;

   if( socketId > 0 ) close( socketId );

   return 0;
}