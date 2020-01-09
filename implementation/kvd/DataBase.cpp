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

#include "DataBase.h"

std::string DataBase::ProcessQuery( const std::string &s_query ) const
{
   if(s_query == "LIST") return "bar baz max tRex";
   if(s_query == "PUT") return "PUT query OK";
   if(s_query == "GET") return "GET query OK";
   if(s_query == "ERASE") return "ERASE query OK";
   if(s_query == "STOP") return "STOP";

   return "UNKNOWN_CMD";
/*
   std::string ret;
   ret.resize( s_query.size() );

   for (std::size_t i = 0; i < s_query.size(); ++i)
     ret[i] = std::toupper(s_query[i]);


   return ret;
*/
}

