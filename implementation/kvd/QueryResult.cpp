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

#include"QueryResult.h"

/**
 * CTOR
 */
QueryResult::QueryResult() :
   queryStatus( eQueryStatus::esNotExecuted )
{

}

/**
 * CTOR
 */
QueryResult::QueryResult( const eQueryStatus &eqs ) :
   queryStatus( eqs )
{

}

/**
 * CTOR
 */
QueryResult::QueryResult( const eQueryStatus &eqs, const std::string &s_data ) :
   queryStatus( eqs ),
   sData( s_data )
{

}