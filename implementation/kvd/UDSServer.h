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
          UDSServer(asio::io_service& io_service, const std::string& file);
         ~UDSServer() = default;
          UDSServer( const UDSServer& ) = delete;
          UDSServer& operator=( const UDSServer& ) = delete;
   void   HandleAccept( std::shared_ptr<session> p_new_session, const asio::error_code& error );

private:
           asio::io_service&   io_service_;
   stream_protocol::acceptor   acceptor_;
   std::shared_ptr<DataBase>   pDataBase;
};
