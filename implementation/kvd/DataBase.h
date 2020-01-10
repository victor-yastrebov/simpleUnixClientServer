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

#include<string>
#include<optional>

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
   std::string sKey;
   std::string sValue;
   eQueryType  eType;
};

class DataBase
{
public:
   DataBase();
  ~DataBase() = default;
   DataBase( const DataBase& ) = delete;
   DataBase& operator=( const DataBase& ) = delete;

   std::string   ProcessQuery( const std::string &s_query ) const;

private:
public:
   QueryInfo ParseQueryString( const std::string &sw_query ) const;
   bool   ProcessPutQuery( const QueryInfo &query_info ) const;
   bool   ProcessEraseQuery( const QueryInfo &query_info ) const;
   std::optional<std::string>   ProcessGetQuery( const QueryInfo &query_info ) const;
     bool   CreateDbFolder() const noexcept;
   size_t   Hash( const std::string& s) const noexcept;

        const std::string   sPathToDb;
   std::hash<std::string>   hashFn;
};
