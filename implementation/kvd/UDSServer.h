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

// using session_ptr = std::shared_ptr<session>;

class UDSServer
{
public:
   UDSServer(asio::io_service& io_service, const std::string& file) :
      io_service_(io_service),
      acceptor_(io_service, stream_protocol::endpoint(file))
   {
      pDataBase = std::make_shared<DataBase>();

      std::shared_ptr<session> p_new_session( new session(io_service_, pDataBase) );
      acceptor_.async_accept(
         p_new_session->socket(),
         std::bind(
            &UDSServer::handle_accept, this,
            p_new_session, std::placeholders::_1
         )
      );
   }

  ~UDSServer() = default;
  UDSServer( const UDSServer& ) = delete;
  UDSServer& operator=( const UDSServer& ) = delete;

  void handle_accept(std::shared_ptr<session> p_new_session, const asio::error_code& error)
  {
    if (!error)
    {
      p_new_session->start();

      p_new_session.reset(new session( io_service_, pDataBase ));
      acceptor_.async_accept(
         p_new_session->socket(),
         std::bind(
            &UDSServer::handle_accept, this,
            p_new_session, std::placeholders::_1 
         )
      );

    }
  }

private:
   asio::io_service& io_service_;
   stream_protocol::acceptor acceptor_;
   std::shared_ptr<DataBase> pDataBase;
};
