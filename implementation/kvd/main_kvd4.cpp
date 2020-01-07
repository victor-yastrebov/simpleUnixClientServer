#include<iostream>
#include<algorithm>

#include"asio.hpp"


#include<cstdio>
#include<array>
#include<memory>

#if defined(ASIO_HAS_LOCAL_SOCKETS)

using asio::local::stream_protocol;

class session
  : public std::enable_shared_from_this<session>
{
public:
  session(asio::io_service& io_service)
    : socket_(io_service)
  {
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
      // Compute result.
      for (std::size_t i = 0; i < bytes_transferred; ++i)
        data_[i] = std::toupper(data_[i]);

      asio::async_write(socket_,
          asio::buffer(data_, bytes_transferred),
          std::bind(&session::handle_write,
            shared_from_this(),
            std::placeholders::_1));
    }
  }

  void handle_write(const asio::error_code& error)
  {
    if (!error)
    {
      socket_.async_read_some(asio::buffer(data_),
          std::bind(&session::handle_read,
            shared_from_this(),
            std::placeholders::_1,
            std::placeholders::_2));
    }
  }

private:
  // The socket used to communicate with the client.
  stream_protocol::socket socket_;

  // Buffer used to store data received from the client.
  std::array<char, 1024> data_;
};

typedef std::shared_ptr<session> session_ptr;

class server
{
public:
  server(asio::io_service& io_service, const std::string& file)
    : io_service_(io_service),
      acceptor_(io_service, stream_protocol::endpoint(file))
  {
    session_ptr new_session(new session(io_service_));
    acceptor_.async_accept(new_session->socket(),
        std::bind(&server::handle_accept, this, new_session,
          std::placeholders::_1));
  }

  void handle_accept(session_ptr new_session,
      const asio::error_code& error)
  {
    if (!error)
    {
      new_session->start();
      new_session.reset(new session(io_service_));
      acceptor_.async_accept(new_session->socket(),
          std::bind(&server::handle_accept, this, new_session,
            std::placeholders::_1));
    }
  }

private:
  asio::io_service& io_service_;
  stream_protocol::acceptor acceptor_;
};

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: stream_server <file>\n";
      std::cerr << "*** WARNING: existing file is removed ***\n";
      return 1;
    }

    asio::io_service io_service;

    std::remove(argv[1]);
    server s(io_service, argv[1]);

    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}

#else // defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
# error Local sockets not available on this platform.
#endif // defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
