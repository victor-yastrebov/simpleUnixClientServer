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
#include<set>

#include"DataBase.h"
#include"Client.h"

class UDSServer
{
public:
         UDSServer();
        ~UDSServer();
         UDSServer( const UDSServer& ) = delete;
         UDSServer& operator=( const UDSServer& ) = delete;
   int   startProcessing( const std::string &s_path_to_db );

private:
    SysLogLogger   sysLogger;
       const int   nMaxClients;
             int   listenerSocket;
   std::set<int>   connectedSockets;
          fd_set   readSet;

             int   prepareListenerSocket();
             int   processSockets( const std::string &s_path_to_db );
             int   processConnectedSockets( const std::string& s_path_to_db );
            void   processNewConnection();
            void   setNonBlockingMode();
             int   waitForEvents();
            void   closeSockets();
};