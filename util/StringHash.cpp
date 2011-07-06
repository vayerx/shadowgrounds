
#include "precompiled.h"

#include "StringHash.h"
#include "../system/Logger.h"

#include "../util/fb_hash_map.h"

#include <assert.h>
#include "../util/Debug_MemoryManager.h"


#define STRINGHASH_HASHCODE_CALC(name, result) \
	{ \
		int hashCode = 0; \
		int hashi = 0; \
		int hashmult = 0; \
		while(name[hashi] != '\0') \
		{ \
			hashCode += (name[hashi] << hashmult); \
			hashCode += (name[hashi] * (13 + hashi)); \
			hashmult+=4; \
			if (hashmult > 23) hashmult -= 23; \
			hashi++; \
		} \
		*result = hashCode; \
	}


namespace util
{	
	typedef FB_HASH_MAP<int, std::string> StringHashType;

	class StringHashImpl
	{
	private:
		StringHashImpl()
		{
			nextUniqueHandle = 100000;
		};

		~StringHashImpl()
		{
			// nop
		};

		StringHashType hash;
		StringHashHandle nextUniqueHandle;

		friend class StringHash;
	};



	StringHash::StringHash()
	{
		this->impl = new StringHashImpl();
	}

	StringHash::~StringHash()
	{
		delete this->impl;
	}

	StringHashHandle StringHash::addUniqueString(const std::string &str)
	{
		StringHashHandle handle = impl->nextUniqueHandle;
		impl->nextUniqueHandle++;

		assert(!doesStringExistByHandle(handle));

		/*
		StringHashHandle handle = STRINGHASH_HASHCODE_CALC(str);

		// if there already exists a string with the same hash code (probably another instance of the same string),
		// add the handle number...
		int failsafe = 0;
		while (impl->hash.find(handle) != impl->hash.end())
		{
			handle++;
			failsafe++;
			if (failsafe > 1000)
			{
				assert(!"StringHash::addString - failsafe limit reached.");
				break;
			}
		}
		*/
		impl->hash.insert(std::pair<int,std::string>(handle, str));

		// returns a non-zero handle.
		assert(handle != 0);

		return handle;
	}

	bool StringHash::doesStringExistByHandle(StringHashHandle strHandle) const
	{
		if (impl->hash.find(strHandle) != impl->hash.end())
		{
			return true;
		} else {
			return false;
		}
	}

	std::string StringHash::getStringByHandle(StringHashHandle strHandle) const
	{
		StringHashType::iterator iter = impl->hash.find(strHandle);
		if (iter != impl->hash.end())
		{
			return iter->second;
		} else {
			LOG_ERROR("StringHash::getStringByHandle - No string exists with given handle.");
			assert(!"StringHash::getStringByHandle - No string exists with given handle.");
			return "";
		}
	}

	// NOTE: if you want unique string handles (no multiple references to one string entry), never call these
	StringHashHandle StringHash::getStringHandleByStringAddingIfMissing(const std::string &str)
	{
		StringHashHandle handle;
		STRINGHASH_HASHCODE_CALC(str, &handle);

		while (true)
		{
			if (doesStringExistByHandle(handle))
			{
				// hashcode already taken? is it the same string?
				std::string tmp = getStringByHandle(handle);
				if (tmp == str)
				{
					return handle;
				} else {
					// it's some other string, keep on looking for the right one...
					handle++;
				}
			} else {
				// found an empty slot, put it there.
				impl->hash.insert(std::pair<int,std::string>(handle, str));
				return handle;
			}
		}

	}

	StringHashHandle StringHash::getStringHandleByString(const std::string &str)
	{
		StringHashHandle handle;
		STRINGHASH_HASHCODE_CALC(str, &handle);

		while (true)
		{
			if (doesStringExistByHandle(handle))
			{
				// hashcode already taken? is it the same string?
				std::string tmp = getStringByHandle(handle);
				if (tmp == str)
				{
					return handle;
				} else {
					// it's some other string, keep on looking for the right one...
					handle++;
				}
			} else {
				// found an empty slot, so the string probably is not in the hash..
				// FIXME: this is an erroronous response if there are empty slots due to removed string entries...
				return 0;
			}
		}
	}

	// NOTE: these should not be called if there may be multiple references to the string or the other
	// handle references become invalid. (see above note)
	void StringHash::removeStringByString(const std::string &str)
	{

	}

	void StringHash::removeStringByHandle(StringHashHandle strHandle)
	{

	}

	void StringHash::clear()
	{

	}

}


