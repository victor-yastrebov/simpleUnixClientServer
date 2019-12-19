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

class UDSClient
{
public:
          UDSClient() = default;
         ~UDSClient() = default;
          UDSClient( const UDSClient& ) = delete;
          UDSClient& operator=( const UDSClient& ) = delete;
    int   Connect();
    int   StartSession() const;
};
