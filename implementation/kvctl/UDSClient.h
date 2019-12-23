/**
 *
 * @author <va.yastrebov>
 * @description
 * <pre>
 *    Class is responsible for quering data from DB
  * </pre>
 *
 * @class UDSClient
 */

#include<string>

class UDSClient
{
public:
          UDSClient();
         ~UDSClient() = default;
          UDSClient( const UDSClient& ) = delete;
          UDSClient& operator=( const UDSClient& ) = delete;
    int   Connect();
    int   Query( const std::string &s_query ) const;

private:
    int socketId;
};
