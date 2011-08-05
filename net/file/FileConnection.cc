
// Copyright(C) Jukka Kokkonen, 2007

#include "FileConnection.h"

#include "FileBind.h"
#include "FileDriver.h"
#include "../../system/Timer.h"
#include <assert.h>
#include <string>

// TEMP: for debugging
#include <stdio.h>

#define FILECONNECTION_BUF_SIZE (65536 * 4) * 2



namespace net
{
  namespace file
  {

		class FileConnectionImpl
		{
		private:
			FileConnectionImpl()
			{
				iHaveTheWriteLockFile = false;
				connected = false;
			}

			~FileConnectionImpl()
			{
				if (iHaveTheWriteLockFile)
				{
					freeLockFile();
				}
			}

			void freeLockFile()
			{
				assert(iHaveTheWriteLockFile);

				if (lockFilename.empty())
				{
					iHaveTheWriteLockFile = false;
					return;
				}

				iHaveTheWriteLockFile = false;
				remove(lockFilename.c_str());
			}

			void acquireLockFile()
			{
				assert(!iHaveTheWriteLockFile);

				if (lockFilename.empty())
				{
					iHaveTheWriteLockFile = true;
					return;
				}

				while (true)
				{
					// TODO: use proper lock file creation (posix link / windows access locks)!
					FILE *f = fopen(lockFilename.c_str(), "rb");
					if (f != NULL)
					{
						// the lock file existed. continue to wait for it's disappearance.
						fclose(f);
					} else {
						// the lock file did not exist - quickly create it!
						FILE *f = fopen(lockFilename.c_str(), "wb");
						if (f != NULL)
						{
							fclose(f);
							// lockFileHandle = f;
							iHaveTheWriteLockFile = true;
							break;
						} else {
							// oh crap. failed, maybe someone was quicker and has just opened/locked it?
							// (or maybe the lockfile just cannot be created due to bad filename, out of disk space, etc.)
						}
					}
					Sleep(5);
				}

				// ok, got it.
			}

			bool iHaveTheWriteLockFile;
			bool connected;
			std::string lockFilename;

			friend class FileConnection;
		};


    FileConnection::FileConnection(const char *filename)
    {
			this->impl = new FileConnectionImpl();
			impl->connected = true;
    }


    FileConnection::~FileConnection()
    {
			close();
			delete this->impl;
    }


    void FileConnection::flush() 
      throw (NetDriverException*)
    {
      // this is problematic... cannot flush really.
      // cos the same process is also the one receiving the data.
      // thus, flush would have to wait forever for the data being received.
      return;
    }
 

    int FileConnection::send(const void *buf, int maxlen) 
      throw (NetDriverException*)
    {
      if (impl->connected)
      {
        throw new NetDriverException(NetDriverException::EXCEPTION_TYPE_INVALID_STATE,
          "FileConnection::send - Cannot send, connection closed.");
      }

			int wrote = 0; //fwrite(...);
      return wrote;
    }
    

    int FileConnection::receive(void *buf, int maxlen) 
      throw (NetDriverException*)
    {
			assert(maxlen != 0);

			if (!impl->connected)
			{
				throw new NetDriverException(NetDriverException::EXCEPTION_TYPE_INVALID_STATE,
					"FileConnection::receive - Cannot receive, connection closed.");
			}

      int got = 0; // fread(...);

      return got;
    }


    void FileConnection::close() 
      throw (NetDriverException*)
    {
			if (impl->connected)
			{
				// TODO: create a marker file to indicate being closed!
        impl->connected = false;
			}
    }


  }
}
