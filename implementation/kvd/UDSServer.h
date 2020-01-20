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

#include<string>
#include<string_view>
#include<unordered_map>

#include"DataBase.h"
#include"Session.h"
#include"asio.hpp"

class UDSServer
{
public:
          UDSServer( const std::string& s_sock_fname );
         ~UDSServer();
          UDSServer( const UDSServer& ) = delete;
          UDSServer& operator=( const UDSServer& ) = delete;
   void   Run();

private:
                        void   HandleAccept( const asio::error_code& error );
                        void   StartToListenForNewSession();
                        void   SubscribeToEvents( std::shared_ptr<Session> &s );
                        void   OnSessionIsOver( const size_t n_sess_id );
                        void   OnStopServer();

            asio::io_service   asioService;
   stream_protocol::acceptor   asioAcceptor;
   std::shared_ptr<DataBase>   pDataBase;
                const size_t   nMaxOnlineUsers;
    std::atomic<std::size_t>   numOnlineUsers;

                      size_t   curSessionId;
   std::unordered_map<size_t, std::weak_ptr<Session>>   umSessions;
    std::shared_ptr<Session>   pCurSession;
           std::atomic<bool>   bServerIsStopped;
};
