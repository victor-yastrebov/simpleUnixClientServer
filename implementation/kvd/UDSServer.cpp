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

#include<sys/un.h>
#include<syslog.h>
#include<netinet/in.h>
#include<signal.h>
#include<unistd.h>
#include<fcntl.h>

#include<algorithm>
#include<set>
#include<iostream>
#include<sstream>

// #include"UDS.h"
#include"UDSServer.h"

#include<cstdio>
#include<array>
#include<memory>



/**
 * CTOR
 */
UDSServer::UDSServer(asio::io_service io_service, const std::string &s_sock_file) :
   conServer( io_service, s_sock_file )
{

}

/**
 * DTOR
 */
UDSServer::~UDSServer()
{

}

/**
 * Listen and process incoming connections
 */
int UDSServer::StartProcessing()
{
   syslog( LOG_NOTICE, "KVD start processing" );

   std::string s_db_path = "/home/user/UnixClientServer/clientServerDB/implementation/kvd/my_db.txt";
   // db.Initialize( const_cast<char*>( s_db_path.c_str() ) );


   return 1;
}

/**
 * Process incoming DB query
 */
// C++17 std::stringstream
std::string UDSServer::ProcessQuery( const std::string &s_query ) const
{

   if(s_query == "LIST") return "bar baz max tRex";
   if(s_query == "PUT") return "PUT query OK";
   if(s_query == "GET") return "GET query OK";
   if(s_query == "ERASE") return "ERASE query OK";
   if(s_query == "STOP") return "STOP";

   return "UNKNOWN CMD";
}
