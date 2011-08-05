#ifndef NET_FILE_FILEDRIVER_H
#define NET_FILE_FILEDRIVER_H

// Copyright(C) Jukka Kokkonen, 2002

#include "../INetDriver.h"
#include "../../container/LinkedList.h"

namespace net
{
  namespace file
  {
    // proto
    class FileBind;
    class FileConnection;

    /**
     * A file net driver.
     */
    class FileDriver : public INetDriver
    {
    public:
      FileDriver();

      virtual const char *getDriverName();

      virtual INetConnection *connectTo(const char *to)
        throw (NetDriverException*);
   
      virtual INetBind *bind(const char *port)
        throw (NetDriverException*);
   
      virtual void setNetModeFlags(int netModeFlags)
        throw (NetDriverException*);

      virtual int getNetModeFlags();
      
      virtual int getSupportedModeFlags();
   
      virtual ~FileDriver();
  
    private:
      int modeFlags;

      friend class FileBind;
      friend class FileConnection;
    };
  }
}

#endif
