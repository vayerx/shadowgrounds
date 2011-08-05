// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_FILESYSTEM_OUTPUT_STREAM_H
#define INCLUDED_FILESYSTEM_OUTPUT_STREAM_H

#ifndef INCLUDED_BOOST_SHARED_PTR_HPP
#define INCLUDED_BOOST_SHARED_PTR_HPP
#include <boost/shared_ptr.hpp>
#endif
#ifndef INCLUDED_STRING
#define INCLUDED_STRING
#include <string>
#endif

namespace frozenbyte {
namespace filesystem {

class IOutputStreamBuffer
{
public:
	virtual ~IOutputStreamBuffer() {}

	virtual void putByte(unsigned char byte) = 0;
};

class OutputStream
{
	boost::shared_ptr<IOutputStreamBuffer> streamBuffer;
	bool textStrings;

public:
	OutputStream();
	~OutputStream();

	void setBuffer(boost::shared_ptr<IOutputStreamBuffer> streamBuffer);
	void useTextStrings(); // Insert pure strings. Put std::endl manually!

private:
	OutputStream &write(const std::string &value);
	OutputStream &write(bool value);

	OutputStream &write(unsigned char value);
	OutputStream &write(signed char value);

	OutputStream &write(unsigned short value);
	OutputStream &write(signed short value);

	OutputStream &write(unsigned int value);
	OutputStream &write(signed int value);

	OutputStream &write(float value);
	OutputStream &write(double value);

	friend OutputStream &operator << (OutputStream &, const std::string &);
	friend OutputStream &operator << (OutputStream &, bool);
	friend OutputStream &operator << (OutputStream &, unsigned char);
	friend OutputStream &operator << (OutputStream &, signed char);
	friend OutputStream &operator << (OutputStream &, unsigned short);
	friend OutputStream &operator << (OutputStream &, signed short);
	friend OutputStream &operator << (OutputStream &, unsigned int);
	friend OutputStream &operator << (OutputStream &, signed int);
	friend OutputStream &operator << (OutputStream &, float);
	friend OutputStream &operator << (OutputStream &, double);
};

inline OutputStream &operator << (OutputStream &stream, const std::string &value) { return stream.write(value); }
inline OutputStream &operator << (OutputStream &stream, bool value) { return stream.write(value);  }

inline OutputStream &operator << (OutputStream &stream, unsigned char value) { return stream.write(value); }
inline OutputStream &operator << (OutputStream &stream, signed char value) { return stream.write(value); }

inline OutputStream &operator << (OutputStream &stream, unsigned short value) { return stream.write(value); }
inline OutputStream &operator << (OutputStream &stream, signed short value) { return stream.write(value); }

inline OutputStream &operator << (OutputStream &stream, unsigned int value) { return stream.write(value); }
inline OutputStream &operator << (OutputStream &stream, signed int value) { return stream.write(value); }

inline OutputStream &operator << (OutputStream &stream, float value) { return stream.write(value); }
inline OutputStream &operator << (OutputStream &stream, double value) { return stream.write(value); }

} // end of namespace filesystem
} // end of namespace frozenbyte

#endif
