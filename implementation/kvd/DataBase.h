/**
 *
 * @author <va.yastrebov>
 * @description
 * <pre>
 *    Class is responsible for store/load data from database
 * </pre>
 *
 * @class DataBase
 */

#pragma once

#include<memory>
#include<string>
#include<optional>

#include"SysLogger.h"

enum class eQueryType
{
   qtLIST,
   qtPUT,
   qtGET,
   qtERASE,
   qtUNKNOWN
};

struct QueryInfo 
{
   std::string   sKey;
   std::string   sValue;
    eQueryType   eType;
};

class DataBase
{
public:
                 DataBase( std::shared_ptr<SysLogger> &p_logger );
                ~DataBase() = default;
                 DataBase( const DataBase& ) = delete;
      DataBase&  operator=( const DataBase& ) = delete;
   std::string   ProcessQuery( const std::string &s_query ) const;

private:
                    QueryInfo   ParseQueryString( const std::string &sw_query ) const;
                         bool   ProcessPutQuery( const QueryInfo &query_info ) const;
                  std::string   ProcessEraseQuery( const QueryInfo &query_info ) const;
                         bool   ProcessEraseKeyQuery( const std::string &s_key ) const;
                         void   ProcessEraseAllKeysQuery() const;
   std::optional<std::string>   ProcessGetQuery( const QueryInfo &query_info ) const;
                  std::string   ProcessListQuery( const QueryInfo &query_info ) const;
                         bool   CreateDbFolder() const noexcept;
                       size_t   Hash( const std::string& s) const noexcept;
                  std::string   ListKeys( const std::string& s_prefix = std::string() ) const;
                         bool   MatchListQuery( const std::string &s_prefix, const std::string &s_key ) const noexcept;

   std::shared_ptr<SysLogger>   pLogger;
            const std::string   sPathToDb;
       std::hash<std::string>   hashFn;
};
