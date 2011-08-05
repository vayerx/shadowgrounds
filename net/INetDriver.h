
#ifndef NET_INETDRIVER_H
#define NET_INETDRIVER_H

// Copyright(C) Jukka Kokkonen, 2002

#include "netflags.h"
#include "NetDriverException.h"
#include "INetConnection.h"
#include "INetBind.h"

namespace net
{

  /**
   * Network driver interface.
   */
  class INetDriver
  {
  public:
    virtual INetConnection *connectTo(char *to)
      throw (NetDriverException*) = 0;

    virtual INetBind *bind(char *port)
      throw (NetDriverException*) = 0;

    virtual void setNetModeFlags(int netModeFlags)
      throw (NetDriverException*) = 0;

    virtual int getNetModeFlags() = 0;

    virtual int getSupportedModeFlags() = 0;

    virtual const char *getDriverName() = 0;

    virtual ~INetDriver() { };
  };
}


#endif

