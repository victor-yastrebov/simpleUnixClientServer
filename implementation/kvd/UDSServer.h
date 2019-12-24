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

// implemented via:
// https://stackoverflow.com/questions/17954432/creating-a-daemon-in-linux/17955149#17955149

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
   std::string   ProcessQuery( const std::string &s_query ) const;

private:
    SysLogLogger   sysLogger;
       const int   nMaxClients;
};
