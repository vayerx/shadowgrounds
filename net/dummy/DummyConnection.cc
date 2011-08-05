
// Copyright(C) Jukka Kokkonen, 2002

#include "DummyConnection.h"

#include "DummyBind.h"
#include "DummyDriver.h"
#include "../../system/Timer.h"
#include <assert.h>

// TEMP: for debugging
#include <stdio.h>

#define DUMMYCONNECTION_BUF_SIZE (65536 * 4) * 2



namespace net
{
  namespace dummy
  {

		int DummyConnection::lagRand[DUMMYCONNECTION_LAG_RAND_AMOUNT] =
		{ 0, 30, 60, 100, 100, -20, -20, 100, 0, 40, 
			0, -30, -60, -100, -100, 20, 20, -100, 0, -40, };

    DummyConnection::DummyConnection(DummyConnection *peer)
    {
      this->connectedTo = peer;
      if (peer != NULL)
      {
        peer->connectedTo = this;
      }
      recvBuf = new char[DUMMYCONNECTION_BUF_SIZE];
      recvBufUsed = 0;
      recvBufTiming = new short[DUMMYCONNECTION_BUF_SIZE];
			recvBufPos = 0;
			currentLag = DummyDriver::lagMinPing;
			Timer::update();
			currentTime = Timer::getTime();
    }


    DummyConnection::~DummyConnection()
    {
      delete [] recvBuf;
      if (connectedTo != NULL)
      {
        connectedTo->connectedTo = NULL;
      }
    }


    void DummyConnection::flush() 
      throw (NetDriverException*)
    {
      // this is problematic... cannot flush really.
      // cos the same process is also the one receiving the data.
      // thus, flush would have to wait forever for the data being received.
      return;
    }
 

    int DummyConnection::send(const void *buf, int maxlen) 
      throw (NetDriverException*)
    {
      if (connectedTo == NULL)
      {
        throw new NetDriverException(NetDriverException::EXCEPTION_TYPE_INVALID_STATE,
          "DummyConnection::send - Cannot send, connection closed.");
      }
      if (connectedTo->recvBufUsed >= DUMMYCONNECTION_BUF_SIZE)
      {
        // TODO: might want the exception to be optional...
        // (use some ifdef to do that... maybe _DEBUG?)
        throw new NetDriverException(NetDriverException::EXCEPTION_TYPE_BUFFER_FULL,
          "DummyConnection::send - Buffer full.");
        return 0;
      }
      int used = 0;
      if (connectedTo->recvBufUsed + maxlen > DUMMYCONNECTION_BUF_SIZE)
      {
        used = DUMMYCONNECTION_BUF_SIZE - connectedTo->recvBufUsed;
      } else {
        used = maxlen;
      }
      memcpy(&connectedTo->recvBuf[connectedTo->recvBufUsed], buf, used);
			// create timing info (for artificial lag)...
			Timer::update();
			short currentTime = (short)(Timer::getTime() % 10000); 
			for (int i = 0; i < used; i++)
			{
				connectedTo->recvBufTiming[connectedTo->recvBufUsed + i] = currentTime;
			}
      connectedTo->recvBufUsed += used;
      return used;      
    }
    

    int DummyConnection::receive(void *buf, int maxlen) 
      throw (NetDriverException*)
    {
			assert(maxlen != 0);

      int got = 0;
      if (recvBufUsed == 0)
      {
				if (connectedTo == NULL)
				{
					throw new NetDriverException(NetDriverException::EXCEPTION_TYPE_INVALID_STATE,
						"DummyConnection::receive - Cannot receive, connection closed.");
				}
        return 0;
      }
      if (maxlen > recvBufUsed - recvBufPos)
      {
        got = recvBufUsed - recvBufPos;
      } else {
        got = maxlen;
      }

			got = createArtificialLag(got);

//printf("net buf %d, got %d\n", recvBufUsed - got, got);
			// FIXME: starts to eat performance pretty bad if buffer gets
			// big enough

      memcpy(buf, &recvBuf[recvBufPos], got);
			if (recvBufPos + got > DUMMYCONNECTION_BUF_SIZE / 2)
			{
				for (int i = recvBufPos + got; i < recvBufUsed; i++)
				{
					recvBuf[i - (recvBufPos + got)] = recvBuf[i];
					recvBufTiming[i - (recvBufPos + got)] = recvBufTiming[i];
				}
				recvBufUsed -= (recvBufPos + got);
				recvBufPos = 0;
			} else {
				recvBufPos += got;
			}
      return got;
    }


    void DummyConnection::close() 
      throw (NetDriverException*)
    {
			if (connectedTo != NULL)
        connectedTo->connectedTo = NULL;
      connectedTo = NULL;      
    }


		int DummyConnection::createArtificialLag(int receiveBytes)
		{
			if (DummyDriver::lagMaxPing == 0)
			{
				return receiveBytes;
			}

			// artificial lag
			
			// FIXME: may possibly break 
			// (and loop almost forever) if timer overflows

			Timer::update();
			while (currentTime < Timer::getTime() - 1000)
			{
				currentTime += 1000;
				currentLag += lagRand[(currentTime / 1000 / 3) % DUMMYCONNECTION_LAG_RAND_AMOUNT] * DummyDriver::lagVariationSpeed / 100;
				if (currentLag > DummyDriver::lagMaxPing)
				{
					currentLag = DummyDriver::lagMaxPing;
				}
				if (currentLag < DummyDriver::lagMinPing)
				{
					currentLag = DummyDriver::lagMinPing;
				}
			}

			// FIXME: may not be able to handle lags way over 10 secs.
			// or actually 5 sec...
			// FIXME: may not be able to handle negative timer values.

			Timer::update();
			short currentLaggedTime = (short)((Timer::getTime() - currentLag) % 10000); 

			int timedReceive = receiveBytes;
      for (int i = 0; i < receiveBytes; i++)
      {
        if (recvBufTiming[recvBufPos + i] > currentLaggedTime
					&& recvBufTiming[recvBufPos + i] < currentLaggedTime + 5000)
				{
//printf("lagged = %d, buf = %d\n", currentLaggedTime, recvBufTiming[i]);
					timedReceive = i;
					break;
				}
      }

			return timedReceive;

		}

  }
}
