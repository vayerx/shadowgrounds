
#include "UniqueEditorObjectHandleManager.h"

#include "../system/Logger.h"
#include "../convert/str2int.h"
#include <stdio.h>
#include <assert.h>


static int ueoh_base = 0;
static int ueoh_current_iterator_value = 0;

static bool ueoh_error = false;
static bool ueoh_first_time_init = false;
static bool ueoh_locked = false;


void UniqueEditorObjectHandleManager::init()
{
	ueoh_first_time_init = false;
	ueoh_locked = false;
	ueoh_error = false;

	{
		FILE *f = fopen("config/editor_unique_handle_base.txt", "rb");
		if (f != NULL)
		{
			// base file existed (this is NOT the first time)

			fseek(f, 0, SEEK_END);
			int size = ftell(f);
			fseek(f, 0, SEEK_SET);
			char *buf = new char[size + 1];
			int got = fread(buf, size, 1, f);
			buf[size] = '\0';
			if (got != 1)
			{
				buf[0] = '\0';
				Logger::getInstance()->error("UniqueEditorObjectHandleManager::init - Failed reading data.");
				assert(!"UniqueEditorObjectHandleManager::init - Failed reading data.");

				ueoh_error = true;
			}

			// NOTE: no end-of-line or crap like that supported.
			ueoh_base = str2int(buf);
			if (ueoh_base <= 0 || ueoh_base >= (1<<11))
			{
				Logger::getInstance()->error("UniqueEditorObjectHandleManager::init - Bad base value.");
				assert(!"UniqueEditorObjectHandleManager::init - Bad base value.");
				ueoh_base = 0;
				ueoh_error = true;
			}

			delete [] buf;
			fclose(f);
		} else {
			// base file did not exist (this IS the first time)
			ueoh_first_time_init = true;

#ifdef LEGACY_FILES
		// (should be ifdef project_survivor, but survivor's editor does not have that defined)
			int retValue = -1;
#else
			int retValue = system("tools\\fetch_unique_handle_base.bat");
#endif

			ueoh_base = retValue;

			if (ueoh_base <= 0 || ueoh_base >= (1<<11))
			{
				Logger::getInstance()->error("UniqueEditorObjectHandleManager::init - Fetch returned bad base value.");
				assert(!"UniqueEditorObjectHandleManager::init - Fetch returned bad base value.");
				ueoh_base = 1;
				ueoh_error = true;
			}

			char buf[16];
			sprintf(buf, "%d", ueoh_base);

			FILE *f = fopen("config/editor_unique_handle_base.txt", "wb");
			if (f != NULL)
			{
				int got = fwrite(buf, strlen(buf), 1, f);
				if (got != 1)
				{
					Logger::getInstance()->error("UniqueEditorObjectHandleManager::init - Failed writing data.");
					assert(!"UniqueEditorObjectHandleManager::init - Failed writing data.");

					ueoh_error = true;
				}
				fclose(f);
			} else {
				Logger::getInstance()->error("UniqueEditorObjectHandleManager::init - Failed to open file for writing.");
				assert(!"UniqueEditorObjectHandleManager::init - Failed to open file for writing.");

				ueoh_error = true;
			}
		}
	}

	{
		FILE *f = fopen("config/editor_unique_handle_iterator.txt", "rb");
		if (f != NULL)
		{
			fseek(f, 0, SEEK_END);
			int size = ftell(f);
			fseek(f, 0, SEEK_SET);
			char *buf = new char[size + 1];
			int got = fread(buf, size, 1, f);
			buf[size] = '\0';
			if (got != 1)
			{
				buf[0] = '\0';
				Logger::getInstance()->error("UniqueEditorObjectHandleManager::init - Failed reading iterator value data.");
				assert(!"UniqueEditorObjectHandleManager::init - Failed reading iterator value data.");

				ueoh_error = true;
			}

			// NOTE: no end-of-line or crap like that supported.
			ueoh_current_iterator_value = str2int(buf);

			if (ueoh_current_iterator_value <= 0 || ueoh_current_iterator_value >= (1<<20))
			{
				Logger::getInstance()->error("UniqueEditorObjectHandleManager::init - Bad iterator value.");
				assert(!"UniqueEditorObjectHandleManager::init - Bad iterator value.");
				ueoh_current_iterator_value = 0;
				ueoh_error = true;
			}

			delete [] buf;
			fclose(f);
		} else {
			// iterator file did not exist (this IS the first time)
			ueoh_first_time_init = true;
			ueoh_current_iterator_value = 1;
		}
	}

	{
		// already locked???
		FILE *f = fopen("config/editor_lock.tmp", "rb");
		if (f != NULL)
		{
			fclose(f);
			ueoh_locked = true;
		} else {
			// do lock.
			FILE *f = fopen("config/editor_lock.tmp", "wb");
			if (f != NULL)
			{
				fclose(f);
			}
		}
	}

}


bool UniqueEditorObjectHandleManager::wasFirstTimeInit()
{
	return ueoh_first_time_init;
}


bool UniqueEditorObjectHandleManager::wasLocked()
{
	return ueoh_locked;
}


bool UniqueEditorObjectHandleManager::wasError()
{
	return ueoh_error;
}


void UniqueEditorObjectHandleManager::uninit()
{
	ueoh_first_time_init = false;
	ueoh_locked = false;
	ueoh_error = false;

	// write current unique handle to disk.
	char buf[16];
	sprintf(buf, "%d", ueoh_current_iterator_value);

	{
		FILE *f = fopen("config/editor_unique_handle_iterator.txt", "wb");
		if (f != NULL)
		{
			int got = fwrite(buf, strlen(buf), 1, f);
			if (got != 1)
			{
				Logger::getInstance()->error("UniqueEditorObjectHandleManager::uninit - Failed writing data.");
				assert(!"UniqueEditorObjectHandleManager::uninit - Failed writing data.");

				ueoh_error = true;
			}
			fclose(f);
		} else {
			Logger::getInstance()->error("UniqueEditorObjectHandleManager::uninit - Failed to open file for writing.");
			assert(!"UniqueEditorObjectHandleManager::uninit - Failed to open file for writing.");

			ueoh_error = true;
		}
	}

	// remove lock
	remove("config/editor_lock.tmp");
}


UniqueEditorObjectHandle UniqueEditorObjectHandleManager::createNewUniqueHandle(unsigned int internalHandleValue)
{
	assert(ueoh_base != 0);

	UniqueEditorObjectHandle ret = 0;

	ueoh_current_iterator_value += 1;
	ueoh_current_iterator_value &= ((1<<20)-1);

	ret |= internalHandleValue;
	ret |= (((UniqueEditorObjectHandle)ueoh_current_iterator_value) << 32);
	ret |= (((UniqueEditorObjectHandle)ueoh_base) << 52);

	return ret;
}


