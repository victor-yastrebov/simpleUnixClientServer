/**
 *
 * @author <va.yastrebov>
 * @description
 * <pre>
 *    Class is responsible for processing incoming connections from clients
 * using LibEvent library
  * </pre>
 *
 * @class UDSServerLibEvent
 */

#include <signal.h>
#include<thread>

#include"UDSServerLibEvent.h"

/**
 * CTOR
 */
UDSServerLibEvent::UDSServerLibEvent() :
   pListener( nullptr )
{
   std::cout << "Server CTOR" << std::endl;
}

/**
 * DTOR
 */
UDSServerLibEvent::~UDSServerLibEvent()
{
   std::cout << "Server DTOR" << std::endl;
   if( pListener )
   {
      evconnlistener_free( pListener );
   }

   // event_free(signal_event);

   // optional
   libevent_global_shutdown();
}

/**
 * Main run function
 */
void UDSServerLibEvent::Run()
{
   pBase = event_base_new();
   if( NULL == pBase)
   {
      std::cout << "Error(): failed to create base" << std::endl;
      return;
   }

   // set up handler for signal processing (e.x. Ctrl+C)
   event *p_signal_event =
      evsignal_new( pBase, SIGINT, SignalEvent, (void *)pBase );
   if( ! p_signal_event || event_add( p_signal_event, NULL ) < 0 )
   {
      std::cout << "Could not create/add a signal event" << std::endl;
   }

   // remove file from HDD
   std::remove( UDS::sServerSockFile.c_str() );

   sockaddr_un sun;
   memset( &sun, 0, sizeof( sun ) );
   sun.sun_family = AF_UNIX;
   strcpy( sun.sun_path, UDS::sServerSockFile.c_str() );

   pListener = evconnlistener_new_bind(
      pBase,
      &UDSServerLibEvent::HandleAcceptSuccess, this,
      LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
      -1,
      ( struct sockaddr* )&sun, sizeof( sun )
   );

   if( ! pListener )
   {
      std::cout << "Couldn't create listener" << std::endl;
      return;
   }

   std::cout << "Create listener success" << std::endl;

   evconnlistener_set_error_cb( pListener, HandleAcceptError );

   // It is a blocking call which executes your defined callbacks inside
   // a loop in the thread that calls the function
   event_base_dispatch( pBase );
}

/**
 * Client connection accept error handler
 */
void UDSServerLibEvent::HandleAcceptError( struct evconnlistener *listener, void *ctx )
{
   event_base *p_base = evconnlistener_get_base( listener );
   const int err = EVUTIL_SOCKET_ERROR();

   std::cout << "HandleAcceptError(): get error " << err << " (" <<
      evutil_socket_error_to_string(err) << ")" << std::endl;

   event_base_loopexit( p_base, NULL );
}

/**
 * Client connection accept success handler
 */
void UDSServerLibEvent::HandleAcceptSuccess(
      struct evconnlistener *listener,
      evutil_socket_t fd,
      sockaddr *address,
      int socklen,
      void *ctx)
{
   // We got a new connection! Set up a bufferevent for it
   event_base *p_listener_base = evconnlistener_get_base( listener );
   bufferevent *p_client_socket_bev = bufferevent_socket_new( p_listener_base, fd, BEV_OPT_CLOSE_ON_FREE );

   UDSServerLibEvent::mClientSocket = fd;
   pClientBev = p_client_socket_bev;

   bufferevent_setcb( p_client_socket_bev, HandleRecvData, HandleReadyToWriteData, HandleEvent, ctx );
   bufferevent_enable( p_client_socket_bev, EV_READ | EV_WRITE );
}

/**
 * Handle write data
 *
 * This callback is invoked by libevent automatically when there is enough room
 * in the OUTPUT side of bufferevent for some new data to be written into
 */
void UDSServerLibEvent::HandleReadyToWriteData( struct bufferevent *bev, void *ctx )
{
   evbuffer *p_output_buf = bufferevent_get_output( bev );
   const int num_bytes_to_write = evbuffer_get_length( p_output_buf );

   if( bDataIsWritten )
   {
      std::cout << "close socket" << std::endl;
      bufferevent_free( bev );
      bDataIsWritten = false;
   }
}

/**
 * Handle recv data from client socket
 */
void UDSServerLibEvent::HandleRecvData(struct bufferevent *bev, void *ctx)
{
   std::cout << "Handle recv data" << std::endl;

   UDSServerLibEvent *p_this = static_cast<UDSServerLibEvent*>(ctx);

   // Returns the input buffer
   evbuffer * const p_input_buf = bufferevent_get_input( bev );

   const size_t num_bytes = evbuffer_get_length( p_input_buf );
   std::cout << "Recv: " << num_bytes << " bytes " << std::endl;

   std::vector<BYTE> v_buf( num_bytes );

   const ev_ssize_t num_bytes_copy =
      evbuffer_copyout( p_input_buf, v_buf.data(), num_bytes );
   evbuffer_drain( p_input_buf, num_bytes );

   AppProtocol ap;

   bool status_ok = false;
   std::string s_buf = ap.DecodeMsg( v_buf, status_ok );

   std::cout << "Recv query: " << s_buf << std::endl;

   if( s_buf.compare( "exit" ) == 0 )
   {
      event_base_loopbreak( p_this->pBase );
      return;
   }

   std::string s_ans = p_this->GetAnswer( s_buf );

   std::vector<BYTE> v_enc = ap.EncodeMsg( s_ans );


   /*
   struct bufferevent *buf_ev = bufferevent_new(
      client_fd, buffered_on_read, buffered_on_write, buffered_on_error, client);

   bufferevent_enable(buf_ev, EV_READ | EV_WRITE);
   */
/**/
   // Create output buffer
   // evbuffer_new()
   evbuffer *p_output_buf = bufferevent_get_output( bev );

   // std::vector<BYTE> v_enc = ap.EncodeMsg( s_ans );

   evbuffer_add( p_output_buf, v_enc.data(), v_enc.size() );

   evbuffer_add_buffer( bufferevent_get_output( bev ), p_output_buf );

   bDataIsWritten = true;

   // moves all data from p_input_buf to the end of p_output_buf
   // evbuffer_add_buffer( p_output_buf, p_input_buf);
/**/
}

void UDSServerLibEvent::HandleEvent(struct bufferevent *bev, short events, void *ctx)
{
   std::cout << "!!!! HandleEvent()" << std::endl;

   if (events & BEV_EVENT_ERROR)
   {
      std::cout << "Error from bufferevent code: " <<
         EVUTIL_SOCKET_ERROR() << std::endl;
   }

   if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR))
   {
      bufferevent_free(bev);
      std::cout << "After buffer free" << std::endl;
   }
}

std::string UDSServerLibEvent::GetAnswer( const std::string &s_query )
{
   std::string s_ans;
   if( s_query.compare( "list" ) == 0 )
   {
      s_ans = "foo bar baz";
      // s_ans = "LIST_RESULT";
   }
   else if( s_query.compare( "get" ) == 0 )
   {
      s_ans = "key value";
   }
   else
   {
      s_ans = "UNKNOWN CMD";
   }

   return s_ans;
}

/**
 * Signal to application processing
 */
void UDSServerLibEvent::SignalEvent( evutil_socket_t sig, short events, void *user_data )
{
   struct event_base *base = static_cast<event_base*>( user_data );
   struct timeval delay = { 2, 0 };

   std::cout << "Caught an interrupt signal; "
      "exiting cleanly in two seconds" << std::endl;

   event_base_loopexit( base, &delay );
}
