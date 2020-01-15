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


UDSServer::UDSServer(asio::io_service& io_service, const std::string& file) :
   io_service_(io_service),
   acceptor_(io_service, stream_protocol::endpoint(file))
{
   pDataBase = std::make_shared<DataBase>();

   std::shared_ptr<session> p_new_session( new session(io_service_, pDataBase) );
   acceptor_.async_accept(
      p_new_session->socket(),
      std::bind(
         &UDSServer::HandleAccept, this,
         p_new_session, std::placeholders::_1
      )
   );
}

void UDSServer::HandleAccept(std::shared_ptr<session> p_new_session, const asio::error_code& error)
{
   if (!error)
   {
      p_new_session->start();

      p_new_session.reset(new session( io_service_, pDataBase ));
      acceptor_.async_accept(
         p_new_session->socket(),
         std::bind(
            &UDSServer::HandleAccept, this,
            p_new_session, std::placeholders::_1 
         )
      );
   }
}
