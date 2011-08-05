#ifndef LOCALERESOURCE_H
#define LOCALERESOURCE_H

#include <string>
#include <map>

namespace util {

// NOTE: convert()'s returned pointer points to static buffer, overwritten
// on next call. (copy the result to another buffer if necessary)

class LocaleResource
{
	std::map<std::string, std::string> values;

public:
	LocaleResource();
	~LocaleResource();

	const char *get(const char *key) const;
	const char *convert(const char *str) const;

	void addPair(const char *key, const char *value);
};

} // util

#endif

