
#ifndef NET_INETBIND_H
#define NET_INETBIND_H

// Copyright(C) Jukka Kokkonen, 2002

#include "NetDriverException.h"
#include "INetConnection.h"

namespace net
{

  /**
   * Net bind interface.
   */
  class INetBind
  {
  public:
    virtual INetConnection *acceptConnection()
      throw (NetDriverException*) = 0;

    virtual void listen(int backlog) 
      throw (NetDriverException*) = 0;

    virtual int getMaxBacklog() = 0;

    virtual void unbind()
      throw (NetDriverException*) = 0;
     
    virtual ~INetBind() { };
  };
}

#endif

