/**
 *
 * @author <va.yastrebov>
 * @description
 * <pre>
 *    Class is responsible for processing incoming connections from clients
  * </pre>
 *
 * @class UDSServer
 */

#include<string>

#include"DataBase.h"
#include"Client.h"

class UDSServer
{
public:
        UDSServer();
       ~UDSServer();
        UDSServer( const UDSServer& ) = delete;
        UDSServer& operator=( const UDSServer& ) = delete;
   int   StartProcessing();

private:
    SysLogLogger   sysLogger;
       const int   nMaxClients;
};
