#ifndef NET_TCPIP_TCPIPDRIVER_H
#define NET_TCPIP_TCPIPDRIVER_H

// Copyright(C) Jukka Kokkonen, 2002

#include "../INetDriver.h"

class Connection;

namespace net
{
  namespace tcpip
  {
    // proto
    class TCPIPBind;
    class TCPIPConnection;

    /**
     * A TCP/IP net driver.
     */
    class TCPIPDriver : public INetDriver
    {
    public:
      TCPIPDriver();

      virtual const char *getDriverName();

      virtual INetConnection *connectTo(const char *to)
        throw (NetDriverException*);
   
      virtual INetBind *bind(const char *port)
        throw (NetDriverException*);
   
      virtual void setNetModeFlags(int netModeFlags)
        throw (NetDriverException*);

      virtual int getNetModeFlags();
      
      virtual int getSupportedModeFlags();
   
			void setArtificialLag(int minPing, int maxPing, int variationSpeed);
			
      virtual ~TCPIPDriver();
  
    private:
      int modeFlags;

			static int lagVariationSpeed;
			static int lagMinPing;
			static int lagMaxPing;

			//friend class TCPIPBind;
      friend class TCPIPConnection;
    };
  }
}

#endif
