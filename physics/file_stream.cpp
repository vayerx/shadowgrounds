
#include "precompiled.h"

#include "file_stream.h"
#include "../filesystem/file_package_manager.h"
#include "../filesystem/output_file_stream.h"

namespace frozenbyte {
namespace physics {

using namespace filesystem;

// ------------
// InputStream
// ------------

InputPhysicsStream::InputPhysicsStream(const char *filename)
{
	stream = FilePackageManager::getInstance().getFile(filename);
}

InputPhysicsStream::~InputPhysicsStream()
{
}

NxU8 InputPhysicsStream::readByte() const
{
	unsigned char value;
	stream >> value;
	return value;
}

NxU16 InputPhysicsStream::readWord() const
{
	unsigned short value;
	stream >> value;
	return value;
}

NxU32 InputPhysicsStream::readDword() const
{
	unsigned int value;
	stream >> value;
	return value;
}

float InputPhysicsStream::readFloat() const
{
	float value;
	stream >> value;
	return value;
}

double InputPhysicsStream::readDouble() const
{
	double value;
	stream >> value;
	return value;
}

void InputPhysicsStream::readBuffer(void *buffer, NxU32 size) const
{
	char *ptr = static_cast<char *> (buffer);
	stream.read(ptr, size);
}

NxStream &InputPhysicsStream::storeByte(NxU8 b)
{
	return *this;
}

NxStream &InputPhysicsStream::storeWord(NxU16 w)
{
	return *this;
}

NxStream &InputPhysicsStream::storeDword(NxU32 d)
{
	return *this;
}

NxStream &InputPhysicsStream::storeFloat(NxReal f)
{
	return *this;
}

NxStream &InputPhysicsStream::storeDouble(NxF64 f)
{
	return *this;
}

NxStream &InputPhysicsStream::storeBuffer(const void *buffer, NxU32 size)
{
	return *this;
}

int InputPhysicsStream::getSize()
{
	return stream.getSize();
}

// ------------
// OutputStream
// ------------

OutputPhysicsStream::OutputPhysicsStream(const char *filename)
{
	stream = createOutputFileStream(filename);
}

OutputPhysicsStream::~OutputPhysicsStream()
{
}

NxU8 OutputPhysicsStream::readByte() const
{
	return 0;
}

NxU16 OutputPhysicsStream::readWord() const
{
	return 0;
}

NxU32 OutputPhysicsStream::readDword() const
{
	return 0;
}

float OutputPhysicsStream::readFloat() const
{
	return 0;
}

double OutputPhysicsStream::readDouble() const
{
	return 0;
}

void OutputPhysicsStream::readBuffer(void *buffer, NxU32 size) const
{
}

NxStream &OutputPhysicsStream::storeByte(NxU8 b)
{
	stream << b;
	return *this;
}

NxStream &OutputPhysicsStream::storeWord(NxU16 w)
{
	stream << w;
	return *this;
}

NxStream &OutputPhysicsStream::storeDword(NxU32 d)
{
	stream << d;
	return *this;
}

NxStream &OutputPhysicsStream::storeFloat(NxReal f)
{
	stream << f;
	return *this;
}

NxStream &OutputPhysicsStream::storeDouble(NxF64 f)
{
	stream << f;
	return *this;
}

NxStream &OutputPhysicsStream::storeBuffer(const void *buffer, NxU32 size)
{
	const unsigned char *ptr = static_cast<const unsigned char *> (buffer);
	for(unsigned int i = 0; i < size; ++i)
		stream << ptr[i];

	return *this;
}

} // physics
} // frozenbyte
