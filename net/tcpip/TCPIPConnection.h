#ifndef NET_TCPIPCONNECTION_H
#define NET_TCPIPCONNECTION_H

// Copyright(C) Jukka Kokkonen, 2002

#include "../INetConnection.h"

class Connection;

namespace net
{
  namespace tcpip
  {
		class TCPIPConnectionImpl;

    /**
     * TCP/IP net connection.
     */
    class TCPIPConnection : public INetConnection
    {
    public:
      TCPIPConnection(Connection *c); 

      virtual ~TCPIPConnection();
 
      virtual void flush() 
        throw (NetDriverException*);

      virtual int send(const void *buf, int maxlen) 
        throw (NetDriverException*);

      virtual int receive(void *buf, int maxlen) 
        throw (NetDriverException*);

      virtual void close() 
        throw (NetDriverException*);

    private:
      Connection *c;
			TCPIPConnectionImpl *impl;
    };
  }
}

#endif

