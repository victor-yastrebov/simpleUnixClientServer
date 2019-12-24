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

// C++17 has built-in type "byte"
using BYTE = unsigned char;
using DataConstIterator = std::vector<BYTE>::const_iterator;

class AppProtocol
{
public:
                       AppProtocol() = default;
                      ~AppProtocol() = default;
                       AppProtocol( const AppProtocol& ) = delete;
                       AppProtocol& operator=( const AppProtocol& ) = delete;
   std::vector<BYTE>   encodeMsg( const std::string &s_msg ) const;
         std::string   decodeMsg( const std::vector<BYTE> &v_packet, bool &status_ok ) const;

private:
                           BYTE   getByteInPosition( const int val, const int pos ) const;
                            int   getFieldValueOfFourBytesLen() const;
              std::vector<BYTE>   genLenField( const int len ) const;

      mutable DataConstIterator   itData;
                      const int   nHeaderSize = 4;
};
