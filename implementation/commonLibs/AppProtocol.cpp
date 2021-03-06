/**
 *
 * @author <va.yastrebov>
 * @description
 * <pre>
 *    Class is responsible encoding and decoding application protocol sections
 *    that are used for communications between client and server
 * </pre>
 *
 * @class AppProtocol
 */

#include<iostream>

#include"AppProtocol.h"

/**
 * CTOR
 */
AppProtocol::AppProtocol() :
   nHeaderSize( 4 )
{

}

/**
 * Encode query into bytestream for furher transmitting
 */
std::vector<BYTE> AppProtocol::EncodeMsg( const std::string &s_msg ) const
{
   std::vector<BYTE> v_msg( s_msg.cbegin(), s_msg.cend() );

   // first 4 bytes - describes full packet length
   const int n_full_packet_len = s_msg.size() + nHeaderSize;

   std::vector<BYTE> v_packet;
   v_packet.reserve( n_full_packet_len );

   std::vector<BYTE> v_len = GenLenField( n_full_packet_len );

   // put full packet length
   v_packet.insert( v_packet.end(),
      std::make_move_iterator( v_len.begin() ),
      std::make_move_iterator( v_len.end() )
   );

   // put payload
   v_packet.insert( v_packet.end(),
      std::make_move_iterator( v_msg.begin() ),
      std::make_move_iterator( v_msg.end() )
   );

   return v_packet;
}

/**
 * Decode result of query from bytestream
 */
std::string AppProtocol::DecodeMsg(
   const std::vector<BYTE> &v_packet, bool &status_ok ) const
{
   status_ok = false;

   if( v_packet.size() < nHeaderSize )
   {
      // cannot get packet length field
      return std::string();
   }

   itData = v_packet.begin();

   const int n_packet_size = GetFieldValueOfFourBytesLen();
   if( v_packet.size() < n_packet_size )
   {
      // full packet is not received
      return std::string();
   }

   std::vector<BYTE>::const_iterator it_msg_end = v_packet.end();
   const std::string s_msg( itData, it_msg_end );

   if( s_msg.size() != n_packet_size - nHeaderSize )
   {
      std::cout << "Error. Incorrect packet len" << std::endl;
   }
   else
   {
      status_ok = true;
   }

   return s_msg;
}

/**
 * Generate length field
 */
std::vector<BYTE> AppProtocol::GenLenField( const int n_msg_len ) const
{
   std::vector<BYTE> v_len;
   v_len.push_back( GetByteInPosition( n_msg_len, 3) );
   v_len.push_back( GetByteInPosition( n_msg_len, 2) );
   v_len.push_back( GetByteInPosition( n_msg_len, 1) );
   v_len.push_back( GetByteInPosition( n_msg_len, 0) );

   return v_len;
}

/**
 * Get value of the byte on the given position
 * The least significant byte is on the position 0 (the most right).
 */
BYTE AppProtocol::GetByteInPosition( const int val, const int pos ) const
{
   return ( val >> ( pos * 8 ) ) & 0xFF;
}

/**
 * Get value of four bytes as int
 */
int AppProtocol::GetFieldValueOfFourBytesLen() const
{
   int field_val = 0;

   field_val = ( *itData << 8 ) | ( *std::next( itData ) );
   // pass two already analysed bytes
   itData += 2;
   // put already read bytes on the position of the MSB
   field_val = field_val << 2 * 8;
   // add value of LSB
   field_val += ( *itData << 8 ) | ( *std::next( itData ) );
   // pass two already analysed bytes
   itData += 2;

   return field_val;
}
