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
	nMaxClients( 4096 )   // max number of online clients
{

}

/**
 * DTOR
 */
UDSServer::~UDSServer()
{

}

/**
 * Listen and process incoming connections
 */
int UDSServer::StartProcessing()
{
   sysLogger.LogToSyslog( "KVD service started" );

   int listener;
   struct sockaddr_un addr;
   char buf[1024];
   int bytes_read;

   listener = socket(PF_UNIX, SOCK_STREAM, 0);
   if( listener < 0 )
   {
      sysLogger.LogToSyslog( "Create listener failed" );
      return 1;
   }

   // set up socket for non blocking mode
   fcntl( listener, F_SETFL, O_NONBLOCK );

   memset( &addr, 0, sizeof( addr ) );
   addr.sun_family = AF_UNIX;
   strcpy( addr.sun_path, SERVER_SOCK_FILE );
   unlink( SERVER_SOCK_FILE );
   if( bind( listener, ( struct sockaddr *)&addr, sizeof( addr ) ) < 0 )
   {
      sysLogger.LogToSyslog( "Bind listener socket failed" );
      return 2;
   }

   const int max_waiting_conn = 2;
   listen( listener, max_waiting_conn );

   std::set<int> clients;
   // here the incoming data is stored 
   // std::unorderd_map<int, Client> clients_map;
   clients.clear();

   bool b_should_terminate = false;

   DataBase db( "/tmp/kvd_db.txt" );

   AppProtocol app_protocol;
   std::vector<BYTE> v_query_result( 32768 );

/*
   bool a = true;
   while(a){
      int aa = 0;
   }
*/
   while( 1 )
   {
      // Заполняем множество сокетов из которых нам требуется читать данные
      fd_set readset;
      FD_ZERO( &readset );
      FD_SET( listener, &readset );   // добавляет дескриптор listener в множество readset

      for( std::set<int>::iterator it = clients.begin(); it != clients.end(); it++ )
      {
         FD_SET(*it, &readset);
      }

      // Задаём таймаут
      timeval timeout;
      timeout.tv_sec = 15;
      timeout.tv_usec = 0;

      // Ждём события в одном из сокетов
      int mx = std::max( listener, *std::max_element( clients.begin(), clients.end() ) );

      // n (первый парметр) должен быть на единицу больше самого большого номера описателей из всех наборов.
      if( select( mx+1, &readset, NULL, NULL, &timeout ) <= 0 )
      {
         sysLogger.LogToSyslog( "select timeout. No active sockets found" );
         continue;
      }

      // Определяем тип события и выполняем соответствующие действия
      if( FD_ISSET( listener, &readset ) )   // проверяет, содержится ли дескриптор fd в множестве set
      {
         // Поступил новый запрос на соединение, используем accept
         int sock = accept( listener, NULL, NULL );
         if( sock < 0 )
         {
            sysLogger.LogToSyslog( "Accept failed" );
            return 3;
         }

         // set up socket for non-blocking mode
         fcntl( sock, F_SETFL, O_NONBLOCK );

         clients.insert( sock );
     }

     for( std::set<int>::iterator it = clients.begin(); it != clients.end(); it++ )
     {
         if( FD_ISSET( *it, &readset ) )
         {
            // Поступили данные от клиента, читаем их
            bytes_read = recv( *it, buf, 1024, 0 );

            if( bytes_read <= 0 )
            {
               sysLogger.LogToSyslog( "Connection is terminated" );
               // Соединение разорвано, удаляем сокет из множества
               close( *it );
               clients.erase( *it );
               continue;
            }

            sysLogger.LogToSyslog( "Recv num bytes: ", bytes_read);

            /*bool a = true;
            while(a){
               int aa = 0;
            }*/

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

            // Отправляем данные обратно клиенту
            // sysLogger.LogToSyslog( "Sending back: " << s_ans << std::endl;

            if( qr.sData.empty() )
            {
               qr.sData = "Empty response";
            }
            std::vector<BYTE> v_result =
               app_protocol.encodeMsg( qr.sData );

            send( *it, v_result.data(), v_result.size(), 0 );

            if( s_query == ".exit" )
            {
               b_should_terminate = true;
               break;
            }
         }
      } // end of for (it = client.begin() ...)

     if( b_should_terminate )
     {
        for( std::set<int>::iterator it = clients.begin(); it != clients.end(); it++ )
        {
            close( *it );
        }
        break;
     }

     // run over std::um if some of the objects are in state 1 -> than do send()

   } // end of while( 1 )

   sysLogger.LogToSyslog( "LOOP is DONE" );


   return 1;
}

/**
 * Process incoming DB query
 */
// C++17 std::stringstream
std::string UDSServer::ProcessQuery( const std::string &s_query ) const
{

   if(s_query == "LIST") return "bar baz max tRex";
   if(s_query == "PUT") return "PUT query OK";
   if(s_query == "GET") return "GET query OK";
   if(s_query == "ERASE") return "ERASE query OK";
   if(s_query == "STOP") return "STOP";

   return "UNKNOWN CMD";
}
