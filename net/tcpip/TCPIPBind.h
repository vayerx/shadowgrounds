#ifndef NET_TCPIPBIND_H
#define NET_TCPIPBIND_H

// Copyright(C) Jukka Kokkonen, 2002

#include "../INetBind.h"

class Connection;

namespace net
{
  namespace tcpip
  {
    // proto
    class TCPIPDriver;


    /**
     * TCP/IP net bind interface.
     */
    class TCPIPBind : public INetBind
    {
    public:
      TCPIPBind(const char *name);

      virtual INetConnection *acceptConnection() 
        throw (NetDriverException*);

      virtual void listen(int backlog) 
        throw (NetDriverException*);

      virtual int getMaxBacklog();

      virtual void unbind() 
        throw (NetDriverException*);
     
      virtual ~TCPIPBind();

    private:
      Connection *c;
      Connection *nc;

      friend TCPIPDriver;
    };
  }
}

#endif

