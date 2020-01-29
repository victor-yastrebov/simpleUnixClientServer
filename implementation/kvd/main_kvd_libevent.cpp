#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include <arpa/inet.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include<iostream>
#include<vector>
#include<functional>

class UDSServerLibEvent
{
public:
   UDSServerLibEvent()
   {
      std::cout << "Server CTOR" << std::endl;
   }

   ~UDSServerLibEvent()
   {
      std::cout << "Server DTOR" << std::endl;
   }

   void Run()
   {
      pBase = event_base_new();

      struct sockaddr_in sin;
      const int port = 9876;

      /* Clear the sockaddr before using it, in case there are extra
      * platform-specific fields that can mess us up. */
      memset(&sin, 0, sizeof(sin));
      /* This is an INET address */
      sin.sin_family = AF_INET;
      /* Listen on 0.0.0.0 */
      sin.sin_addr.s_addr = htonl(0);
      /* Listen on the given port. */
      sin.sin_port = htons(port);

      pListener = evconnlistener_new_bind(
         pBase,
         &UDSServerLibEvent::HandleAcceptSuccess, this,
         LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
         -1,
         (struct sockaddr*)&sin, sizeof( sin )
      );

      if( ! pListener )
      {
         std::cout << "Couldn't create listener" << std::endl;
         return;
      }

      evconnlistener_set_error_cb( pListener, HandleAcceptError );

      event_base_dispatch(pBase);
   }

private:
      event_base   *pBase;
   evconnlistener  *pListener;
     struct event  *pTimer;

   static void HandleAcceptError( struct evconnlistener *listener, void *ctx )
   {
      struct event_base *base = evconnlistener_get_base(listener);
      const int err = EVUTIL_SOCKET_ERROR();

      std::cout << "HandleAcceptError(): get error " << err << " (" <<
         evutil_socket_error_to_string(err) << ")" << std::endl;

      event_base_loopexit(base, NULL);
   }

   static void HandleAcceptSuccess(
      struct evconnlistener *listener,
      evutil_socket_t fd,
      struct sockaddr *address,
      int socklen,
      void *ctx
   )
   {
      /* We got a new connection! Set up a bufferevent for it. */
      struct event_base *base = evconnlistener_get_base(listener);
      struct bufferevent *bev = bufferevent_socket_new(
             base, fd, BEV_OPT_CLOSE_ON_FREE);

      bufferevent_setcb(bev, HandleRecvData, NULL, HandleEvent, ctx);

      bufferevent_enable(bev, EV_READ|EV_WRITE);
   }

   static void HandleRecvData(struct bufferevent *bev, void *ctx)
   {
      UDSServerLibEvent *p_this = (static_cast<UDSServerLibEvent*>(ctx));

      // Returns the input buffer
      evbuffer * const p_input_buf = bufferevent_get_input( bev );

      const size_t num_bytes = evbuffer_get_length( p_input_buf );
      std::cout << "Recv: " << num_bytes << " bytes " << std::endl;

      // use fill constructor
      std::string s_buf( num_bytes, ' ' );

      const ev_ssize_t num_bytes_copy =
         evbuffer_copyout( p_input_buf, s_buf.data(), num_bytes);
      evbuffer_drain( p_input_buf, num_bytes );

      s_buf.pop_back();
      s_buf.pop_back();

      std::cout << "Recv query: " << s_buf << std::endl;

      if( s_buf.compare( "exit" ) == 0 )
      {
         event_base_loopbreak( p_this->pBase );
         return;
      }

      std::string s_ans = p_this->GetAnswer( s_buf );
      s_ans += "\n";

      // Create output buffer
      evbuffer *p_output_buf = evbuffer_new(); //bufferevent_get_output( bev );

      evbuffer_add( p_output_buf, s_ans.c_str(), s_ans.size() );

      evbuffer_add_buffer( bufferevent_get_output( bev ), p_output_buf );

      // moves all data from p_input_buf to the end of p_output_buf
      // evbuffer_add_buffer( p_output_buf, p_input_buf);
   }

   static void HandleEvent(struct bufferevent *bev, short events, void *ctx)
   {
      if (events & BEV_EVENT_ERROR)
             perror("Error from bufferevent");
      if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
             bufferevent_free(bev);
      }
   }

   std::string GetAnswer( const std::string &s_query )
   {
      std::string s_ans;
      if( s_query.compare( "list" ) == 0 )
      {
         s_ans = "foo bar baz";
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
};

int main(int argc, char **argv)
{
   UDSServerLibEvent sle;
   sle.Run();
   std::cout << "OK" << std::endl;
   return 1;
}