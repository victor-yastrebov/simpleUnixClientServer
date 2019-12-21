#include"UDSClient.h"
#include"../kvd/DataBase.h"
#include"../kvd/Daemon.h"

int main()
{
   /**/
   SysLogLogger sl;
   {
      Daemon daemon;

      // if(!daemon.Daemonise()) return 1;

      sl.LogToSyslog( "---New instance start---" );

      DataBase db( "/home/user/UnixClientServer/clientServerDB/implementation/kvd/my_db.txt" );
      db.ExecuteQuery( "put keystring1 valuestring2" );
      db.ExecuteQuery( "put keystring2 valuestring2" );
      db.ExecuteQuery( "put keystring3 valuestring3" );
      db.ExecuteQuery( "list" );
   }

   sl.LogToSyslog( "Finished success" );
   return 0;
   /**/

   UDSClient uds_client;
   uds_client.Connect();
   uds_client.StartSession();



   return 0;
}