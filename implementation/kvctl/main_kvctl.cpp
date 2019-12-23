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

      // DataBase db( "/home/user/UnixClientServer/clientServerDB/implementation/kvd/my_db.txt" );
      // db.ExecuteQuery( ".constants" );
/*
      db.ExecuteQuery( "put aeystring1 valuestring1" );
      db.ExecuteQuery( "put beystring2 valuestring2" );
      db.ExecuteQuery( "put ceystring3 valuestring3" );
      db.ExecuteQuery( "put deystring4 valuestring4" );
      db.ExecuteQuery( "put eeystring5 valuestring5" );
      db.ExecuteQuery( "put feystring6 valuestring6" );


      db.ExecuteQuery( "put geystring7 valuestring7" );
      db.ExecuteQuery( "put heystring8 valuestring8" );
      db.ExecuteQuery( "put ieystring9 valuestring9" );
      db.ExecuteQuery( "put jeystring10 valuestring10" );
      db.ExecuteQuery( "put keystring11 valuestring11" );
      db.ExecuteQuery( "put leystring12 valuestring12" );
      db.ExecuteQuery( "put meystring13 valuestring13" );
      db.ExecuteQuery( "put neystring14 valuestring14" );
      db.ExecuteQuery( "put oeystring15 valuestring15" );
      db.ExecuteQuery( "put peystring16 valuestring16" );

      const QueryResult query_result = db.ExecuteQuery( "list" );

      std::string s( query_result.sData.begin(),query_result.sData.end() );

      std::cout << query_result.sData << std::endl;

      // db.ExecuteQuery( ".btree" );
*/
   }

   // sl.LogToSyslog( "Finished success" );
   // return 0;

   UDSClient uds_client;
   uds_client.Connect();
   uds_client.StartSession();



   return 0;
}