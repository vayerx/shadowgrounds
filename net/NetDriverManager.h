
#ifndef NET_NETDRIVERMANAGER_H
#define NET_NETDRIVERMANAGER_H

#include "../container/LinkedList.h"
#include "INetDriver.h"

namespace net
{
  class NetDriverManager
  {
  public:
    static void registerDriver(INetDriver *driver);

    static INetDriver *getDriver(char *name);

  private:
    static LinkedList *driverList;
  };
}

#endif

