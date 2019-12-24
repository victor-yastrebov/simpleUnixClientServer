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

#include"UDS.h"

// In C++17 we can declare and define variables in a header file
std::string UDS::sClientSockFile = "/tmp/client.sock";
std::string UDS::sServerSockFile = "/tmp/server.sock";