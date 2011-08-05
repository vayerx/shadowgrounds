// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_IMODE_H
#define INCLUDED_EDITOR_IMODE_H

namespace frozenbyte {
namespace filesystem {
	class InputStream;
	class OutputStream;
}

namespace editor {

class Exporter;

class IMode
{
public:
	virtual ~IMode() {}

	virtual void tick() = 0;
	virtual void reset() = 0;
	virtual void update() = 0;

	virtual void doExport(Exporter &exporter) const = 0;
	virtual filesystem::OutputStream &writeStream(filesystem::OutputStream &stream) const = 0;
	virtual filesystem::InputStream &readStream(filesystem::InputStream &stream) = 0;
};

inline filesystem::OutputStream &operator << (filesystem::OutputStream &stream, const IMode &mode) 
{ 
	return mode.writeStream(stream);
}

inline filesystem::InputStream &operator >> (filesystem::InputStream &stream, IMode &mode) 
{ 
	return mode.readStream(stream);
}

} // end of namespace editor
} // end of namespace frozenbyte

#endif
