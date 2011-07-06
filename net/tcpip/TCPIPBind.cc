
// Copyright(C) Jukka Kokkonen, 2002

#include "TCPIPBind.h"
#include "TCPIPConnection.h"
#include "Connection.h"

#define TCPIPBIND_MAX_BACKLOG 32

namespace net
{
  namespace tcpip
  {

    TCPIPBind::TCPIPBind(const char *name)
    {
      int port = atoi(name);
      c = new Connection();
      if (c->bind((short)port) != 0)
      {
        delete c;
        c = NULL;
      }
      nc = new Connection();
    }


    TCPIPBind::~TCPIPBind()
    {
      if (nc != NULL)
      {
        delete nc;
      }
      if (c != NULL)
      {
        c->unbind();
        delete c;
      } 
    }


    INetConnection *TCPIPBind::acceptConnection()
      throw (NetDriverException*)
    {
      Connection *retc = NULL;
      int result = c->accept(nc);
      if (result == 0)
      {
        retc = nc;
        retc->setNonBlocking(true);
        nc = new Connection();
      } else {
        if (result != NONBLOCKING_EAGAIN)
        {
          throw new NetDriverException(NetDriverException::EXCEPTION_TYPE_UNDEFINED,
            "TCPIPBind::accept - Accept failed.");
        }
      }
      if (retc != NULL)
        return new TCPIPConnection(retc);
      else
        return NULL;
    }


    void TCPIPBind::listen(int backlog)
      throw (NetDriverException*)
    {
      if (c->listen(backlog) != 0)
      {
        throw new NetDriverException(NetDriverException::EXCEPTION_TYPE_UNDEFINED,
          "TCPIPBind::listen - Listen failed.");
      } else {
        c->setNonBlocking(true);
      }
    }


    int TCPIPBind::getMaxBacklog()
    {
      return TCPIPBIND_MAX_BACKLOG;
    }

 
    void TCPIPBind::unbind()
      throw (NetDriverException*)
    {
      c->unbind();
    }
  }
}
