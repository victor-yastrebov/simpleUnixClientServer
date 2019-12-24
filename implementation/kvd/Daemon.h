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

class Daemon
{
public:
          Daemon();
         ~Daemon();
          Daemon( const Daemon& ) = delete;
          Daemon& operator=( const Daemon& ) = delete;
   bool   Daemonise();

private:
   bool   isDaemon;
};