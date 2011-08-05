
#ifndef NET_NETDRIVEREXCEPTION_H
#define NET_NETDRIVEREXCEPTION_H

#include <stdlib.h>
#include <string.h>

// Copyright(C) Jukka Kokkonen, 2002

namespace net
{
  class NetDriverException
  {
    public:
      enum EXCEPTION_TYPE
      {
        EXCEPTION_TYPE_UNDEFINED = 0,
        EXCEPTION_TYPE_INVALID_PARAMETER = 1,
        EXCEPTION_TYPE_UNSUPPORTED_PARAMETER = 2,
        EXCEPTION_TYPE_CONNECTION_REFUSED = 3,
        EXCEPTION_TYPE_UNKNOWN_HOST = 4,
        EXCEPTION_TYPE_BIND_FAILED = 5,
        EXCEPTION_TYPE_RESOURCE_ALLOCATION_FAILED = 6,
        EXCEPTION_TYPE_INVALID_STATE = 7,
        EXCEPTION_TYPE_ACCEPT_FAILED = 8,
        EXCEPTION_TYPE_SEND_FAILED = 9,
        EXCEPTION_TYPE_RECEIVE_FAILED = 10,
        EXCEPTION_TYPE_BUFFER_FULL = 11,
        EXCEPTION_TYPE_TRY_AGAIN = 12
      };

      NetDriverException()
      {
        this->msg = NULL;
        this->exType = EXCEPTION_TYPE_UNDEFINED;
      }

      /*
      NetDriverException(char *msg) 
      {
        int slen = strlen(msg);
        this->msg = new char[slen + 1];
        strcpy(this->msg, msg);
        this->exType = EXCEPTION_TYPE_UNDEFINED;
      } 
      */ 

      NetDriverException(EXCEPTION_TYPE exType, char *msg) 
      {
        if (msg == NULL)
        {
          this->msg = NULL;
        } else {
          int slen = strlen(msg);
          this->msg = new char[slen + 1];
          strcpy(this->msg, msg);
        }
        this->exType = exType;
      }  

      NetDriverException(EXCEPTION_TYPE exType) 
      {
        this->msg = NULL;
        this->exType = exType;
      }  
    
      ~NetDriverException()
      {
        if (msg != NULL)
        {
          delete [] msg;
          msg = NULL;
        }
      }
    
      char *getMessage()
      {
        return msg;
      }

      EXCEPTION_TYPE getExceptionType()
      {
        return exType;
      }

    private:
      char *msg;
      EXCEPTION_TYPE exType;
  };
}

#endif

