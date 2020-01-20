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
UDSServer::UDSServer(  const std::string& s_sock_fname ) :
   asioAcceptor( asioService, local_str_proto::endpoint( s_sock_fname ) ),
   nMaxOnlineUsers( 4096 ),
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

   // terminate all sessions
   {
      std::scoped_lock lock( mutSessions );
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
   }

   std::cout << "UDSServer DTOR end" << std::endl;
}

void UDSServer::Run()
{
   while( false == bServerIsStopped )
   {
      std::cout << "Is stopped: " << asioService.stopped() << std::endl;
      // asioService.reset();

      // std::cout << "use_count()" << pCurSession.use_count() << std::endl;

      asioService.run();
      std::this_thread::sleep_for( std::chrono::seconds( 3 ) );
   }
}

/**
 * Create and intialize new session for communication with
 * client socket
 */
void UDSServer::StartToListenForNewSession()
{
   pCurSession = std::make_shared<Session>(
      asioService, pDataBase, curSessionId );
   // std::cout << "StartToListenForNewSession() start: use_count() - " << pCurSession.use_count() << std::endl;
    
   ++curSessionId;

   SubscribeToEvents( pCurSession );

   asioAcceptor.async_accept(
      pCurSession->GetSocket(),
      std::bind(
         &UDSServer::HandleAccept, this, std::placeholders::_1
      )
   );
   // std::cout << "StartToListenForNewSession() after async_accept: use_count() - " << pCurSession.use_count() << std::endl;
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
   // std::cout << "HandleAccept() start: use_count() - " << pCurSession.use_count() << std::endl;
   // std::cout << "HandleAccept() fired" << std::endl; 
   if( error )
   {
      std::cout << "HandleAccept() error occured: " 
         << error << std::endl;
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


   std::cout << "numOnlineUsers: " << numOnlineUsers.load() << std::endl;
   if( numOnlineUsers.load() < nMaxOnlineUsers )
   {
      // std::cout << "Start new session" << std::endl;
      StartToListenForNewSession();
   }
   else
   {
      pCurSession.reset();
   }

   // std::cout << "HandleAccept() end: use_count() - " << pCurSession.use_count() << std::endl;
}

/**
 * Process session is over event
 */
void UDSServer::OnSessionIsOver( const size_t n_sess_id )
{
   std::cout << "OnSessionIsOver() start for: " << n_sess_id << std::endl;

   numOnlineUsers.fetch_sub( 1 );
   std::cout << "Num online users: " <<
      numOnlineUsers.load() << std::endl;

   {
      // this session is not longer valid
      std::scoped_lock lock( mutSessions );
      umSessions.erase( n_sess_id );
   }

   if( ! bServerIsStopped.load() &&
       numOnlineUsers.load() + 1 == nMaxOnlineUsers )
   {
      // if( asioService.stopped() )
      // {
      //    std::cout << "Is stopped" << std::endl;
      //    asioService.reset();
      // }
      StartToListenForNewSession();
   }
   std::cout << "OnSessionIsOver() end for " << n_sess_id << std::endl;
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
