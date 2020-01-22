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
   inline const static std::string sClientSockFile = "/tmp/client2.sock";
   inline const static std::string sServerSockFile = "/tmp/server.sock";
};