/**
 *
 * @author <va.yastrebov>
 * @description
 * <pre>
 *    Class is responsible for processing clients queries
  * </pre>
 *
 * @class Session
 */

#include<memory>
#include<iostream>

#include"DataBase.h"
#include"AppProtocol.h"
#include"asio.hpp"

using local_str_proto = asio::local::stream_protocol;

class Session
  : public std::enable_shared_from_this<Session>
{
public:
                              Session( asio::io_service& io_service,
                                       std::shared_ptr<DataBase> &p_db,
                                       const size_t id );
                             ~Session();
   local_str_proto::socket&  getSocket();
                      void   Start();
                    size_t   getId() const noexcept;

   std::function<void(int)>   sessionIsOverEvent;
      std::function<void()>   stopServerEvent;

private:
      void   SessionIsOverNotify();
      void   StopServerNotify();
      void   SendMsg( const std::string &s_msg );
      void   HandleRead( const asio::error_code& error, size_t bytes_transferred );
      void   HandleWrite( const asio::error_code& error, std::size_t bytes_transferred );

     local_str_proto::socket   mSocket;
   std::shared_ptr<DataBase>   pDataBase;
      std::array<char, 1024>   mData;
                 AppProtocol   appProtocol;
                      size_t   nId;
};