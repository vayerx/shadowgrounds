#ifndef NET_DUMMY_DUMMYDRIVER_H
#define NET_DUMMY_DUMMYDRIVER_H

// Copyright(C) Jukka Kokkonen, 2002

#include "../INetDriver.h"
#include "../../container/LinkedList.h"

namespace net
{
  namespace dummy
  {
    // proto
    class DummyBind;
    class DummyConnection;

    /**
     * A dummy net driver.
     */
    class DummyDriver : public INetDriver
    {
    public:
      DummyDriver();

      virtual const char *getDriverName();

      virtual INetConnection *connectTo(const char *to)
        throw (NetDriverException*);
   
      virtual INetBind *bind(const char *port)
        throw (NetDriverException*);
   
      virtual void setNetModeFlags(int netModeFlags)
        throw (NetDriverException*);

      virtual int getNetModeFlags();
      
      virtual int getSupportedModeFlags();
   
      virtual ~DummyDriver();
  
	    static void setArtificialLag(int minPing, int maxPing, int variationSpeed);

    private:
      static LinkedList *portBindList;
      int modeFlags;

			static int lagVariationSpeed;
			static int lagMinPing;
			static int lagMaxPing;

      friend class DummyBind;
      friend class DummyConnection;
    };
  }
}

#endif
