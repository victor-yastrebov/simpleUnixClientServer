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

      DataBase db;
      db.Initialize( "/home/user/UnixClientServer/clientServerDB/implementation/kvd/my_db.txt" );

      db.ExecuteQuery( "insert keystring valuestring" );

      return 1;

      // db.ExecuteQuery( "insert" );
      db.ExecuteQuery( "insert 2 ilonmask ilonmask@tesla.ru" );
      db.ExecuteQuery( "insert 3 vayastrebov yastrebov@corp123.ru" );
      db.ExecuteQuery( "select" );
      db.ExecuteQuery( ".btree" );
      db.ExecuteQuery( ".exit" );
    }

   sl.LogToSyslog( "Finished success" );
   return 0;
   /**/

   UDSClient uds_client;
   uds_client.Connect();
   uds_client.StartSession();



   return 0;
}