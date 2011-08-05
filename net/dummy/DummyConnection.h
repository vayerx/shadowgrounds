#ifndef NET_DUMMYCONNECTION_H
#define NET_DUMMYCONNECTION_H

// Copyright(C) Jukka Kokkonen, 2002

#include "../INetConnection.h"

#define DUMMYCONNECTION_LAG_RAND_AMOUNT 20

namespace net
{
  namespace dummy
  {
    /**
     * Dummy net connection.
     */
    class DummyConnection : public INetConnection
    {
    public:
      DummyConnection(DummyConnection *peer); 

      virtual ~DummyConnection();
 
      virtual void flush() 
        throw (NetDriverException*);

      virtual int send(const void *buf, int maxlen) 
        throw (NetDriverException*);

      virtual int receive(void *buf, int maxlen) 
        throw (NetDriverException*);

      virtual void close() 
        throw (NetDriverException*);

    private:
      DummyConnection *connectedTo;
      char *recvBuf;
      int recvBufUsed;
			int recvBufPos;

      short *recvBufTiming;

			int currentLag;
			int currentTime;

			int createArtificialLag(int receiveBytes);

			static int lagRand[DUMMYCONNECTION_LAG_RAND_AMOUNT];

    };
  }
}

#endif

