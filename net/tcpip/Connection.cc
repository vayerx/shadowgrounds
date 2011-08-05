
#include "Connection.h"

#ifdef _MSC_VER
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <stdio.h> 
#include <stdlib.h>
#include <sys/types.h>
#ifndef _MSC_VER
#include <sys/socket.h>
#include <netinet/in.h> 
#include <netdb.h>
#endif
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#ifdef SGI_CONNECTION
#include <strings.h>
#endif

#ifdef _MSC_VER
bool winsock_inited = false;
void init_winsock()
{
  if (!winsock_inited)
  {
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
 
    wVersionRequested = MAKEWORD( 2, 2 );
 
    err = WSAStartup(wVersionRequested, &wsaData);

    // TODO err handling

    winsock_inited = true;
  }
}

void uninit_winsock()
{
  if (winsock_inited)
  {
    WSACleanup();
    winsock_inited = false;
  }
}
#endif


Connection::Connection()
{
#ifdef SGI_CONNECTION
  memset((char *)&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  //addr.sin_addr.s_addr = INADDR_ANY;
#else
#ifdef _MSC_VER
  init_winsock();
  memset((char *)&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
#else
  addr = (struct sockaddr_in) { AF_INET, 0 };
#endif
#endif  
  sock = 0;
  connected = false;
  listening = false;
  bound = false;
};

Connection::~Connection()
{
  if (connected)
  {
    this->close();
  }
  if (bound)
  {
    this->unbind();
  }
};

int Connection::connect(char *hostname, short port)
{
  struct hostent *hp;

  addr.sin_port = htons(port);

  if (hostname == NULL) return CONNECT_NULL_NAME;
  if (port == 0) return CONNECT_NULL_PORT;
        
  hp = gethostbyname(hostname);
  if (hp == NULL)  return CONNECT_UNKNOWN_HOST;

#ifdef _MSC_VER
  for (int i = 0; i < hp->h_length; i++)
    ((char*)&addr.sin_addr)[i] = hp->h_addr_list[0][i];
#else
  bcopy(hp->h_addr_list[0], (char*)&addr.sin_addr, hp->h_length);
#endif

#ifdef SGI_CONNECTION
  sock = socket(AF_INET, SOCK_STREAM, 0);
#else
  sock = socket(AF_INET, SOCK_STREAM, 0);
#endif

  if (::connect(sock, (sockaddr *) &addr, sizeof(addr)) == -1)
  { 
#ifdef _MSC_VER
    ::closesocket(sock);
#else
    ::close(sock);
#endif
    return CONNECT_CONNECTION_REFUSED; 
  }

  connected = true;
  return 0;
};

int Connection::close()
{
  if (connected)
  {
#ifdef _MSC_VER
    ::closesocket(sock);
#else
    ::close(sock);
#endif
    connected = false;
    return 0;
  } else {
    return CLOSE_NOT_CONNECTED;
  }
}

int Connection::unbind()
{
  if (bound)
  {
#ifdef _MSC_VER
    ::closesocket(sock);
#else
    ::close(sock);
#endif
    bound = false;
    return 0;
  } else {
    return UNBIND_NOT_BOUND;
  }
}

int Connection::listen(int backlog)
{
  if (bound)
  {
    ::listen(sock, backlog);
    listening = true;
    return 0;
  } else {
    return LISTEN_NOT_BOUND;
  }
}

int Connection::bind(short port)
{
#ifdef SGI_CONNECTION
  sock = socket(PF_INET, SOCK_STREAM, 0);
#else
  sock = socket(AF_INET, SOCK_STREAM, 0);
#endif

  if (sock < 0) return BIND_COULD_NOT_GET_SOCKET;

  addr.sin_port = htons(port);

  if (::bind(sock, (sockaddr *) &addr, sizeof(addr)) == -1)
  {
#ifdef _MSC_VER
    ::closesocket(sock);
#else
    ::close(sock);
#endif
    return BIND_COULD_NOT_BIND_SOCKET;
  }

  bound = true;
  return 0;
}

int Connection::accept(Connection *conn)
{
  int ns = -1;

  if (!listening) return ACCEPT_NOT_LISTENING;

  if (conn == NULL) return ACCEPT_CONNECTION_NULL;
  if (conn->connected || conn->bound) return ACCEPT_CONNECTION_USED;

#ifdef SGI_CONNECTION
  int addrlen = sizeof(addr);
#else
#ifdef _MSC_VER
  int addrlen = sizeof(addr);
#else
  socklen_t addrlen = sizeof(addr);
#endif
#endif

  ns = ::accept(sock, (sockaddr *) &addr, &addrlen);
#ifdef _MSC_VER
  if (ns == INVALID_SOCKET)
  {
    if (WSAGetLastError() == WSAEWOULDBLOCK) return NONBLOCKING_EAGAIN;
    return ACCEPT_FAILED;
  }

#else
  if (ns < 0)
  {
    if (errno == EAGAIN) return NONBLOCKING_EAGAIN;
    return ACCEPT_FAILED;
  }
#endif

  conn->sock = ns;
  conn->connected = true;
  return 0;
}

int Connection::setNonBlocking(bool value)
{  
  if (!connected && !listening) return SETNONBLOCKING_NO_SOCKET;  
#ifdef _MSC_VER
  if (value)
  {
    unsigned long nonzero = 1;
    ioctlsocket(sock, FIONBIO, &nonzero);
    //WSAAsyncSelect(sock, NULL, 0, 0);
  } else {
    // or does it want NULL?
    unsigned long zero = 0;
    ioctlsocket(sock, FIONBIO, &zero);
  }
#else
  if (value)
  {
    // NOTICE: modification to original...
    signal(SIGIO, SIG_IGN);

    fcntl(sock, F_SETOWN, getpid());
    
#ifdef SGI_CONNECTION
    if (::fcntl(sock, F_SETFL, FNONBLK) == -1)
      return SETNONBLOCKING_FAILED;
#else
    if (::fcntl(sock, F_SETFL, O_ASYNC | O_NONBLOCK) == -1)
      return SETNONBLOCKING_FAILED;
#endif
  } else { 
    if (::fcntl(sock, F_SETFL, 0) == -1)
      return SETNONBLOCKING_FAILED;
  }
#endif
  return 0;
}

bool Connection::isConnected()
{
  return connected;
}

int Connection::send(char *buf, int len)
{
#ifdef _MSC_VER

//printf("send,");

  int ret = ::send(sock, buf, len, 0);
  unsigned long foo;
  WSAIoctl(sock, SIO_FLUSH, NULL, 0, NULL, 0, &foo, NULL, NULL);
  if (ret == SOCKET_ERROR)
  {
    if (WSAGetLastError() == WSAEWOULDBLOCK) return NONBLOCKING_EAGAIN;
    return SEND_FAILED;
  }

#else
#ifdef SGI_CONNECTION
  int ret = ::send(sock, buf, len, 0);
#else
  int ret = ::send(sock, buf, len, MSG_NOSIGNAL);
#endif
  if (ret == -1)
  {
    if (errno == EAGAIN) return NONBLOCKING_EAGAIN;
    return SEND_FAILED;
  }
#endif
  return ret;
}

int Connection::recv(char *buf, int maxlen)
{
#ifdef _MSC_VER

  int ret = ::recv(sock, buf, maxlen, 0);

  if (ret == SOCKET_ERROR)
  {
    if (WSAGetLastError() == WSAEWOULDBLOCK) return NONBLOCKING_EAGAIN;
    return RECV_FAILED;
  }
#else
#ifdef SGI_CONNECTION
  int ret = ::recv(sock, buf, maxlen, 0);
#else
  int ret = ::recv(sock, buf, maxlen, MSG_NOSIGNAL);
#endif
  if (ret == -1) 
  {
    if (errno == EAGAIN) return NONBLOCKING_EAGAIN;
    return RECV_FAILED;
  }
#endif
  return ret;
}


