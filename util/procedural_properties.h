// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_PROCEDURAL_PROPERTIES_H
#define INCLUDED_PROCEDURAL_PROPERTIES_H

#include <DatatypeDef.h>
#include <boost/scoped_ptr.hpp>
#include <string>

namespace util {

class ProceduralProperties
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	ProceduralProperties();
	~ProceduralProperties();

	const VC2I &getTextureSize() const;

	struct Layer
	{
		std::string texture;
		VC2 scale;
		VC2 speed;

		Layer();
	};

	struct Source
	{
		Layer texture;
		Layer offset;
		VC2 radius;
		VC2 linearSpeed;

		Source();
	};

	struct Effect
	{
		std::string name;
		std::string fallback;
		bool enableDistortion;

		Source source1;
		Source source2;
		Source distortion1;
		Source distortion2;

		Effect();
	};

	int getEffectAmount() const;
	const Effect &getEffect(int index) const;
};

} // util

#endif
