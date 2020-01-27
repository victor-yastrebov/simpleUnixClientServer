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
#include<mutex>

#include"DataBase.h"
#include"Session.h"
#include"asio.hpp"
#include"SysLogger.h"

using um_wptr_sess = std::unordered_map<size_t, std::weak_ptr<Session>>;

class UDSServer
{
public:
          UDSServer( const std::string& s_sock_fname, std::shared_ptr<SysLogger> &p_logger );
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

   std::shared_ptr<SysLogger>   pLogger;
             asio::io_service   asioService;
    local_str_proto::acceptor   asioAcceptor;
    std::shared_ptr<DataBase>   pDataBase;
                 const size_t   nMaxOnlineUsers;
     std::atomic<std::size_t>   numOnlineUsers;
                       size_t   curSessionId;
                   std::mutex   mutSessions;
                 um_wptr_sess   umSessions;
     std::shared_ptr<Session>   pCurSession;
            std::atomic<bool>   bServerIsStopped;
};
