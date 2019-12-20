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

      DataBase db;
      db.Initialize( "/home/user/UnixClientServer/clientServerDB/implementation/kvd/my_db.txt" );

      db.ExecuteQuery( "insert" );
      // db.ExecuteQuery( "insert 2 3 ilonmask@tesla.ru" );
      //db.ExecuteQuery( "select" );
      // db.ExecuteQuery( ".exit" );
    }

   sl.LogToSyslog( "Finished success" );
   return 0;
   /**/

   UDSClient uds_client;
   uds_client.Connect();
   uds_client.StartSession();



   return 0;
}