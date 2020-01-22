/**
 *
 * @author <va.yastrebov>
 * @description
 * <pre>
 *    Class is responsible for quering data from remote DB
  * </pre>
 *
 * @class UDSClient
 */

#include<string>

#include"asio.hpp"

using local_str_proto = asio::local::stream_protocol;

class UDSClient
{
public:
         UDSClient( const std::string &s_sock_file );
        ~UDSClient();
         UDSClient( const UDSClient& ) = delete;
         UDSClient& operator=( const UDSClient& ) = delete;
   int   Connect( const std::string &s_server_sock_file );
   int   ProcSession( const std::string &s_query );

private:
         const std::string   sSockFile;
          asio::io_service   ioService;
   local_str_proto::socket   mSocket;
};
