
#ifndef Connection_h
#define Connection_h

//
// Connection class for linux TCP/IP
// Copyright(C) Jukka Kokkonen, 2000
//
// Modified 2002: Added support for Windows too.
// (Also partial support for SGI or other unix systems)
//

#define CONNECT_NULL_NAME -1
#define CONNECT_NULL_PORT -2
#define CONNECT_UNKNOWN_HOST -3
#define CONNECT_CONNECTION_REFUSED -4

#define BIND_COULD_NOT_GET_SOCKET -5
#define BIND_COULD_NOT_BIND_SOCKET -6

#define LISTEN_NOT_BOUND -7

#define ACCEPT_NOT_LISTENING -8
#define ACCEPT_FAILED -9
#define ACCEPT_CONNECTION_USED -10
#define ACCEPT_CONNECTION_NULL -11

#define CLOSE_NOT_CONNECTED -12

#define UNBIND_NOT_BOUND -13
    
#define SETNONBLOCKING_NO_SOCKET -14
#define SETNONBLOCKING_FAILED -15

#define SEND_FAILED -16

#define RECV_FAILED -17

#define NONBLOCKING_EAGAIN -18


#ifdef _MSC_VER
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif


class Connection
{
  private:
    struct sockaddr_in addr;
    int sock;
    bool connected;
    bool listening;
    bool bound;
  public:
    Connection();
    ~Connection();
    int connect(char *hostname, short port);
    int close();
    int bind(short port);
    int listen(int backlog);
    int unbind();
    int accept(Connection *conn);
    int setNonBlocking(bool value);
    bool isConnected();
    int send(char *buf, int len);
    int recv(char *buf, int maxlen);
};



#endif


