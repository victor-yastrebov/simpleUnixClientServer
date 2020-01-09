#include <iostream>
#include <string>
#include <cctype>
#include "asio.hpp"

#include<thread>
#include<array>
//#include <boost/thread.hpp>
//#include <boost/array.hpp>
//#include <boost/bind.hpp>

#if defined(ASIO_HAS_LOCAL_SOCKETS)

using asio::local::stream_protocol;

class uppercase_filter
{
public:
  uppercase_filter(asio::io_service& io_service)
    : socket_(io_service)
  {
  }

  stream_protocol::socket& socket()
  {
    return socket_;
  }

  void start()
  {
    // Wait for request.
    socket_.async_read_some(asio::buffer(data_),
        std::bind(&uppercase_filter::handle_read,
          this, std::placeholders::_1,
          std::placeholders::_2));
  }

private:
  void handle_read(const std::error_code& ec, std::size_t size)
  {
    if (!ec)
    {
      // Compute result.
      for (std::size_t i = 0; i < size; ++i)
        data_[i] = std::toupper(data_[i]);

      // Send result.
      asio::async_write(socket_, asio::buffer(data_, size),
          std::bind(&uppercase_filter::handle_write,
            this, std::placeholders::_1));
    }
    else
    {
      throw std::system_error(ec);
    }
  }

  void handle_write(const std::error_code& ec)
  {
    if (!ec)
    {
      // Wait for request.
      socket_.async_read_some(asio::buffer(data_),
          std::bind(&uppercase_filter::handle_read,
            this, std::placeholders::_1,
            std::placeholders::_2));
    }
    else
    {
      throw std::system_error(ec);
    }
  }

  stream_protocol::socket socket_;
  std::array<char, 512> data_;
};

void run(asio::io_service* io_service)
{
  try
  {
    io_service->run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception in thread: " << e.what() << "\n";
    std::exit(1);
  }
}

int main()
{
  try
  {
    asio::io_service io_service;

    // Create filter and establish a connection to it.
    uppercase_filter filter(io_service);
    stream_protocol::socket socket(io_service);
    asio::local::connect_pair(socket, filter.socket());
    filter.start();

    // The io_service runs in a background thread to perform filtering.
    std::thread thread(std::bind(run, &io_service));

    for (;;)
    {
      // Collect request from user.
      std::cout << "Enter a string: ";
      std::string request;
      std::getline(std::cin, request);

      // Send request to filter.
      asio::write(socket, asio::buffer(request));

      // Wait for reply from filter.
      std::vector<char> reply(request.size());
      asio::read(socket, asio::buffer(reply));

      // Show reply to user.
      std::cout << "Result: ";
      std::cout.write(&reply[0], request.size());
      std::cout << std::endl;
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
    std::exit(1);
  }
}

#else // defined(ASIO_HAS_LOCAL_SOCKETS)
# error Local sockets not available on this platform.
#endif // defined(ASIO_HAS_LOCAL_SOCKETS)
