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

#include"UDSServer.h"

/**
 * CTOR
 */
UDSServer::UDSServer( asio::io_service& io_service, const std::string& s_sock_fname ) :
   asioService( io_service ),
   asioAcceptor( io_service, stream_protocol::endpoint( s_sock_fname ) ),
   nMaxOnlineUsers( 1 ),
   numOnlineUsers( 0 )
{
   pDataBase = std::make_shared<DataBase>();

   StartToListenForNewSession();
}

/**
 * Create and intialize new session for communication with
 * client socket
 */
void UDSServer::StartToListenForNewSession()
{
   std::shared_ptr<Session> p_new_session =
      std::make_shared<Session>( asioService, pDataBase );
   p_new_session->sessionIsOverEvent =
      std::bind( &UDSServer::OnSessionIsOver, this );

   asioAcceptor.async_accept(
      p_new_session->getSocket(),
      std::bind(
         &UDSServer::HandleAccept, this,
         p_new_session, std::placeholders::_1
      )
   );
}

/**
 * Accept new connection handler
 */
void UDSServer::HandleAccept( std::shared_ptr<Session> p_new_session, const asio::error_code& error )
{
   std::cout << "HandleAccept() fired" << std::endl; 
   if( error )
   {
      std::cout << "HandleAccept() error occured: " 
         << error << std::endl;
   }
   else
   {
      numOnlineUsers.fetch_add( 1 );
      p_new_session->Start();
   }

   std::cout << "numOnlineUsers: " << numOnlineUsers.load() << std::endl; 
   if( numOnlineUsers.load() < nMaxOnlineUsers )
   {
      std::cout << "Start new session" << std::endl; 

      p_new_session.reset( new Session( asioService, pDataBase ));
      p_new_session->sessionIsOverEvent =
         std::bind( &UDSServer::OnSessionIsOver, this );

      asioAcceptor.async_accept(
         p_new_session->getSocket(),
         std::bind(
            &UDSServer::HandleAccept, this,
            p_new_session, std::placeholders::_1 
         )
      );
   }

}

/**
 * Process session is over event
 */
void UDSServer::OnSessionIsOver()
{
   numOnlineUsers.fetch_sub( 1 );
   std::cout << "Num online users: " <<
      numOnlineUsers.load() << std::endl;

   if( numOnlineUsers.load() < nMaxOnlineUsers )
   {
      StartToListenForNewSession();
   }
}
