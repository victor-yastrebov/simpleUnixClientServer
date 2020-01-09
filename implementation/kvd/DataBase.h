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

class DataBase
{
public:
   DataBase() = default;
  ~DataBase() = default;
   DataBase( const DataBase& ) = delete;
   DataBase& operator=( const DataBase& ) = delete;

   std::string   ProcessQuery( const std::string &s_query ) const;
};
