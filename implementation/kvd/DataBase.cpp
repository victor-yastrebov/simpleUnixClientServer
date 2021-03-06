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

/**
 * CTOR
 */
DataBase::DataBase( std::shared_ptr<SysLogger> &p_logger ) :
   pLogger( p_logger ),
   sPathToDb( "/tmp/kvd/db" )
{
   CreateDbFolder();
}

/**
 * Parse client query string
 */
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

   bool b_unknown_type = false;
   // fill in type
   if( s_type == "list" ) query_info.eType = eQueryType::qtLIST;
   else if( s_type == "put" ) query_info.eType  =  eQueryType::qtPUT;
   else if( s_type == "get" ) query_info.eType = eQueryType::qtGET;
   else if( s_type == "erase" ) query_info.eType = eQueryType::qtERASE;
   else b_unknown_type = true;

   if( b_no_params || b_unknown_type )
   {
      return query_info;
   }

   // fill in key
   const std::string::size_type key_end_pos =
      s_query.find_first_of( ' ', type_end_pos + 1 );
   if( std::string::npos != key_end_pos ) 
   {
      query_info.sKey.assign(
         s_query.begin() + type_end_pos + 1,
         s_query.begin() + key_end_pos );
   }
   else
   {
      query_info.sKey.assign(
         s_query.begin() + type_end_pos + 1, s_query.end() );
      return query_info;
   }

   // fill in value 
   query_info.sValue.assign(
      s_query.begin() + key_end_pos + 1, s_query.end() );

   return query_info;
}

/**
 * Process client query
 */
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
         s_ans = "Query OK";
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
      s_ans = "UNKNOWN COMMAND";
      break;
   }

   return s_ans.value();
}

/**
 * Create folder that will store entire DB
 */
bool DataBase::CreateDbFolder() const noexcept
{
   bool status_ok = true;

   std::error_code ec;
   std::filesystem::create_directories( sPathToDb, ec );
   if( ec )
   {
      pLogger->Log( "ERROR: failed to create directory ", ec );
      status_ok = false;
   }

   return status_ok;
}

/**
 * Calculate hash value
 */
size_t DataBase::Hash( const std::string& s) const noexcept
{
   return hashFn( s );
}

/**
 * Process erase query
 */
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

/**
 * Process erase query for particular key
 */
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

/**
 * Process erase query for all keys
 */
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

/**
 * Process list query
 */
std::string DataBase::ProcessListQuery( const QueryInfo &query_info ) const
{
   return ListKeys( query_info.sKey );
}

/**
 * Find all keys in database. If prefix is set only keys that starts with this
 * prefix is returned
 */
std::string DataBase::ListKeys( const std::string& s_prefix /*= std::string()*/ ) const
{
   namespace fs = std::filesystem;

   std::string s_result;

   for( auto& p: fs::directory_iterator( sPathToDb ) )
   {
      std::string s_path_to_record = p.path();

      std::ifstream fs( s_path_to_record );  

      size_t key_len = 0;
      fs >> key_len;
      fs.ignore( 1 );   // ignore '\n'

      std::string s_key;
      s_key.resize( key_len );
      fs.read( s_key.data(), key_len );
      fs.ignore( 1 );   // ignore '\n'

      int is_valid = 0;
      fs >> is_valid ;
      fs.ignore( 1 );   // ignore '\n'

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

  return s_result;
}

/**
 * Analyze wheter current key should be returned in list query or not
 */
bool DataBase::MatchListQuery( const std::string &s_prefix,
   const std::string &s_key ) const noexcept
{
   if( s_prefix.empty() ) return true;

   return ( 0 == s_key.compare( 0, s_prefix.size(), s_prefix ) );
}

/**
 * Process get value for particular key from DB query
 */
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

   std::string s_key;
   s_key.resize( key_len );
   fs.read( s_key.data(), key_len );
   fs.ignore( 1 );   // ignore '\n'

   int is_valid = 0;
   fs >> is_valid ;
   fs.ignore( 1 );   // ignore '\n'

   if( 0 == is_valid )
   {
      return std::nullopt;
   }

   size_t value_len = 0;
   fs >> value_len;
   fs.ignore( 1 );

   std::string s_value;
   s_value.resize( value_len );
   fs.read( s_value.data(), value_len);

   return s_value;
}

/**
 * Process insert key/value query into DB
 */
bool DataBase::ProcessPutQuery( const QueryInfo &query_info ) const
{
   const size_t str_hash = Hash( query_info.sKey );
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
