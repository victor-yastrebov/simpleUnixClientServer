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

#include<string>
#include<vector>

using BYTE = unsigned char;
using DataConstIterator = std::vector<BYTE>::const_iterator;

class AppProtocol
{
public:
                       AppProtocol();
                      ~AppProtocol() = default;
                       AppProtocol( const AppProtocol& ) = delete;
                       AppProtocol& operator=( const AppProtocol& ) = delete;
   std::vector<BYTE>   EncodeMsg( const std::string &s_msg ) const;
         std::string   DecodeMsg( const std::vector<BYTE> &v_packet, bool &status_ok ) const;

private:
                        BYTE   GetByteInPosition( const int val, const int pos ) const;
                         int   GetFieldValueOfFourBytesLen() const;
           std::vector<BYTE>   GenLenField( const int len ) const;

   mutable DataConstIterator   itData;
                   const int   nHeaderSize;
};

