#include<iostream>

#include"UDS.h"
#include"UDSClient.h"

#if defined(ASIO_HAS_LOCAL_SOCKETS)

/**
 * Print application usage
 */
void PrintHelp()
{
   std::cout << "Client for database by va.yastrebov." << std::endl;
   std::cout << "Usage examples: " << std::endl;
   std::cout << " - PUT <key> <value>" << std::endl;
   std::cout << " - GET <key>" << std::endl;
   std::cout << " - ERASE [key]" << std::endl;
   std::cout << " - LIST [prefix]" << std::endl;
}

/**
 * Create query string from input CLI args
 */
std::string GetQuery( const int argc, const char *const argv[] )
{
   std::stringstream ss;
   for( int i = 1; i < argc; ++i)
   {
      ss << argv[i] << " ";
   }

   std::string s_query = ss.str();
   if( argc > 1 )
   {
      s_query.pop_back();
   }

   return s_query;
}

int main( int argc, char* argv[] )
{
  int ret_code = 0;

  try
  {
    const std::string s_query = GetQuery( argc, argv );

    if( s_query.empty() )
    {
       PrintHelp();
       return 1;
    }

    UDSClient client( UDS::sClientSockFile );

    if( ! client.Connect( UDS::sServerSockFile ) )
    {
       std::cout << "Connection to server failed" << std::endl;
       return 1;
    }

    ret_code = client.ProcSession( s_query );

  }
  catch( std::exception& e )
  {
    std::cout << "Exception: " << e.what() << std::endl;
    ret_code = 1;
  }
  catch( ... )
  {
     std::cout << "Unknow exception type" << std::endl;
     ret_code = 1;
  }

  return ret_code;
}

#else // defined(ASIO_HAS_LOCAL_SOCKETS)
# error Local sockets not available on this platform
#endif // defined(ASIO_HAS_LOCAL_SOCKETS)
