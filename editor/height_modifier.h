// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_HEIGHT_MODIFIER_H
#define INCLUDED_EDITOR_HEIGHT_MODIFIER_H

#include <boost/scoped_ptr.hpp>
#include <datatypedef.h>

namespace frozenbyte {
namespace editor {

class HeightModifier
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	HeightModifier(unsigned short *buffer, const VC2I &resolution, const VC3 &size);
	~HeightModifier();

	enum Shape
	{
		Circle,
		Rectangle
	};

	// Strength in -1..1 range
	void changeHeight(const VC3 &position, float radius, float strength, Shape shape);
	void flatten(const VC3 &position, float radius, float strength, float height, Shape shape);
	void smoothen(const VC3 &position, float radius, float strength, Shape shape);
};

} // editor
} // frozenbyte

#endif
