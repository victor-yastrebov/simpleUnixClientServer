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

// client-based echo server with client
// https://github.com/jasonish/libevent-examples/blob/master/buffered-echo-server/libevent_echosrv_buffered.c

#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include <arpa/inet.h>
#include <sys/un.h>
#include <unistd.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include<iostream>
#include<vector>
#include<functional>

#include "AppProtocol.h"
#include "UDS.h"

class UDSServerLibEvent
{
public:
          UDSServerLibEvent();
         ~UDSServerLibEvent();
   void   Run();

private:
                       event_base   *pBase;
                    evconnlistener  *pListener;
                      struct event  *pTimer;
     inline static evutil_socket_t   mClientSocket;
         inline static bufferevent  *pClientBev;

   static void   HandleAcceptError( struct evconnlistener *listener, void *ctx );
   static void   HandleAcceptSuccess( struct evconnlistener *listener, evutil_socket_t fd, sockaddr *address, int socklen, void *ctx );
   static void   HandleReadyToWriteData( struct bufferevent *bev, void *ctx );
   static void   HandleRecvData(struct bufferevent *bev, void *ctx);
   static void   HandleEvent(struct bufferevent *bev, short events, void *ctx);
   static void   SignalEvent( evutil_socket_t, short, void * );
   std::string   GetAnswer( const std::string &s_query );

          inline static bool   bDataIsWritten = false;
};