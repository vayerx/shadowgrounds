#ifndef INCLUDED_FIND_FILE_WRAPPER_H
#define INCLUDED_FIND_FILE_WRAPPER_H

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

#endif