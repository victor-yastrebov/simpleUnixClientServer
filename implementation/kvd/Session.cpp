/**
 *
 * @author <va.yastrebov>
 * @description
 * <pre>
 *    Class is responsible for processing clients queries
  * </pre>
 *
 * @class Session
 */

#include"Session.h"

/**
 * CTOR
 */
Session::Session( asio::io_service& io_service, std::shared_ptr<DataBase> &p_db, const size_t id ) :
    mSocket( io_service ),
    pDataBase( p_db ),
    sessionIsOverEvent( nullptr ),
    stopServerEvent( nullptr ),
    nId( id )
  {
     std::cout << "session CTOR with id: " << getId() << std::endl;
  }

/**
 * DTOR
 */
Session::~Session()
{
   try
   {
      std::cout << "session DTOR with id: " << getId() << std::endl;

      asio::error_code ec;
      mSocket.shutdown( asio::local::stream_protocol::socket::shutdown_both, ec);
      mSocket.close( ec );

      SessionIsOverNotify();
   }
   catch( std::exception &e )
   {
      std::cout << "session DTOR exception caught: " << e.what() << std::endl;
   }
   catch( ... )
   {
      std::cout << "session DTOR unknown exception caught" << std::endl;
   }
}

/**
 * Session socket getter
 */
local_str_proto::socket& Session::getSocket()
{
   return mSocket;
}

/**
 * Start new session with client
 */
void Session::Start()
{
  mSocket.async_read_some(
     asio::buffer( mData ),
     std::bind( &Session::HandleRead,
        shared_from_this(),
        std::placeholders::_1,
        std::placeholders::_2
     )
  );
}

/**
 * Read handler
 */
void Session::HandleRead( const asio::error_code& error, size_t bytes_transferred )
{
   if( !error )
   {
      bool status_ok = false;
      const std::string s_query = appProtocol.decodeMsg(
         std::vector<BYTE>( mData.begin(), mData.begin() + bytes_transferred ), status_ok );

      if( ! status_ok )
      {
         std::cout << "Received msg is not full" << std::endl;
         return;
      }

      std::cout << "Recv query: " << s_query << std::endl;
      // Command for stopping server
      if( s_query == "exit")
      {
         StopServerNotify();
         return;
      }

      const std::string ret = pDataBase->ProcessQuery( s_query );

      SendMsg( ret );
   }
   else
   {
      std::cout << "handle_read error: " << error << std::endl;
   }
}

/**
 * Send msg to remote
 */
void Session::SendMsg( const std::string &s_msg )
{
   std::vector<BYTE> v_enc_data =
      appProtocol.encodeMsg( s_msg );

   for( std::size_t i = 0; i < v_enc_data.size(); ++i )
   {
      mData[i] = v_enc_data[i];
   }

   std::cout << "Bytes to send: " << v_enc_data.size() << std::endl;
   // std::cout << "Before: " << mSocket.get_io_service().stopped() << std::endl;
   // mSocket.get_io_service().reset();

   // std::cout << "After: " << mSocket.get_io_service().stopped() << std::endl;
   asio::async_write(
      mSocket,
      asio::buffer( mData, v_enc_data.size() ),
      std::bind(
         &Session::HandleWrite,
         shared_from_this(),
         std::placeholders::_1,
         std::placeholders::_2
      )
   );
}

/**
 * Write handler
 */
void Session::HandleWrite( const asio::error_code& error, std::size_t bytes_transferred )
{
   if( error )
   {
      std::cout << "HandleWrite() error: " << error << std::endl;
   }

   std::cout << "HandleWrite end: " << bytes_transferred << "ec: " << error << std::endl;
}

/**
 * Notify server about connection finish
 */
void Session::SessionIsOverNotify()
{
   if( sessionIsOverEvent != nullptr )
   {
      sessionIsOverEvent( getId() );
   }
}

/**
 * Sent to server stop command for stop processing
 */
void Session::StopServerNotify()
{
   if( stopServerEvent != nullptr ) stopServerEvent();
}

/**
 * Get session id
 */
size_t Session::getId() const noexcept
{
   return nId;
}