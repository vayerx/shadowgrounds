// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_FILESYSTEM_FILE_PACKAGE_MANAGER
#define INCLUDED_FILESYSTEM_FILE_PACKAGE_MANAGER

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif
#ifndef INCLUDED_BOOST_SHARED_PTR_HPP
#define INCLUDED_BOOST_SHARED_PTR_HPP
#include <boost/shared_ptr.hpp>
#endif
#ifndef INCLUDED_FILESYSTEM_IFILE_PACKAGE
#include "ifile_package.h"
#endif

namespace frozenbyte {
namespace filesystem {

class IFileList;
struct FilePackageManagerData;

class FilePackageManager
{
	boost::scoped_ptr<FilePackageManagerData> data;

public:
	FilePackageManager();
	~FilePackageManager();

	void addPackage(boost::shared_ptr<IFilePackage> filePackage, int priority);	
	boost::shared_ptr<IFileList> findFiles(const std::string &dir, const std::string &extension, bool caseSensitive = false);
	InputStream getFile(const std::string &fileName);
	unsigned int getCrc(const std::string &fileName);

	void setInputStreamErrorReporting(bool logNonExisting);

	static FilePackageManager &getInstance();
	static void setInstancePtr(FilePackageManager *instance);

};

} // end of namespace filesystem
} // end of namespace frozenbyte

#endif
