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

/**
 * Listen and process incoming connections
 */
int UDSClient::Connect()
{
   // TODO: move some logic here later
   return 1;
}

/**
 * Start query session to DB
 */
int UDSClient::StartSession() const
{
   int sock = socket( PF_UNIX, SOCK_STREAM, 0 );
   if( sock < 0 )
   {
       std::cout << "Error: failed to create socket" << std::endl;
       return 1;
   }

   struct sockaddr_un addr;
   memset( &addr, 0, sizeof( addr ) );
   addr.sun_family = AF_UNIX;
   strcpy( addr.sun_path, CLIENT_SOCK_FILE );
   unlink( CLIENT_SOCK_FILE );

   if( bind( sock, ( struct sockaddr * )&addr, sizeof( addr ) ) < 0 )
   {
      std::cout << "Error: failed to bind socket" << std::endl;
      return 1;
   }

   memset( &addr, 0, sizeof( addr ) );
   addr.sun_family = AF_UNIX;
   strcpy( addr.sun_path, SERVER_SOCK_FILE );
   if( connect( sock, ( struct sockaddr * )&addr, sizeof( addr ) ) == -1 )
   {
      std::cout << "Error: failed to connect to remote socket" << std::endl;
      return 1;
   }

   const size_t buf_size = 8192;
   char buf[buf_size];
   while( true )
   {
      std::cout << "$ ";
      std::string s_cmd;
      std::cin >> s_cmd;

      if( send( sock, s_cmd.c_str(), s_cmd.size(), 0 ) < 0 )
      {
         std::cout << "Error: failed to send msg due to network error" << std::endl;
         break;
      }

      int len;
      if( ( len = recv( sock, buf, buf_size, 0 ) ) < 0 )
      {
         std::cout << "Error: failed to recv msg due to network error" << std::endl;
      }
      std::string s_ans( buf, len );
      std::cout << s_ans << std::endl;
   }

   if( sock > 0 ) close( sock );

}