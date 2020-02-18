/**
 *
 * @author <va.yastrebov>
 * @description
 * <pre>
 *    Class contains file adresses for client-server
 *    communications using unix domain sockets
 * </pre>
 *
 * @class UDS
 */

#pragma once

#include<string>

struct UDS
{
   inline const static std::string sClientSockFile = "/tmp/client.sock";
   inline const static std::string sServerSockFile = "/tmp/server7.sock";
};
