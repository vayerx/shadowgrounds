
// Copyright(C) Jukka Kokkonen, 2002


#include "TCPIPDriver.h"
#include "TCPIPBind.h"
#include "TCPIPConnection.h"
#include "Connection.h"

#include "../dummy/DummyDriver.h"


namespace net
{
  namespace tcpip
  {
		int TCPIPDriver::lagVariationSpeed = 0;
		int TCPIPDriver::lagMinPing = 0;
		int TCPIPDriver::lagMaxPing = 0;


    TCPIPDriver::TCPIPDriver()
    {
      modeFlags = NET_NETMODE_FLAG_NONBLOCKING;
    }


    TCPIPDriver::~TCPIPDriver()
    {
      // nop
    }


    const char *TCPIPDriver::getDriverName()
    {
      return "tcpip";
    }

    
    INetConnection *TCPIPDriver::connectTo(const char *to)
      throw (NetDriverException*)
    {
      if (to == NULL)
      {
        throw new NetDriverException(NetDriverException::EXCEPTION_TYPE_INVALID_PARAMETER,
          "TCPIPDriver::connectTo - Invalid parameter (null)."); 
      }

      int slen = strlen(to);
      int sep = -1;
      for (int i = 0; i < slen; i++)
      {
        if (to[i] == ':')
        {
          sep = i;
          break;
        }
      }
      if (sep == -1)
      {
        throw new NetDriverException(NetDriverException::EXCEPTION_TYPE_INVALID_PARAMETER,
          "TCPIPDriver::connectTo - Invalid parameter (expected \"hostname:port\")."); 
      }
      char *hostname = new char[slen + 1];
      strncpy(hostname, to, sep);
      hostname[sep] = '\0';
      int port = atoi(&to[sep + 1]);

      Connection *c = new Connection();
      int connOk = c->connect(hostname, (short)port);

      delete [] hostname;

      if (connOk != 0)
      {
        throw new NetDriverException(NetDriverException::EXCEPTION_TYPE_CONNECTION_REFUSED,
          "TCPIPDriver::connectTo - Connection refused."); 
      } else {
        c->setNonBlocking(true);
      }
      return new TCPIPConnection(c);
    }
   

    INetBind *TCPIPDriver::bind(const char *port)
      throw (NetDriverException*)
    {
      if (port == NULL)
      {
        throw new NetDriverException(NetDriverException::EXCEPTION_TYPE_INVALID_PARAMETER,
          "TCPIPDriver::bind - Invalid argument (null port name).");
      }
      TCPIPBind *tcpipBind = new TCPIPBind(port);
      if (tcpipBind->c == NULL)
      {
        delete tcpipBind;
        throw new NetDriverException(NetDriverException::EXCEPTION_TYPE_BIND_FAILED,
          "TCPIPDriver::bind - Bind failed.");
      }
      return tcpipBind;
    }

      
    void TCPIPDriver::setNetModeFlags(int netModeFlags)
      throw (NetDriverException*)
    {
      if (netModeFlags != NET_NETMODE_FLAG_NONBLOCKING)
        throw new NetDriverException(NetDriverException::EXCEPTION_TYPE_UNSUPPORTED_PARAMETER,
          "TCPIPDriver::setNetModeFlags - Unsupported flags set.");
    }
 

    int TCPIPDriver::getNetModeFlags()
    {
      return modeFlags;
    }


    int TCPIPDriver::getSupportedModeFlags()
    {
      // none supported, that is, none of them may be chaged.
      // yet, some of the flags may be initially set.
      return 0;
    }

    void TCPIPDriver::setArtificialLag(int minPing, int maxPing, int variationSpeed)
		{
			this->lagMinPing = minPing;
			this->lagMaxPing = maxPing;
			this->lagVariationSpeed = variationSpeed;

			// HACK: use dummy driver to simulate lag...
			net::dummy::DummyDriver::setArtificialLag(minPing, maxPing, variationSpeed);
		}

  }
}
