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

#include"DataBase.h"
#include"Session.h"
#include"asio.hpp"

class UDSServer
{
public:
          UDSServer( asio::io_service& io_service, const std::string& s_sock_fname );
         ~UDSServer() = default;
          UDSServer( const UDSServer& ) = delete;
          UDSServer& operator=( const UDSServer& ) = delete;
   void   HandleAccept( std::shared_ptr<Session> p_new_session, const asio::error_code& error );
   void   OnSessionIsOver();

private:
                        void   StartToListenForNewSession();

            asio::io_service&  asioService;
   stream_protocol::acceptor   asioAcceptor;
   std::shared_ptr<DataBase>   pDataBase;
                const size_t   nMaxOnlineUsers;
    std::atomic<std::size_t>   numOnlineUsers;
};
