
// Copyright(C) Jukka Kokkonen, 2002

#include "DummyBind.h"
#include "DummyDriver.h"

#define DUMMYBIND_MAX_BACKLOG 1024


namespace net
{
  namespace dummy
  {

    DummyBind::DummyBind(const char *name)
    {
      int slen = strlen(name);
      this->name = new char[slen + 1];
      strcpy(this->name, name);
      acceptedConnections = new LinkedList();
      acceptAmount = -1;
    }


    DummyBind::~DummyBind()
    {
      delete [] name;
      delete acceptedConnections;
    }


    INetConnection *DummyBind::acceptConnection()
      throw (NetDriverException*)
    {
      if (acceptAmount == -1)
      {
         throw new NetDriverException(NetDriverException::EXCEPTION_TYPE_INVALID_STATE,
          "DummyBind::acceptConnection - Cannot accept, not listening.");
      }
      if (!acceptedConnections->isEmpty())
      {
        acceptAmount++;
        DummyConnection *conn = (DummyConnection *)acceptedConnections->popFirst();
        return conn;
      } else {
        return NULL;
      }
    }


    void DummyBind::listen(int backlog)
      throw (NetDriverException*)
    {
      if (acceptAmount != -1)
      {
        throw new NetDriverException(NetDriverException::EXCEPTION_TYPE_INVALID_STATE,
          "DummyBind::listen - Already listening.");
      }
      acceptAmount = backlog;
    }


    int DummyBind::getMaxBacklog()
    {
      return DUMMYBIND_MAX_BACKLOG;
    }

 
    void DummyBind::unbind()
      throw (NetDriverException*)
    {
      // TODO: should delete the connections in accept queue and disconnect
      // their peer connections.
      DummyDriver::portBindList->remove(this); 
      acceptAmount = -1;
    }


    void DummyBind::addAcceptedConnection(DummyConnection *connection)
    {
      if (acceptAmount != -1)
      {
        acceptedConnections->append(connection); 
        acceptAmount--;
      }
      // else assert(0)
    }


    bool DummyBind::isAcceptingConnections()
    {
      if (acceptAmount > 0) 
        return true;
      else 
        return false;
    }


    const char *DummyBind::getName()
    {
      return name;
    }


  }
}
