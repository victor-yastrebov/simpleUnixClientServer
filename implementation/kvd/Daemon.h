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

// daemon implementation via:
// https://stackoverflow.com/questions/17954432/creating-a-daemon-in-linux/17955149#17955149

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
