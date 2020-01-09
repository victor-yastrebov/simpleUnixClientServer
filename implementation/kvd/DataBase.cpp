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

#include<string_view>
#include<iostream>

#include "DataBase.h"

eQueryType DataBase::GetQueryType( const std::string& s_query ) const
{
   eQueryType query_type = eQueryType::qtUNKNOWN;

   if( s_query.empty() ) 
   {
      return query_type;
   }

   std::string::size_type pos = s_query.find_first_of( ' ' );
   // query can be without arguments
   if( std::string::npos == pos )
   {
      pos = s_query.size(); 
   }

   std::string_view s_type = s_query.substr( 0, pos );

   std::cout << "s_type is: " << s_type << std::endl;

   if(s_type == "LIST") query_type = eQueryType::qtLIST;
   if(s_type == "PUT") query_type =  eQueryType::qtPUT;
   if(s_type == "GET") query_type = eQueryType::qtGET;
   if(s_type == "ERASE") query_type = eQueryType::qtERASE;

   return query_type;
}

std::string DataBase::ProcessQuery( const std::string &s_query ) const
{
   const eQueryType query_type = GetQueryType( s_query ); 
   std::string s_ans;

   switch( query_type )
   {
   case eQueryType::qtLIST:
      s_ans = "bar baz max tRex";
      break;
   case eQueryType::qtPUT:
      s_ans = "PUT query OK";
      break;
   case eQueryType::qtGET:
      s_ans = "GET query OK";
      break;
   case eQueryType::qtERASE:
      s_ans = "ERASE query OK";
      break;
   default:
      s_ans = "UNKNOWN_CMD";
      break;
   }

   return s_ans;
}
