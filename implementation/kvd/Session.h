#include<memory>
#include<iostream>

#include"DataBase.h"
#include"AppProtocol.h"
#include"asio.hpp"

using asio::local::stream_protocol;

class session
  : public std::enable_shared_from_this<session>
{
public:
  session( asio::io_service& io_service, std::shared_ptr<DataBase> &p_db ) :
    socket_(io_service),
    pDataBase( p_db )
  {
     std::cout << "session CTOR" << std::endl;
  }

  ~session()
  {
     std::cout << "session DTOR" << std::endl;
  }

  stream_protocol::socket& socket()
  {
    return socket_;
  }

  void start()
  {
    socket_.async_read_some(asio::buffer(data_),
       std::bind(&session::handle_read,
          shared_from_this(),
          std::placeholders::_1,
          std::placeholders::_2));
  }

  void handle_read(const asio::error_code& error,
      size_t bytes_transferred)
  {
    if (!error)
    {
      bool status_ok = false;
      const std::string s_query = appProtocol.decodeMsg(
         std::vector<BYTE>( data_.begin(), data_.begin() + bytes_transferred ), status_ok );

      if( ! status_ok )
      {
         std::cout << "Received msg is not full" << std::endl;
         return;
      }
      std::cout << "Recv query: " << s_query << std::endl;

      const std::string ret = pDataBase->ProcessQuery( s_query ); 
      std::cout << "Recv query: " << s_query << std::endl;

      std::vector<BYTE> v_enc_data =
         appProtocol.encodeMsg( ret );

      for (std::size_t i = 0; i < v_enc_data.size(); ++i)
        data_[i] = v_enc_data[i];

      // std::cout << "Bytes to send: " << ret.size() << std::endl;
      asio::async_write(socket_,
          asio::buffer(data_, v_enc_data.size()),
          std::bind(&session::handle_write,
            shared_from_this(),
            std::placeholders::_1));
    }
    else
    {
       std::cout << "handle_read error: " << error << std::endl;
    }
  }

  void handle_write(const asio::error_code& error)
  {
    // std::cout << "handle write ec: " << error << std::endl;
    if (!error)
    {
/*
      socket_.async_read_some(asio::buffer(data_),
          std::bind(&session::handle_read,
            shared_from_this(),
            std::placeholders::_1,
            std::placeholders::_2));
*/
    }
  }

private:
  // The socket used to communicate with the client.
  stream_protocol::socket socket_;
  std::shared_ptr<DataBase> pDataBase;

  // Buffer used to store data received from the client.
  std::array<char, 1024> data_;
  AppProtocol appProtocol;
};

