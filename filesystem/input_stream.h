// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_FILESYSTEM_INPUT_STREAM_H
#define INCLUDED_FILESYSTEM_INPUT_STREAM_H

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

class IInputStreamBuffer
{
public:
	virtual ~IInputStreamBuffer() {}

	virtual unsigned char popByte() = 0;
	virtual bool isEof() const = 0;
	virtual int getSize() const = 0;

	virtual void popBytes(char *buffer, int bytes) = 0;
};

class InputStream
{
	boost::shared_ptr<IInputStreamBuffer> streamBuffer;

public:
	InputStream();
	~InputStream();

	void setBuffer(boost::shared_ptr<IInputStreamBuffer> streamBuffer);
	bool isEof() const;
	int getSize() const;

private:
	InputStream &read(std::string &value);
	InputStream &read(bool &value);

	InputStream &read(unsigned char &value);
	InputStream &read(char &value);
	InputStream &read(signed char &value);

	InputStream &read(unsigned short &value);
	InputStream &read(signed short &value);

	InputStream &read(unsigned int &value);
	InputStream &read(signed int &value);

	InputStream &read(float &value);
	InputStream &read(double &value);

public:
	// Optimized readers
	InputStream &read(unsigned char *buffer, int elements);
	InputStream &read(char *buffer, int elements);
	InputStream &read(unsigned short *buffer, int elements);

	friend InputStream &operator >> (InputStream &, std::string &);
	friend InputStream &operator >> (InputStream &, bool &);
	friend InputStream &operator >> (InputStream &, unsigned char &);
	friend InputStream &operator >> (InputStream &, char &);
	friend InputStream &operator >> (InputStream &, signed char &);
	friend InputStream &operator >> (InputStream &, unsigned short &);
	friend InputStream &operator >> (InputStream &, signed short &);
	friend InputStream &operator >> (InputStream &, unsigned int &);
	friend InputStream &operator >> (InputStream &, signed int &);
	friend InputStream &operator >> (InputStream &, float &);
	friend InputStream &operator >> (InputStream &, double &);
};

inline InputStream &operator >> (InputStream &stream, std::string &value) { return stream.read(value); }
inline InputStream &operator >> (InputStream &stream, bool &value) { return stream.read(value); }

inline InputStream &operator >> (InputStream &stream, unsigned char &value) { return stream.read(value); }
inline InputStream &operator >> (InputStream &stream, char &value) { return stream.read(value); }
inline InputStream &operator >> (InputStream &stream, signed char &value) { return stream.read(value); }

inline InputStream &operator >> (InputStream &stream, unsigned short &value) { return stream.read(value); }
inline InputStream &operator >> (InputStream &stream, signed short &value) { return stream.read(value); }

inline InputStream &operator >> (InputStream &stream, unsigned int &value) { return stream.read(value); }
inline InputStream &operator >> (InputStream &stream, signed int &value) { return stream.read(value); }

inline InputStream &operator >> (InputStream &stream, float &value) { return stream.read(value); }
inline InputStream &operator >> (InputStream &stream, double &value) { return stream.read(value); }

} // end of namespace filesystem
} // end of namespace frozenbyte

#endif
