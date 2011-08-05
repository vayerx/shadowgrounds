#ifndef INCLUDED_FROZENBYTE_PHYSICS_FILE_STREAM_H
#define INCLUDED_FROZENBYTE_PHYSICS_FILE_STREAM_H

#include "../filesystem/input_stream.h"
#include "../filesystem/output_stream.h"
#include "NxPhysics.h"
#include "NxStream.h"

namespace frozenbyte {
namespace physics {

class InputPhysicsStream : public NxStream
{
	mutable filesystem::InputStream stream;

public:
	InputPhysicsStream(const char *filename);
	~InputPhysicsStream();

	NxU8 readByte() const;
	NxU16 readWord() const;
	NxU32 readDword() const;
	float readFloat() const;
	double readDouble() const;
	void readBuffer(void *buffer, NxU32 size) const;

	NxStream &storeByte(NxU8 b);
	NxStream &storeWord(NxU16 w);
	NxStream &storeDword(NxU32 d);
	NxStream &storeFloat(NxReal f);
	NxStream &storeDouble(NxF64 f);
	NxStream &storeBuffer(const void *buffer, NxU32 size);

	int getSize();
};

class OutputPhysicsStream : public NxStream
{
	mutable filesystem::OutputStream stream;

public:
	OutputPhysicsStream(const char *filename);
	~OutputPhysicsStream();

	NxU8 readByte() const;
	NxU16 readWord() const;
	NxU32 readDword() const;
	float readFloat() const;
	double readDouble() const;
	void readBuffer(void *buffer, NxU32 size) const;

	NxStream &storeByte(NxU8 b);
	NxStream &storeWord(NxU16 w);
	NxStream &storeDword(NxU32 d);
	NxStream &storeFloat(NxReal f);
	NxStream &storeDouble(NxF64 f);
	NxStream &storeBuffer(const void *buffer, NxU32 size);
};

} // physics
} // frozenbyte

#endif
