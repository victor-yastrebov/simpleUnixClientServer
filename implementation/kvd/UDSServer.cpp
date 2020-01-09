/**
 *
 * @author <va.yastrebov>
 * @description
 * <pre>
 *    Class is responsible for processing incoming connections from clients
  * </pre>
 *
 * @class UDSServer
 */

#include<sys/un.h>
#include<syslog.h>
#include<netinet/in.h>
#include<signal.h>
#include<unistd.h>
#include<fcntl.h>

#include<algorithm>
#include<set>
#include<iostream>
#include<sstream>

// #include"UDS.h"
#include"UDSServer.h"

#include<cstdio>
#include<array>
#include<memory>
