#include <lofar_config.h>
#include "NetFuncs.h"

#include <Common/Thread/Mutex.h>
#include <Common/LofarLogger.h>
#include <Common/SystemCallException.h>

#include <cstring>
#include <cstdio>
#include <netdb.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <boost/format.hpp>
using boost::format;

namespace {
  LOFAR::Mutex getAddrInfoMutex;
}

namespace LOFAR {

  safeAddrInfo::safeAddrInfo()
  :
    addrinfo(0)
  {
  }

  safeAddrInfo::~safeAddrInfo() {
    if(addrinfo) freeaddrinfo(addrinfo);
  }

  void safeGetAddrInfo(safeAddrInfo &result, bool TCP, const std::string &hostname, uint16 port) {
    struct addrinfo hints;
    char portStr[16];

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_flags = AI_NUMERICSERV; // we only use numeric port numbers, not strings like "smtp"

    if (TCP) {
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_protocol = IPPROTO_TCP;
    } else {
      hints.ai_socktype = SOCK_DGRAM;
      hints.ai_protocol = IPPROTO_UDP;
    }

    snprintf(portStr, sizeof portStr, "%hu", port);

    {
      // getaddrinfo does not seem to be thread safe
      ScopedLock sl(getAddrInfoMutex);

      int retval;

      if ((retval = getaddrinfo(hostname.c_str(), portStr, &hints, &result.addrinfo)) != 0) {
        const string errorstr = gai_strerror(retval);

        throw SystemCallException(str(format("getaddrinfo(%s): %s") % hostname % errorstr), 0, THROW_ARGS); // TODO: SystemCallException also adds strerror(0), which is useless here
      }
    }
  }

  struct sockaddr getInterfaceIP(const std::string &iface) {
    int fd = -1;
    struct ifreq ifr;

    if ((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
      THROW_SYSCALL("socket");

    try {
      memset(&ifr, 0, sizeof ifr);
      snprintf(ifr.ifr_name, sizeof ifr.ifr_name, "%s", iface.c_str());

      if (ioctl(fd, SIOCGIFADDR, &ifr) < 0)
        THROW_SYSCALL("ioctl");
    } catch(...) {
      close(fd);
      throw;
    }

    close(fd);
    return ifr.ifr_addr;
  }

  int getSocketPort(int fd) {
    struct sockaddr_in sin;
    socklen_t addrlen = sizeof sin;

    if (getsockname(fd, (struct sockaddr *)&sin, &addrlen) < 0)
      THROW_SYSCALL("getsockname");

    if (sin.sin_family != AF_INET)
      return -1;

    if (addrlen != sizeof sin)
      return -1;

    return ntohs(sin.sin_port);
  }
}
