/**
 *
 * @author <va.yastrebov>
 * @description
 * <pre>
 *    Class contains file adressed for Client-Server
 *    Unix Domain Socket communications
 * </pre>
 *
 * @class UDS
 */

#pragma once

#include<string>

struct UDS
{
   // In C++17 we can declare and define variables in a header file
   static std::string sClientSockFile;
   static std::string sServerSockFile;
};