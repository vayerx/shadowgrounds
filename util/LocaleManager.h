#ifndef UTIL_LOCALEMANAGER_H
#define UTIL_LOCALEMANAGER_H

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

namespace util {

class LocaleResource;

class LocaleManager
{
	struct Locale
	{
		std::vector<boost::shared_ptr<LocaleResource> > resources;

		std::string englishName;
		std::string nativeName;
		char id[2];

		Locale() { id[0] = 'a'; id[1] = 'a'; }
	};

	enum { MAX_LOCALES = 16 };
	Locale locales[MAX_LOCALES];

	std::vector<int> resourceLocale;
	std::vector<std::string> resourceGroup;
	const char *convertPath(const char localeId[2], const char *str) const;

protected:
	virtual int getBank(const char *str) = 0;
	virtual void loadResource(int id, const char *filename, const char localeId[2]);
	virtual void parse(const char localeId[2], const std::string &group);
	virtual void parse();

public:
	LocaleManager();
	virtual ~LocaleManager();

	void init(const char *configurationFile);
	void setCurrentLocale(int resourceId, int locale, const std::string &group);

	const char *getLocaleIdString(int number) const;
	const char *getLocaleEnglishName(int number) const;
	const char *getLocaleNativeName(int number) const;
	const LocaleResource *getResource(int resourceId) const;

	// Helpers
	const char *convertPath(int resourceId, const char *str) const;
	const char *convert(int resourceId, const char *str) const;
	const char *getString(int resourceId, const char *key) const;

//#ifdef FB_RU_HAX
	std::wstring getWideString(int resourceId, const char *key) const;
//#endif

	int getInt(int resourceId, const char *key, int defaultValue = 0) const;

	// added by Pete...
	bool	hasString( int resourceId, const char* key ) const;

	bool	getString( int resourceId, const char* key, const char **return_value ) const;
};

} // util

#endif
