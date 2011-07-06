
// Copyright(C) Jukka Kokkonen, 2007

#include "FileBind.h"
#include "FileDriver.h"

#define FILEBIND_MAX_BACKLOG 1024


namespace net
{
  namespace file
  {

    FileBind::FileBind(const char *name)
    {
      int slen = strlen(name);
      this->name = new char[slen + 1];
      strcpy(this->name, name);
      acceptedConnections = new LinkedList();
      acceptAmount = -1;
    }


    FileBind::~FileBind()
    {
      delete [] name;
      delete acceptedConnections;
    }


    INetConnection *FileBind::acceptConnection()
      throw (NetDriverException*)
    {
      if (acceptAmount == -1)
      {
         throw new NetDriverException(NetDriverException::EXCEPTION_TYPE_INVALID_STATE,
          "FileBind::acceptConnection - Cannot accept, not listening.");
      }
      if (!acceptedConnections->isEmpty())
      {
        acceptAmount++;
        FileConnection *conn = (FileConnection *)acceptedConnections->popFirst();
        return conn;
      } else {
        return NULL;
      }
    }


    void FileBind::listen(int backlog)
      throw (NetDriverException*)
    {
      if (acceptAmount != -1)
      {
        throw new NetDriverException(NetDriverException::EXCEPTION_TYPE_INVALID_STATE,
          "FileBind::listen - Already listening.");
      }
      acceptAmount = backlog;
    }


    int FileBind::getMaxBacklog()
    {
      return FILEBIND_MAX_BACKLOG;
    }

 
    void FileBind::unbind()
      throw (NetDriverException*)
    {
      // TODO: should delete the connections in accept queue and disconnect
      // their peer connections.
      acceptAmount = -1;
    }


    void FileBind::addAcceptedConnection(FileConnection *connection)
    {
      if (acceptAmount != -1)
      {
        acceptedConnections->append(connection); 
        acceptAmount--;
      }
      // else assert(0)
    }


    bool FileBind::isAcceptingConnections()
    {
      if (acceptAmount > 0) 
        return true;
      else 
        return false;
    }


    const char *FileBind::getName()
    {
      return name;
    }


  }
}
