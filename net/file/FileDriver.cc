
// Copyright(C) Jukka Kokkonen, 2007


#include "../../container/LinkedList.h"
#include "FileDriver.h"
#include "FileBind.h"
#include "FileConnection.h"

#include <stdio.h>


namespace net
{
  namespace file
  {
    FileDriver::FileDriver()
    {
      modeFlags = NET_NETMODE_FLAG_NONBLOCKING;
    }


    FileDriver::~FileDriver()
    {
      // nop
    }


    const char *FileDriver::getDriverName()
    {
      return "file";
    }

    
    INetConnection *FileDriver::connectTo(const char *to)
      throw (NetDriverException*)
    {
      if (to == NULL)
      {
        throw new NetDriverException(NetDriverException::EXCEPTION_TYPE_INVALID_PARAMETER,
          "FileDriver::connectTo - Invalid parameter (null)."); 
      }

      if (strncmp(to, "file:", 5) != 0)
      {
        throw new NetDriverException(NetDriverException::EXCEPTION_TYPE_INVALID_PARAMETER,
          "FileDriver::connectTo - Invalid parameter (can only connect to file host)."); 
      }

      const char *port = &to[6];

			FILE *f = fopen(port, "rb+");
			if (f != NULL)
			{
				fclose(f);
        FileConnection *conn = new FileConnection(port);
        return conn;
      }

      throw new NetDriverException(NetDriverException::EXCEPTION_TYPE_CONNECTION_REFUSED,
        "FileDriver::connectTo - Connection refused (port is not being listened)."); 
    }
   

    INetBind *FileDriver::bind(const char *port)
      throw (NetDriverException*)
    {
      if (port == NULL)
      {
        throw new NetDriverException(NetDriverException::EXCEPTION_TYPE_INVALID_PARAMETER,
          "FileDriver::bind - Invalid argument (null port name).");
      }
      /*
      if (strncmp(port, "file:", 5) != 0 || strlen(port) < 6)
      {
        throw new NetDriverException(NetDriverException::EXCEPTION_TYPE_INVALID_PARAMETER,
          "FileDriver::bind - Invalid parameter (can only bind to file host)."); 
      }
      */
			FILE *f = fopen(port, "wb+");
			if (f != NULL)
      {
        throw new NetDriverException(NetDriverException::EXCEPTION_TYPE_BIND_FAILED,
          "FileDriver::bind - Could not bind to port (port name already in use).");
      }
      FileBind *fileBind = new FileBind(port);
      return fileBind;
    }


    void FileDriver::setNetModeFlags(int netModeFlags)
      throw (NetDriverException*)
    {
      if (netModeFlags != NET_NETMODE_FLAG_NONBLOCKING)
        throw new NetDriverException(NetDriverException::EXCEPTION_TYPE_UNSUPPORTED_PARAMETER,
          "FileDriver::setNetModeFlags - Unsupported flags set.");
    }
 

    int FileDriver::getNetModeFlags()
    {
      return modeFlags;
    }


    int FileDriver::getSupportedModeFlags()
    {
      // none supported, that is, none of them may be chaged.
      // yet, some of the flags may be initially set.
      return 0;
    }

  }
}
