#ifndef NET_FILECONNECTION_H
#define NET_FILECONNECTION_H

// Copyright(C) Jukka Kokkonen, 2007

#include "../INetConnection.h"

#define FILECONNECTION_LAG_RAND_AMOUNT 20

namespace net
{
  namespace file
  {
		class FileConnectionImpl;

    /**
     * File net connection.
     */
    class FileConnection : public INetConnection
    {
    public:
      FileConnection(const char *filename); 

      virtual ~FileConnection();
 
      virtual void flush() 
        throw (NetDriverException*);

      virtual int send(const void *buf, int maxlen) 
        throw (NetDriverException*);

      virtual int receive(void *buf, int maxlen) 
        throw (NetDriverException*);

      virtual void close() 
        throw (NetDriverException*);

			// special id'ed package management...
			// (packages can be changed later on, and read multiple times, etc.)
			void enableRecordIdManagement(bool recordIdEnabled);

			// seek stream position with given id or set to stream end if not found.
      void seekRecordId(int recordId, int subRecordId)
        throw (NetDriverException*);

			// following send will use this record id.
		  void setRecordId(int recordId, int subRecordId)
	      throw (NetDriverException*);

    private:
			FileConnectionImpl *impl;
    };
  }
}

#endif

