// Copyright (c) 2006 Frozenbyte

#ifndef INCLUDED_MOD_SELECTOR_H
#define INCLUDED_MOD_SELECTOR_H

#include <string>
#include <boost/scoped_ptr.hpp>

namespace util {

class ModSelector
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	ModSelector();
	~ModSelector();

	//const std::string &getActiceModFile() const;
	int getActiveIndex() const;
	int getModAmount() const;
	const std::string &getDescription(int index) const;
	const std::string &getModDir(int index) const;
	void fixFileName(std::string &file) const;

	void saveActiveModFile(int index);
	void restoreDir();
	void changeDir();
};

} // util

#endif
