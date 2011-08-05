
#ifndef NET_INETCONNECTION_H
#define NET_INETCONNECTION_H

// Copyright(C) Jukka Kokkonen, 2002

#include "NetDriverException.h"

#ifdef _MSC_VER
#pragma warning ( disable : 4290 ) 
#endif

namespace net
{

  /**
   * Net connection interface.
   */
  class INetConnection
  {
  public:
    virtual void flush() 
      throw (NetDriverException*) = 0;

    virtual int send(const void *buf, int maxlen) 
      throw (NetDriverException*) = 0;

    virtual int receive(void *buf, int maxlen) 
      throw (NetDriverException*) = 0;

    virtual void close() 
      throw (NetDriverException*) = 0;

    virtual ~INetConnection() { };
  };
}

#endif

