#ifndef INCLUDED_ISTORM3D_STREAM_BUFFER_H
#define INCLUDED_ISTORM3D_STREAM_BUFFER_H

#include <boost/shared_ptr.hpp>

#ifdef _WIN32
typedef unsigned __int64 Uint64;
#else
typedef uint64_t Uint64;
#endif

class IStorm3D_Stream
{
public:
	virtual ~IStorm3D_Stream() {}

	virtual void activate() = 0;
	virtual void deactivate() = 0;

	// channels * bits
	virtual void addSample(const char *buffer, int length, Uint64 start, Uint64 duration) = 0;
	virtual Uint64 getCurrentTime() const = 0;
};

class IStorm3D_StreamBuilder
{
public:
	virtual ~IStorm3D_StreamBuilder() {}

	virtual void update() = 0;

	virtual void setStereo(bool stereo) = 0;
	virtual void setFrequency(int data) = 0;
	virtual void setBits(int bits) = 0;
	virtual boost::shared_ptr<IStorm3D_Stream> getStream() = 0;
};


#endif
