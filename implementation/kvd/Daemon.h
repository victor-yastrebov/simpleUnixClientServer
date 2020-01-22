/**
 *
 * @author <va.yastrebov>
 * @description
 * <pre>
 *    Class is responsible for demonisation of application
  * </pre>
 *
 * @class Daemon
 */

#include<memory>

#include"SysLogger.h"

class Daemon
{
public:
          Daemon( std::shared_ptr<SysLogger> &p_logger );
         ~Daemon();
          Daemon( const Daemon& ) = delete;
          Daemon& operator=( const Daemon& ) = delete;
   bool   Daemonise();

private:
   std::shared_ptr<SysLogger>   pLogger;
                         bool   isDaemon;
};
