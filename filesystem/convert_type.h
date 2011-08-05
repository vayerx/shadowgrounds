// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_CONVERT_TYPE_H
#define INCLUDED_EDITOR_CONVERT_TYPE_H

#ifndef INCLUDED_CASSERT
#define INCLUDED_CASSERT
#include <cassert>
#endif

namespace frozenbyte {
namespace filesystem {

template<class Type>
struct ConvertBase
{
	enum { charCount = sizeof(Type) };
	union Values
	{
		unsigned char c[charCount];
		Type t;
	};

	Values value;

	int getSize() const
	{
		return charCount;
	}
};

template<class Type>
struct ConvertFrom: private ConvertBase<Type>
{
	explicit ConvertFrom(const Type &t)
	{
		ConvertBase<Type>::value.t = t;
	}

	using ConvertBase<Type>::getSize;

	unsigned char getByte(int index) const
	{
		assert((index >= 0) && (index < getSize()));
		return ConvertBase<Type>::value.c[index];
	}
};

template<class Type>
struct ConvertTo: private ConvertBase<Type>
{
	using ConvertBase<Type>::getSize;

	void setByte(int index, unsigned char c)
	{
		assert((index >= 0) && (index < getSize()));
		ConvertBase<Type>::value.c[index] = c;
	}

	const Type &getValue() const
	{
		return ConvertBase<Type>::value.t;
	}
};

} // end of namespace filesystem
} // end of namespace frozenbyte

#endif
