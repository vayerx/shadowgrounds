
#include "precompiled.h"

// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#pragma warning(disable:4786)
#endif

#include "procedural_properties.h"
#include "../editor/parser.h"
#include "../editor/string_conversions.h"
#include "../filesystem/file_package_manager.h"
#include "../filesystem/input_stream.h"
#include <fstream>
#include <map>

using namespace frozenbyte;
using namespace frozenbyte::editor;
using namespace boost;
using namespace std;

namespace util {

// Misc

ProceduralProperties::Layer::Layer()
{
}

ProceduralProperties::Source::Source()
{
}

ProceduralProperties::Effect::Effect()
:	enableDistortion(false)
{
}

// ...

typedef map<string, ProceduralProperties::Effect> Effects;

struct ProceduralProperties::Data
{
	VC2I textureSize;
	Effects effects;

	Data()
	{
	}

	bool parse(Source &source, const ParserGroup &group)
	{
		const ParserGroup &textureGroup = group.getSubGroup("Texture");
		const ParserGroup &offsetGroup = group.getSubGroup("Offset");

		// Texture
		source.texture.texture = textureGroup.getValue("name");
		if(source.texture.texture.empty())
			return false;
		source.texture.scale.x = convertFromString<float> (group.getValue("scale_x"), 1.f);
		source.texture.scale.y = convertFromString<float> (group.getValue("scale_y"), 1.f);
		source.texture.speed.x = convertFromString<float> (textureGroup.getValue("speed_x"), 1.f);
		source.texture.speed.y = convertFromString<float> (textureGroup.getValue("speed_y"), 1.f);

		// Offset
		source.offset.texture = offsetGroup.getValue("name");
		if(source.offset.texture.empty())
			return false;
		source.offset.scale.x = convertFromString<float> (offsetGroup.getValue("amplitude"), 1.f);
		source.offset.speed.x = convertFromString<float> (offsetGroup.getValue("speed_x"), 1.f);
		source.offset.speed.y = convertFromString<float> (offsetGroup.getValue("speed_y"), 1.f);

		source.radius.x = convertFromString<float> (offsetGroup.getValue("radius_x"), 1.f);
		source.radius.y = convertFromString<float> (offsetGroup.getValue("radius_y"), 1.f);
		source.linearSpeed.x = convertFromString<float> (offsetGroup.getValue("linear_speed_x"), 0.f);
		source.linearSpeed.y = convertFromString<float> (offsetGroup.getValue("linear_speed_y"), 0.f);

		return true;
	}

	void parseDistortion(Source &source, const ParserGroup &group)
	{
		const ParserGroup &distortionGroup = group.getSubGroup("Distortion");

		source.texture.scale.x = convertFromString<float> (group.getValue("scale_x"), 1.f);
		source.texture.scale.y = convertFromString<float> (group.getValue("scale_y"), 1.f);

		// Distortion
		source.offset.texture = distortionGroup.getValue("name");
		source.offset.scale.x = convertFromString<float> (distortionGroup.getValue("amplitude"), 1.f);
		source.offset.speed.x = convertFromString<float> (distortionGroup.getValue("speed_x"), 1.f);
		source.offset.speed.y = convertFromString<float> (distortionGroup.getValue("speed_y"), 1.f);

		source.radius.x = convertFromString<float> (distortionGroup.getValue("radius_x"), 1.f);
		source.radius.y = convertFromString<float> (distortionGroup.getValue("radius_y"), 1.f);
		source.linearSpeed.x = convertFromString<float> (distortionGroup.getValue("linear_speed_x"), 0.f);
		source.linearSpeed.y = convertFromString<float> (distortionGroup.getValue("linear_speed_y"), 0.f);
	}

	void parse()
	{
		EditorParser parser;
#ifdef LEGACY_FILES
		filesystem::InputStream proc_file = filesystem::FilePackageManager::getInstance().getFile("Data/Effects/procedurals.txt");
#else
		filesystem::InputStream proc_file = filesystem::FilePackageManager::getInstance().getFile("data/effect/procedurals.txt");
#endif
		proc_file >> parser;

		const ParserGroup &root = parser.getGlobals();
		textureSize.x = convertFromString<int> (root.getValue("size_x"), 128);
		textureSize.y = convertFromString<int> (root.getValue("size_y"), 128);

		int effectAmount = root.getSubGroupAmount();
		for(int i = 0; i < effectAmount; ++i)
		{
			const string &name = root.getSubGroupName(i);
			const ParserGroup &group = root.getSubGroup(name);

			Effect effect;
			if(!parse(effect.source1, group.getSubGroup("Source1")))
				continue;
			if(!parse(effect.source2, group.getSubGroup("Source2")))
				continue;

			parseDistortion(effect.distortion1, group.getSubGroup("Source1"));
			parseDistortion(effect.distortion2, group.getSubGroup("Source2"));

			effect.name = name;
			effect.enableDistortion = convertFromString<bool> (group.getValue("enable_distortion"), false);
			effect.fallback = group.getValue("fallback");

			effects[name] = effect;
		}
	}
};

ProceduralProperties::ProceduralProperties()
{
	scoped_ptr<Data> tempData(new Data());
	tempData->parse();

	data.swap(tempData);
}

ProceduralProperties::~ProceduralProperties()
{
}

const VC2I &ProceduralProperties::getTextureSize() const
{
	return data->textureSize;
}

int ProceduralProperties::getEffectAmount() const
{
	return data->effects.size();
}

const ProceduralProperties::Effect &ProceduralProperties::getEffect(int index) const
{
	assert(index >= 0 && index < int(data->effects.size()));

	Effects::iterator it = data->effects.begin();
	for(int i = 0; it != data->effects.end(); ++it)
	{
		if(i++ == index)
			return it->second;
	}

	static Effect empty;
	assert(!"Shouldn't get here");
	return empty;
}

} // util
