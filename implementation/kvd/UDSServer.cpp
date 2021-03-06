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
UDSServer::UDSServer( const std::string& s_sock_fname, std::shared_ptr<SysLogger> &p_logger ) :
   pLogger( p_logger ),
   asioAcceptor( asioService, local_str_proto::endpoint( s_sock_fname ) ),
   nMaxOnlineUsers( 4096 ),
   numOnlineUsers( 0 ),
   curSessionId( 0 ),
   pDataBase( std::make_shared<DataBase>( p_logger ) ),
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

   // terminate all sessions
   {
      std::scoped_lock lock( mutSessions );

      // stop all active connections
      for( auto &p : umSessions )
      {
         std::shared_ptr<Session> p_sp = p.second.lock();
         if( p_sp )
         {
            p_sp.reset();
         }
      }
   }
}

/**
 * Run server for processing client queries
 */
void UDSServer::Run()
{
   while( false == bServerIsStopped )
   {
      // we have high load server, so there is a little chance that
      // server will run out of work. So, however we check it
      asioService.run();
      std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
   }
}

/**
 * Create and intialize new session for communication with
 * client socket
 */
void UDSServer::StartToListenForNewSession()
{
   pCurSession = std::make_shared<Session>(
      asioService, pDataBase, pLogger, curSessionId );

   ++curSessionId;

   SubscribeToEvents( pCurSession );

   asioAcceptor.async_accept(
      pCurSession->GetSocket(),
      std::bind(
         &UDSServer::HandleAccept, this, std::placeholders::_1
      )
   );
}

/**
 * Subscribe to events generated by session
 */
void UDSServer::SubscribeToEvents( std::shared_ptr<Session> &s )
{
   s->sessionIsOverEvent = std::bind(
      &UDSServer::OnSessionIsOver, this,
      std::placeholders::_1
   );

   s->stopServerEvent =
      std::bind( &UDSServer::OnStopServer, this );
}

/**
 * Accept new connection handler
 */
void UDSServer::HandleAccept( const asio::error_code& error )
{
   if( error )
   {
      pLogger->Log( "HandleAccept() error occured: ", error );
   }
   else
   {
      numOnlineUsers.fetch_add( 1 );
      {
         std::scoped_lock lock( mutSessions );
         umSessions.insert( std::make_pair( pCurSession->GetId(), pCurSession ) );
      }
      pCurSession->Start();
   }


   if( numOnlineUsers.load() < nMaxOnlineUsers )
   {
      StartToListenForNewSession();
   }
   else
   {
      pCurSession.reset();
   }
}

/**
 * Process session is over event
 */
void UDSServer::OnSessionIsOver( const size_t n_sess_id )
{
   numOnlineUsers.fetch_sub( 1 );

   {
      // this session is not longer valid
      std::scoped_lock lock( mutSessions );
      umSessions.erase( n_sess_id );
   }

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
   pLogger->Log( "Receive stop server command. Stopping server..." );
   bServerIsStopped.store( true );
   asioService.stop();
}
