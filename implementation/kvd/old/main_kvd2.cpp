#include<iostream>
#include<memory>

#include"asio.hpp"

constexpr int getA()
{
   return 42;
}

int main(void)
{

#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
   std::cout << "Yeah" << std::endl;
#else
   std::cout << "No((" << std::endl;
#endif

   std::unique_ptr<int> p;
   p = std::make_unique<int>( 42 );

   asio::io_context io_context;
   asio::ip::tcp::socket sock( io_context );
   asio::local::stream_protocol::socket socket( io_context );

   if constexpr( const int val = getA(); val > 0 )
   {
      std::cout << "A is greater than zero" << std::endl;
   }

   std::cout << "Hello world From C++17 using console" << std::endl;
   return 1;
}
