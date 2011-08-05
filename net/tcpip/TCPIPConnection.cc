
// Copyright(C) Jukka Kokkonen, 2002

#include "TCPIPConnection.h"
#include "Connection.h"

// HACK: for the dummy lag hack
#include "../../system/Timer.h"
#include "TCPIPDriver.h"
#include "../dummy/DummyConnection.h"
#include "../dummy/DummyDriver.h"

#include <stdio.h>


namespace net
{
  namespace tcpip
  {
		// HACK: internal impl class to allow artificial lag and internal buffering in TCP/IP connections.
    class TCPIPConnectionImpl
		{
			public:
				TCPIPConnectionImpl()
				{
					sendBufIn = new net::dummy::DummyConnection(NULL);
					sendBufOut = new net::dummy::DummyConnection(sendBufIn);
				}

				net::dummy::DummyConnection *sendBufIn;
				net::dummy::DummyConnection *sendBufOut;
		};


    TCPIPConnection::TCPIPConnection(Connection *c)
    {
      this->c = c;
			if (net::tcpip::TCPIPDriver::lagMinPing > 0)
			{
				this->impl = new TCPIPConnectionImpl();
			} else {
				this->impl = NULL;
			}
    }


    TCPIPConnection::~TCPIPConnection()
    {
			if (this->impl != NULL)
				delete this->impl;

      if (c != NULL)
        delete c;
    }


static char footmp[1024+1];

    void TCPIPConnection::flush() 
      throw (NetDriverException*)
    {
      // nop (TODO?)
			if (this->impl != NULL)
			{
				int gotdummy = impl->sendBufOut->receive(footmp, 1024);
				c->send((char *)footmp, gotdummy);
			}

      return;
    }
 

    int TCPIPConnection::send(const void *buf, int maxlen) 
      throw (NetDriverException*)
    {
			int got = 0;

			if (this->impl != NULL)
			{
				got = impl->sendBufIn->send(buf, maxlen);

static int foo = 0;
static int lastFoo = 0;
Timer::update();
foo += (Timer::getTime() - lastFoo);
//foo++;

				if (foo > 50)
				{
					lastFoo = Timer::getTime();
					foo = 0;
					int gotdummy = impl->sendBufOut->receive(footmp, 1024);

					printf("%d,", gotdummy);

					got = c->send((char *)footmp, gotdummy);
				}
			} else {
	      got = c->send((char *)buf, maxlen);
			}

      if (got < 0)
      {
        if (got == NONBLOCKING_EAGAIN)
          return 0;
        throw new NetDriverException(NetDriverException::EXCEPTION_TYPE_SEND_FAILED,
          "TCPIPConnection::send - Send failed.");
      }
      return got;
    }
    

    int TCPIPConnection::receive(void *buf, int maxlen) 
      throw (NetDriverException*)
    {
      int got = c->recv((char *)buf, maxlen);
      if (got < 0) 
      {
        if (got == NONBLOCKING_EAGAIN)
          return 0;
        throw new NetDriverException(NetDriverException::EXCEPTION_TYPE_RECEIVE_FAILED,
          "TCPIPConnection::receive - Receive failed.");
      }
      return got;
    }


    void TCPIPConnection::close() 
      throw (NetDriverException*)
    {
      c->close();
    }

  }
}
