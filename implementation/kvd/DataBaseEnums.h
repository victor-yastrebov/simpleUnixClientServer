/**
 *
 * @author <va.yastrebov>
 * @description
 * <pre>
 *    These file contains enums that are used by DataBase for working
  * </pre>
 *
 * @class DataBaseStuff
 */


#pragma once

enum class eStatementType
{
   stPut,
   stList
};

enum class eMetaCmdResult
{
  mcrSuccess,
  mcrUnrecognisedCmd
} ;

enum class ePrepareResult
{
  prSuccess,
  prNegativeId,
  prStringTooLong,
  prSyntaxError,
  prUnrecognisedStatement
};

enum eNodeType : int
{
    ntInternal = 0,
    ntLeaf = 1
};
