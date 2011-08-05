#ifndef NET_FILEBIND_H
#define NET_FILEBIND_H

// Copyright(C) Jukka Kokkonen, 2007

#include "../../container/LinkedList.h"
#include "../INetBind.h"
#include "FileConnection.h"

namespace net
{
  namespace file
  {
    // proto
    class FileDriver;


    /**
     * File net bind interface.
     */
    class FileBind : public INetBind
    {
    public:
      FileBind(const char *name);

      virtual INetConnection *acceptConnection() 
        throw (NetDriverException*);

      virtual void listen(int backlog) 
        throw (NetDriverException*);

      virtual int getMaxBacklog();

      virtual void unbind() 
        throw (NetDriverException*);
     
      virtual ~FileBind();

      const char *getName();

    private:
      char *name;
      int acceptAmount;

      LinkedList *acceptedConnections;

      void addAcceptedConnection(FileConnection *connection);

      bool isAcceptingConnections();

      friend FileDriver;
    };
  }
}

#endif

