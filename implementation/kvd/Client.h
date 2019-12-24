/**
 *
 * @author <va.yastrebov>
 * @description
 * <pre>
 *    Class is responsible for working with client queries
  * </pre>
 *
 * @class Client 
 */

#pragma once

#include<vector>



class Client
{
public:
                       Client();
                      ~Client();
                       Client( const Client& ) = delete;
                       Client& operator=( const Client& ) = delete;
                void   getSocketId() const;

   // std::vector<BYTE>   vRecvData;
private:
   int   socketId;
};
