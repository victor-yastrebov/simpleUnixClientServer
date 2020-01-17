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
   nMaxOnlineUsers( 2 ),
   numOnlineUsers( 0 ),
   curSessionId( 0 ),
   pDataBase( std::make_shared<DataBase>() ),
   pCurSession( nullptr ),
   bServerIsStopped( false )
{
   StartToListenForNewSession();
}

/**
 * DTOR
 */
UDSServer::~UDSServer()
{
   // reset session that is wating for the incoming connection
   pCurSession.reset();

   // stop all active connections
   for( auto &p : umSessions )
   {
      std::cout << "Inside cycle" << std::endl;
      std::shared_ptr<Session> p_sp = p.second.lock();
      if( p_sp )
      {
         std::cout << "Reset" << std::endl;
         p_sp.reset();
      }
   }

   std::cout << "UDSServer DTOR" << std::endl;
}

/**
 * Create and intialize new session for communication with
 * client socket
 */
void UDSServer::StartToListenForNewSession()
{
   std::cout << "StartToListenForNewSession()" << std::endl;
   pCurSession = std::make_shared<Session>(
      asioService, pDataBase, curSessionId );
    
   ++curSessionId;

   SubscribeToEvents( pCurSession );

   asioAcceptor.async_accept(
      pCurSession->getSocket(),
      std::bind(
         &UDSServer::HandleAccept, this,
         pCurSession, std::placeholders::_1
      )
   );
}

/**
 * Subscribe to events generated by session
 */
void UDSServer::SubscribeToEvents( std::shared_ptr<Session> &s )
{
   s->sessionIsOverEvent =
      std::bind( &UDSServer::OnSessionIsOver, this );

   s->stopServerEvent =
      std::bind( &UDSServer::OnStopServer, this );
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
      umSessions.insert( std::make_pair( p_new_session->getId(), p_new_session ) ); 
      p_new_session->Start();
   }

   std::cout << "numOnlineUsers: " << numOnlineUsers.load() << std::endl; 
   if( numOnlineUsers.load() < nMaxOnlineUsers )
   {
      std::cout << "Start new session" << std::endl; 
      StartToListenForNewSession();
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

   // TODO: remove from umSessions

   if( ! bServerIsStopped.load() &&
       numOnlineUsers.load() + 1 == nMaxOnlineUsers )
   {
      StartToListenForNewSession();
   }
}

/**
 * Process session is over event
 */
void UDSServer::OnStopServer()
{
   std::cout << "UDSServer(): receive stop server command" << std::endl;
   bServerIsStopped.store( true );
   asioService.stop();
}
