
// Copyright(C) Jukka Kokkonen, 2002


#include "../../container/LinkedList.h"
#include "DummyDriver.h"
#include "DummyBind.h"
#include "DummyConnection.h"


namespace net
{
  namespace dummy
  {
		int DummyDriver::lagVariationSpeed = 0;
		int DummyDriver::lagMinPing = 0;
		int DummyDriver::lagMaxPing = 0;

    DummyDriver::DummyDriver()
    {
      modeFlags = NET_NETMODE_FLAG_NONBLOCKING;
    }


    DummyDriver::~DummyDriver()
    {
      // nop
    }


    const char *DummyDriver::getDriverName()
    {
      return "dummy";
    }

    
    INetConnection *DummyDriver::connectTo(const char *to)
      throw (NetDriverException*)
    {
      if (to == NULL)
      {
        throw new NetDriverException(NetDriverException::EXCEPTION_TYPE_INVALID_PARAMETER,
          "DummyDriver::connectTo - Invalid parameter (null)."); 
      }

      if (strncmp(to, "dummy:", 6) != 0)
      {
        throw new NetDriverException(NetDriverException::EXCEPTION_TYPE_INVALID_PARAMETER,
          "DummyDriver::connectTo - Invalid parameter (can only connect to dummy host)."); 
      }

      LinkedListIterator iter = LinkedListIterator(portBindList);
      while (iter.iterateAvailable())
      {
        const char *port = &to[6];
        DummyBind *tmp = (DummyBind *)iter.iterateNext();
        if (strcmp(tmp->getName(), port) == 0)
        {
          if (tmp->isAcceptingConnections())
          {
            DummyConnection *serverconn = new DummyConnection(NULL);
            DummyConnection *conn = new DummyConnection(serverconn);
            tmp->addAcceptedConnection(serverconn);
            return conn;
          }
        }
      }
      throw new NetDriverException(NetDriverException::EXCEPTION_TYPE_CONNECTION_REFUSED,
        "DummyDriver::connectTo - Connection refused (port is not being listened)."); 
    }
   

    INetBind *DummyDriver::bind(const char *port)
      throw (NetDriverException*)
    {
      if (port == NULL)
      {
        throw new NetDriverException(NetDriverException::EXCEPTION_TYPE_INVALID_PARAMETER,
          "DummyDriver::bind - Invalid argument (null port name).");
      }
      /*
      if (strncmp(port, "dummy:", 6) != 0 || strlen(port) < 7)
      {
        throw new NetDriverException(NetDriverException::EXCEPTION_TYPE_INVALID_PARAMETER,
          "DummyDriver::bind - Invalid parameter (can only bind to dummy host)."); 
      }
      */
      LinkedListIterator iter = LinkedListIterator(portBindList);
      while (iter.iterateAvailable())
      {
        DummyBind *tmp = (DummyBind *)iter.iterateNext();
        if (strcmp(tmp->getName(), port) == 0)
        {
          throw new NetDriverException(NetDriverException::EXCEPTION_TYPE_BIND_FAILED,
            "DummyDriver::bind - Could not bind to port (port name already in use).");
        }
      }
      DummyBind *dummyBind = new DummyBind(port);
      portBindList->append(dummyBind);
      return dummyBind;
    }


    void DummyDriver::setArtificialLag(int minPing, int maxPing, int variationSpeed)
    {
			DummyDriver::lagMinPing = minPing;
			DummyDriver::lagMaxPing = maxPing;
			DummyDriver::lagVariationSpeed = variationSpeed;
    }


    void DummyDriver::setNetModeFlags(int netModeFlags)
      throw (NetDriverException*)
    {
      if (netModeFlags != NET_NETMODE_FLAG_NONBLOCKING)
        throw new NetDriverException(NetDriverException::EXCEPTION_TYPE_UNSUPPORTED_PARAMETER,
          "DummyDriver::setNetModeFlags - Unsupported flags set.");
    }
 

    int DummyDriver::getNetModeFlags()
    {
      return modeFlags;
    }


    int DummyDriver::getSupportedModeFlags()
    {
      // none supported, that is, none of them may be chaged.
      // yet, some of the flags may be initially set.
      return 0;
    }

   
    LinkedList *DummyDriver::portBindList = new LinkedList();
  }
}
