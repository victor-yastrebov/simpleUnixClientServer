/**
 *
 * @author <va.yastrebov>
 * @description
 * <pre>
 *    Class is responsible describing result of a query to Database
  * </pre>
 *
 * @class QueryResult
 */

#pragma once

#include<string>

enum class eQueryStatus
{
   esNotExecuted,
   esSuccss,
   esDuplicateKey,
   esInvalidCmd
};

struct QueryResult
{
                  QueryResult();
                  QueryResult( const eQueryStatus &eqs );
                  QueryResult( const eQueryStatus &eqs, const std::string &s_data );
                 ~QueryResult() = default;

   eQueryStatus   queryStatus;
    std::string   sData;
};