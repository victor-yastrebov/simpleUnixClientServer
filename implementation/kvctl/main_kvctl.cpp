#include"UDSClient.h"
#include"../kvd/DataBase.h"
#include"../kvd/Daemon.h"
#include"../kvd/SysLogger.h"

#include"AppProtocol.h"

void printHelp()
{
   std::cout << "Client for database by va.yastrebov." << std::endl;
   std::cout << "Usage examples: " << std::endl;
   std::cout << " - PUT <key> <value>" << std::endl;
   std::cout << " - GET <key>" << std::endl;
   std::cout << " - ERASE [key]" << std::endl;
   std::cout << " - LIST [prefix]" << std::endl;
}

/**
 * Create query string from input CLI args
 */
std::string getQuery( const int argc, const char *const argv[] )
{
   std::stringstream ss;
   for( int i = 1; i < argc; ++i)
   {
      ss << argv[i] << " ";
   }

   std::string s_query = ss.str();
   if( argc > 1 )
   {
      // s_query.pop_back();
      s_query.resize( s_query.size() - 1 );
   }

   return s_query;
}

void testMe(std::string s_query)
{
   AppProtocol ap;
   std::vector<unsigned char> v = ap.encodeMsg( s_query );
   bool status_ok = false;
   std::string ret = ap.decodeMsg( v, status_ok );

   std::cout << s_query << std::endl;
   std::cout << ret << std::endl;
}

int main( int argc, char *argv[] )
{
   const std::string s_query = getQuery( argc, argv );

   // testMe( s_query );

   if( s_query.empty() )
   {
      printHelp();
      return 0;
   }

   SysLogger sl;
   {
      Daemon daemon;

      // if(!daemon.Daemonise()) return 1;
      // sl.LogToSyslog( "---New instance start---" );

      // DataBase db( "/tmp/my_db.txt" );
      // DataBase db( "/tmp/my_db_dummy.txt" );
      //db.ExecuteQuery( ".exit" );

      // return 1;

      // auto ret = db.ExecuteQuery( "list" );
      // db.ExecuteQuery( ".constants" );
      // return 1;
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
   if( ! uds_client.Connect() )
   {
      return 1;
   };
   return uds_client.Query( s_query );
}
