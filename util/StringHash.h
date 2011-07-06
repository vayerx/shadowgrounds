
#ifndef STRINGHASH_H
#define STRINGHASH_H

#include <string>

namespace util
{
	class StringHashImpl;

	typedef int StringHashHandle;

	class StringHash
	{
	public:

		StringHash();
		~StringHash();

		// returns a non-zero handle. (except for empty string, the handle may be zero?)
		StringHashHandle addUniqueString(const std::string &str);

		bool doesStringExistByHandle(StringHashHandle strHandle) const;

		std::string getStringByHandle(StringHashHandle strHandle) const;

		// NOTE: if you want unique string handles (no multiple references to one string entry), never call these
		// Also, if any strings are removed while you also have some unique handles, the result for string seek
		// by these methods may be incorrect (the unique handle will not be found even though the string matches
		// this one)
		StringHashHandle getStringHandleByStringAddingIfMissing(const std::string &str);
		StringHashHandle getStringHandleByString(const std::string &str);

		// NOTE: these should not be called if there may be multiple references to the string or the other
		// handle references become invalid. (see above note)
		void removeStringByString(const std::string &str);
		void removeStringByHandle(StringHashHandle strHandle);

		void clear();

	private:
		StringHashImpl *impl;
	};
}

#endif

