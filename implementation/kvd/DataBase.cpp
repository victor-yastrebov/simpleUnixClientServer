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
#include<filesystem>
#include<functional> // for std::hash
#include<fstream>

#include<vector>

#include "DataBase.h"

DataBase::DataBase() :
   sPathToDb( "/tmp/kvd/db" )
{
   CreateDbFolder();
}

QueryInfo DataBase::ParseQueryString( const std::string& s_query ) const
{
   QueryInfo query_info;
   query_info.eType = eQueryType::qtUNKNOWN;

   if( s_query.empty() ) 
   {
      return query_info;
   }

   std::string::size_type type_end_pos =
      s_query.find_first_of( ' ' );
   bool b_no_params = false;
   // query can be without arguments
   if( std::string::npos == type_end_pos )
   {
      type_end_pos = s_query.size(); 
      b_no_params = true;
   }

   std::string_view s_type = s_query.substr( 0, type_end_pos );

   // std::cout << "s_type is: " << s_type << std::endl;

   bool b_unknown_type = false;
   // fill in type
   if(s_type == "list") query_info.eType = eQueryType::qtLIST;
   else if(s_type == "put") query_info.eType  =  eQueryType::qtPUT;
   else if(s_type == "get") query_info.eType = eQueryType::qtGET;
   else if(s_type == "erase") query_info.eType = eQueryType::qtERASE;
   else b_unknown_type = true;
   
   if( b_no_params || b_unknown_type )
   {
      return query_info;
   }

   // fill in key
// std::cout << "tep: " << type_end_pos << std::endl;
   const std::string::size_type key_end_pos =
      s_query.find_first_of( ' ', type_end_pos + 1 );
   if( std::string::npos != key_end_pos ) 
   {
// std::cout << "1 " << key_end_pos << std::endl;
      query_info.sKey.assign(
         s_query.begin() + type_end_pos + 1,
         s_query.begin() + key_end_pos );
   }
   else
   {
// std::cout << "2" << std::endl;
      query_info.sKey.assign(
         s_query.begin() + type_end_pos + 1, s_query.end() );
      return query_info;
   }

   // fill in value 
   query_info.sValue.assign(
      s_query.begin() + key_end_pos + 1, s_query.end() );

   return query_info;
}

std::string DataBase::ProcessQuery( const std::string &s_query ) const
{
   const QueryInfo query_info = ParseQueryString( s_query ); 
   std::optional<std::string> s_ans;

   switch( query_info.eType )
   {
   case eQueryType::qtLIST:
      s_ans = ProcessListQuery( query_info );
      if( s_ans && s_ans.value().empty() )
      {
         s_ans = "kvctl: no any keys";
      }
      break;
   case eQueryType::qtPUT:
   {
      const bool status_ok = ProcessPutQuery( query_info );
      if( status_ok )
      {
         s_ans = "Query OK";
      }
      else
      {
         std::stringstream ss;
         ss << "kvctl: put key error ";
         ss << "\"" << query_info.sKey +"\"";
         s_ans = ss.str();
      }
      break;
   }
   case eQueryType::qtGET:
      s_ans = ProcessGetQuery( query_info );
      if( ! s_ans )
      {
         std::stringstream ss;
         ss << "kvctl: no key ";
         ss << "\"" << query_info.sKey +"\"";
         s_ans = ss.str();
      }
      break;
   case eQueryType::qtERASE:
   {
      s_ans = ProcessEraseQuery( query_info );
      break;
   }
   default:
      s_ans = "UNKNOWN_CMD";
      break;
   }

   // std::cout << "Key: " << query_info.sKey.size()  << std::endl;
   // std::cout << "Value: " << query_info.sValue.size() << std::endl;

   return s_ans.value();
}

bool DataBase::CreateDbFolder() const noexcept
{
   bool status_ok = true;

   std::error_code ec;
   std::filesystem::create_directories( sPathToDb, ec );
   if( ec )
   {
      std::cout << "ERROR: failed to create directory " << ec << std::endl;
      status_ok = false;
   }

   return status_ok;
}

size_t DataBase::Hash( const std::string& s) const noexcept
{
   return hashFn( s );
}

std::string DataBase::ProcessEraseQuery( const QueryInfo &query_info ) const
{
   std::string s_ret( "Query OK" );

   if( query_info.sKey.empty() )
   {
      ProcessEraseAllKeysQuery();
   }
   else
   {
      const bool status_ok = ProcessEraseKeyQuery( query_info.sKey );
      if( false == status_ok )
      {
         std::stringstream ss;
         ss << "kvctl: no key ";
         ss << "\"" << query_info.sKey +"\"";
         s_ret = ss.str();
      }
   }

   return s_ret;
}

bool DataBase::ProcessEraseKeyQuery( const std::string &s_key ) const
{
   const size_t str_hash = Hash( s_key );
   const std::string s_path_to_record = sPathToDb + "//" + std::to_string( str_hash );

   std::error_code ec;
   if( false == std::filesystem::exists( s_path_to_record, ec ) )
   {
      return false;
   }

   std::fstream fs( s_path_to_record, std::ios::in | std::ios::out );  

   size_t key_len = 0;
   fs >> key_len;
   fs.ignore( 1 );   // ignore '\n'

   fs.ignore( key_len );
   fs.ignore( 1 );   // ignore '\n'

   int is_valid = 0;
   fs << is_valid ;

   return true;
}

void DataBase::ProcessEraseAllKeysQuery() const
{
   namespace fs = std::filesystem;

   std::string s_result;

   for( auto& p: fs::directory_iterator( sPathToDb ) )
   {
      std::string s_path_to_record = p.path();
      std::fstream fs( s_path_to_record, std::ios::in | std::ios::out );

      size_t key_len = 0;
      fs >> key_len;
      fs.ignore( 1 );   // ignore '\n'

      fs.ignore( key_len );
      fs.ignore( 1 );   // ignore '\n'

      int is_valid = 0;
      fs << is_valid ;
   }
}

std::string DataBase::ProcessListQuery( const QueryInfo &query_info ) const
{
   return ListKeys( query_info.sKey );
}

std::string DataBase::ListKeys( const std::string& s_prefix /*= std::string()*/ ) const
{
   namespace fs = std::filesystem;

   std::string s_result;

   for( auto& p: fs::directory_iterator( sPathToDb ) )
   {
      std::string s_path_to_record = p.path();
      // std::cout << p.path() << '\n';

      std::ifstream fs( s_path_to_record );  

      size_t key_len = 0;
      fs >> key_len;
      fs.ignore( 1 );   // ignore '\n'
      // std::cout << "Key len is: " << key_len << std::endl;

      std::string s_key;
      s_key.resize( key_len );
      fs.read( s_key.data(), key_len );
      fs.ignore( 1 );   // ignore '\n'
      // std::cout << "Key is: " << s_key << std::endl;

      int is_valid = 0;
      fs >> is_valid ;
      fs.ignore( 1 );   // ignore '\n'
      // std::cout << "Is valid: " << is_valid << std::endl;
    
      if( 0 == is_valid )
      {
         continue;
      }

      if( MatchListQuery( s_prefix, s_key ) )
      {
         s_result += s_key;
         s_result += '\n';
      }
   }

   if( !s_result.empty() )
   {
      s_result.pop_back();
   }

  // std::cout << "s_result is: " << s_result << std::endl;
  return s_result;
}

bool DataBase::MatchListQuery( const std::string &s_prefix,
   const std::string &s_key ) const noexcept
{
   if( s_prefix.empty() ) return true;

   return ( 0 == s_key.compare( 0, s_prefix.size(), s_prefix ) );
}

std::optional<std::string> DataBase::ProcessGetQuery( const QueryInfo &query_info ) const
{
   const size_t str_hash = Hash( query_info.sKey );
   const std::string s_path_to_record = sPathToDb + "//" + std::to_string( str_hash );

   std::error_code ec;
   if( false == std::filesystem::exists( s_path_to_record, ec ) )
   {
      return std::nullopt;
   }

   // std::ifstream fs( s_path_to_record, std::ifstream::binary );  
   std::ifstream fs( s_path_to_record );  

   size_t key_len = 0;
   fs >> key_len;
   fs.ignore( 1 );   // ignore '\n'
   // std::cout << "Key len is: " << key_len << std::endl;

   std::string s_key;
   s_key.resize( key_len );
   fs.read( s_key.data(), key_len );
   fs.ignore( 1 );   // ignore '\n'
   // std::cout << "Key is: " << s_key << std::endl;

   int is_valid = 0;
   fs >> is_valid ;
   fs.ignore( 1 );   // ignore '\n'
   // std::cout << "Is valid: " << is_valid << std::endl;
    
   if( 0 == is_valid )
   {
      return std::nullopt;
   }

   size_t value_len = 0;
   fs >> value_len;
   fs.ignore( 1 );
   // std::cout << "Value len is: " << value_len << std::endl;

   std::string s_value;
   s_value.resize( value_len );
   fs.read( s_value.data(), value_len);

   // std::cout << "Value is: " << s_value << std::endl;

   return s_value;
}

bool DataBase::ProcessPutQuery( const QueryInfo &query_info ) const
{
   const size_t str_hash = Hash( query_info.sKey );
   // std::cout << "Hash is: " << str_hash << std::endl;
   const int b_row_is_not_deleted = 1;

   std::ofstream of( sPathToDb + "//" + std::to_string( str_hash ) );  
   of << query_info.sKey.size() << std::endl;
   of << query_info.sKey << std::endl;
   of << b_row_is_not_deleted << std::endl;
   of << query_info.sValue.size() << std::endl;
   of << query_info.sValue;

   if( of.bad() )
   {
      return false;
   }

   return true;
}