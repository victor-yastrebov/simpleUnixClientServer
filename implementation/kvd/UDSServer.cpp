/**
 *
 * @author <va.yastrebov>
 * @description
 * <pre>
 *    Class is responsible for processing incoming connections from clients
  * </pre>
 *
 * @class UDSServer
 */

#include<sys/un.h>
#include<syslog.h>
#include<netinet/in.h>
#include<signal.h>
#include<unistd.h>
#include<fcntl.h>

#include<algorithm>
#include<set>
#include<iostream>
#include<sstream>

#include"UDS.h"
#include"UDSServer.h"
#include"QueryResult.h"
#include"Client.h"
#include"AppProtocol.h"

/**
 * CTOR
 */
UDSServer::UDSServer() :
   nMaxClients( 4096 ),   // max number of online clients
   listenerSocket( -1 )
{

}

/**
 * DTOR
 */
UDSServer::~UDSServer()
{

}

/**
 * Set up server socket for listening incoming connections
 */
int UDSServer::prepareListenerSocket()
{
   listenerSocket = socket( PF_UNIX, SOCK_STREAM, 0 );
   if( listenerSocket < 0 )
   {
      sysLogger.LogToSyslog( "Create listener failed" );
      return 0;
   }

   // set up socket for non blocking mode
   fcntl( listenerSocket, F_SETFL, O_NONBLOCK );

   struct sockaddr_un addr = { 0 };
   // memset( &addr, 0, sizeof( addr ) );
   addr.sun_family = AF_UNIX;
   strcpy( addr.sun_path, UDS::sServerSockFile.c_str() );
   unlink( UDS::sServerSockFile.c_str() );

   if( bind( listenerSocket,
             reinterpret_cast<struct sockaddr *>( &addr ),
             sizeof( addr )
           ) < 0 )
   {
      sysLogger.LogToSyslog( "Bind listener socket failed" );
      return 0;
   }

   const int max_waiting_conn = 2;
   listen( listenerSocket, max_waiting_conn );

   return 0;
}

/**
 * Process connected sockets queries
 */
int UDSServer::processConnectedSockets( const std::string& s_path_to_db )
{
   bool b_should_terminate = false;
   char buf[1024];
   int bytes_read;

   AppProtocol app_protocol;
   DataBase db( s_path_to_db );

   for( std::set<int>::iterator it = connectedSockets.begin();
        it != connectedSockets.end(); it++ )
   {
      if( FD_ISSET( *it, &readSet ) )
      {
         // Поступили данные от клиента, читаем их
         bytes_read = recv( *it, buf, 1024, 0 );

         if( bytes_read <= 0 )
         {
            sysLogger.LogToSyslog( "Connection is terminated" );
            // Соединение разорвано, удаляем сокет из множества
            close( *it );
            connectedSockets.erase( *it );
            continue;
         }

         sysLogger.LogToSyslog( "Recv num bytes: ", bytes_read);

         // В С++17 можно было бы использовать std::optional
         // для возвращаемого значения, не вводя переменную status_ok
         bool status_ok = false;
         const std::string s_query = app_protocol.decodeMsg(
            std::vector<BYTE>( buf, buf + bytes_read ), status_ok );

         if( ! status_ok )
         {
            sysLogger.LogToSyslog( "Received msg is not full" );
            continue;
         }

         std::stringstream ss;
         ss << "Receive query: " << s_query << std::endl;
         sysLogger.LogToSyslog( ss.str().c_str() );

         QueryResult qr = db.ExecuteQuery( s_query );
         sysLogger.LogToSyslog( "Query result is: ", qr.sData );

         if( qr.sData.empty() )
         {
            qr.sData = "Empty response";
         }
         std::vector<BYTE> v_result =
            app_protocol.encodeMsg( qr.sData );

         sysLogger.LogToSyslog( "Data to send: ", qr.sData );

         send( *it, v_result.data(), v_result.size(), 0 );

         if( s_query == ".exit" )
         {
            b_should_terminate = true;
            break;
         }
      }
   } // end of for (it = client.begin() ...)

   return b_should_terminate;
}

/**
 * Accept new connection if needed
 */
void UDSServer::processNewConnection()
{
   if( FD_ISSET( listenerSocket, &readSet ) )
   {
      const int sock = accept( listenerSocket, NULL, NULL );
      if( sock < 0 )
      {
         sysLogger.LogToSyslog( "Accept failed" );
         return;
      }

      // set up socket for non-blocking mode
      fcntl( sock, F_SETFL, O_NONBLOCK );
      connectedSockets.insert( sock );
   }
}

/**
 * Switch sockets to non-blocking mode
 */
void UDSServer::setNonBlockingMode()
{
   FD_ZERO( &readSet );
   FD_SET( listenerSocket, &readSet );

   for( std::set<int>::iterator it = connectedSockets.begin();
        it != connectedSockets.end(); it++ )
   {
      FD_SET( *it, &readSet );
   }
}

/**
 * Wait till one of the sockets have some action
 */
int UDSServer::waitForEvents()
{
   // Set timeout
   timeval timeout;
   timeout.tv_sec = 15;
   timeout.tv_usec = 0;

   // Wait for events in one of the sockets
   const int mx =std::max( listenerSocket,
      *std::max_element( connectedSockets.begin(), connectedSockets.end() ) );

   // n must greater by 1 of the max element in the set
   if( select( mx + 1, &readSet, NULL, NULL, &timeout ) <= 0 )
   {
      sysLogger.LogToSyslog( "select timeout. No active sockets found" );
      return -1;
   }

   return 0;
}

/**
 * Close connected sockets
 */
void UDSServer::closeSockets()
{
   for( std::set<int>::iterator it = connectedSockets.begin();
        it != connectedSockets.end(); it++ )
   {
      close( *it );
   }
}

/**
 * Start server base logic
 */
int UDSServer::processSockets( const std::string &s_path_to_db )
{
   // std::set<int> clients;
   // here the incoming data is stored
   // std::unorderd_map<int, Client> clients_map;
   connectedSockets.clear();

   while( 1 )
   {
      setNonBlockingMode();

      if( waitForEvents() < 0)
      {
         continue;
      }

      processNewConnection();
      const int b_should_terminate = processConnectedSockets( s_path_to_db );

      if( b_should_terminate )
      {
         closeSockets();
         break;
      }

     // run over std::um if some of the objects are in state 1 -> than do send()

   } // end of while( 1 )

   return 1;
}

/**
 * Listen and process incoming connections
 */
int UDSServer::startProcessing( const std::string &s_path_to_db )
{
   sysLogger.LogToSyslog( "KVD service started" );
   if( prepareListenerSocket() < 0 )
   {
      sysLogger.LogToSyslog( "Failed to open server listening socket" );
      return 1;
   }

   return processSockets( s_path_to_db );
}