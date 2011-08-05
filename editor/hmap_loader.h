// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_HMAP_LOADER_H
#define INCLUDED_HMAP_LOADER_H

#ifndef INCLUDED_VECTOR
#define INCLUDED_VECTOR
#include <vector>
#endif
#ifndef INCLUDED_STRING
#define INCLUDED_STRING
#include <string>
#endif

namespace frozenbyte {
namespace filesystem {
	class InputStream;
	class OutputStream;
}

namespace editor {

struct HmapFormat
{
	int format;
	int width;
	int height;

	HmapFormat()
	{
		format = 0;
		width = 0;
		height = 0;
	}
};

class HmapLoader
{
	std::vector<unsigned short> data;
	std::string fileName;
	
	HmapFormat format;

public:
	HmapLoader();
	explicit HmapLoader(const std::string &fileName);
	~HmapLoader();

	void update(unsigned short *buffer);
	void smooth();

	std::string getFileName();
	std::vector<unsigned short> &getData();

	int getWidth() const;
	int getHeight() const;

	filesystem::OutputStream &writeStream(filesystem::OutputStream &stream) const;
	filesystem::InputStream &readStream(filesystem::InputStream &stream);
};

inline filesystem::OutputStream &operator << (filesystem::OutputStream &stream, const HmapLoader &loader)
{ 
	return loader.writeStream(stream);
}

inline filesystem::InputStream &operator >> (filesystem::InputStream &stream, HmapLoader &loader)
{ 
	return loader.readStream(stream);
}

} // end of namespace editor
} // end of namespace frozenbyte

#endif
