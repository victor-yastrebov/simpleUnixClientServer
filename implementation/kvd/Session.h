#include<memory>
#include<iostream>

#include"DataBase.h"
#include"AppProtocol.h"
#include"asio.hpp"

using asio::local::stream_protocol;

class Session
  : public std::enable_shared_from_this<Session>
{
public:
  Session( asio::io_service& io_service, std::shared_ptr<DataBase> &p_db ) :
    mSocket( io_service ),
    pDataBase( p_db ),
    sessionIsOverEvent( nullptr ),
    stopServerEvent( nullptr )
  {
     std::cout << "session CTOR" << std::endl;
  }

  ~Session()
  {
      try
      {
         std::cout << "session DTOR" << std::endl;
         if( sessionIsOverEvent != nullptr ) sessionIsOverEvent(); 
      }
      catch( std::exception &e )
      {
         std::cout << "session DTOR exception caught: " << e.what() << std::endl;
      }
      catch( ... )
      {
         std::cout << "session DTOR unknown exception caught" << std::endl;
      }
  }

  stream_protocol::socket& getSocket()
  {
    return mSocket;
  }

  void Start()
  {
     mSocket.async_read_some(
        asio::buffer( mData ),
        std::bind( &Session::HandleRead,
           shared_from_this(),
           std::placeholders::_1,
           std::placeholders::_2
        )
     );
  }

  void HandleRead( const asio::error_code& error, size_t bytes_transferred )
  {
    if( !error )
    {
      bool status_ok = false;
      const std::string s_query = appProtocol.decodeMsg(
         std::vector<BYTE>( mData.begin(), mData.begin() + bytes_transferred ), status_ok );

      if( ! status_ok )
      {
         std::cout << "Received msg is not full" << std::endl;
         return;
      }

      std::cout << "Recv query: " << s_query << std::endl;
      // Command for stopping server
      if( s_query == "exit")
      {
         if( stopServerEvent != nullptr ) stopServerEvent();
         return;
      }

      const std::string ret = pDataBase->ProcessQuery( s_query ); 

      std::vector<BYTE> v_enc_data =
         appProtocol.encodeMsg( ret );

      for (std::size_t i = 0; i < v_enc_data.size(); ++i)
      {
         mData[i] = v_enc_data[i];
      }

      // std::cout << "Bytes to send: " << ret.size() << std::endl;
      asio::async_write(
          mSocket,
          asio::buffer( mData, v_enc_data.size() ),
          std::bind(
             &Session::HandleWrite,
             shared_from_this(),
             std::placeholders::_1
          )
      );
    }
    else
    {
       std::cout << "handle_read error: " << error << std::endl;
    }
  }

  void HandleWrite( const asio::error_code& error )
  {
    if( error )
    {
       std::cout << "handle write ec: " << error << std::endl;
    }
  }

   std::function<void()>   sessionIsOverEvent;
   std::function<void()>   stopServerEvent;

private:
     stream_protocol::socket   mSocket;
   std::shared_ptr<DataBase>   pDataBase;
      std::array<char, 1024>   mData;
                 AppProtocol   appProtocol;
}; 
