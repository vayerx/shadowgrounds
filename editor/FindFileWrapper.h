#ifndef INCLUDED_FIND_FILE_WRAPPER_H
#define INCLUDED_FIND_FILE_WRAPPER_H

#if defined WIN32 && !defined __WINE__

#include <io.h>
#include <stdio.h>

namespace frozenbyte {
namespace editor {

	class FindFileWrapper
	{
	public:
		enum Type
		{
			File,
			Dir
		};

	private:
		Type type;

		long handle;
		int result;

		_finddata_t data;

		bool isValid() const
		{
			std::string file = data.name;
			if(file == ".")
				return false;
			if(file == "..")
				return false;
			if(file == "CVS")
				return false;
			if(file == ".svn")
				return false;

			if(type == Dir)
			{
				if(data.attrib & _A_SUBDIR)
					return true;
				else 
					return false;
			}
			else if(type == File)
			{
				if(data.attrib & _A_SUBDIR)
					return false;
				else 
					return true;
			}

			return false;
		}

	public:
		FindFileWrapper(const char *spec, Type type_)
		:	type(type_),
			handle(-1),
			result(0)
		{
			handle = _findfirst(spec, &data);
			if(!isValid())
				next();
		}

		~FindFileWrapper()
		{
			if(handle != -1)
				_findclose(handle);
		}

		void next()
		{
			while(!end())
			{
				result = _findnext(handle, &data);

				if(isValid())
					break;
			}
		}

		const char *getName() const
		{
			return data.name;
		}

		bool end() const
		{
			if(handle == -1)
				return true;
			if(result == -1)
				return true;

			return false;
		}
	};

} // editor
} // frozenbyte

#else
#include <glob.h>
#include <sys/stat.h>
#include <string.h>
#include "igios.h"

namespace frozenbyte {
namespace editor {

	class FindFileWrapper
	{
	public:
		enum Type
		{
			File,
			Dir
		};

	private:
		Type type;

		glob_t handle;
		size_t current;

		bool isValid() const
		{
			if (handle.gl_pathv == NULL)
				return false;

			std::string file(handle.gl_pathv[current]);
			if(file == ".")
				return false;
			if(file == "..")
				return false;
			if(file == "CVS")
				return false;

			struct stat statbuf;
			stat(handle.gl_pathv[current], &statbuf);

			// FIXME: handle symlinks

			if ((type == Dir && S_ISDIR(statbuf.st_mode)) || ((type == File) && !S_ISDIR(statbuf.st_mode)))
					return true;

			return false;
		}

	public:
		FindFileWrapper(const char *spec, Type type_)
		:	type(type_),
			current(0)
		{
			if (strchr(spec, '\\') != NULL) {
				igiosErrorMessage("FindFileWrapper: string contains backslash \"%s\"", spec);
				assert(false);
			}

			// FIXME: handle errors
			glob(spec, 0, NULL, &handle);

			if(!isValid())
                next();
			}

		~FindFileWrapper()
		{
			globfree(&handle);
		}

		void next()
		{
			while (++current < handle.gl_pathc) {
				if(isValid())
					break;
			}
		}

		const char *getName() const
		{
			if (handle.gl_pathv != NULL) {
				const char *temp = strrchr(handle.gl_pathv[current], '/');
				if (temp == NULL)
					temp = handle.gl_pathv[current];
				else
					temp += 1;

				return temp;
			} else
				return NULL;
		}

		bool end() const
		{
			if (handle.gl_pathv == NULL || current == handle.gl_pathc)
                return true;

			return false;
		}
	};

} // editor
} // frozenbyte

#endif

#endif
