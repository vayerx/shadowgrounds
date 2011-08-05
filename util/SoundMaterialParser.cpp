
#include "precompiled.h"

#include "SoundMaterialParser.h"
#include "../editor/parser.h"
#include "../editor/string_conversions.h"
#include "../filesystem/input_file_stream.h"
#include "../filesystem/file_package_manager.h"
#include "../system/Logger.h"
#include <assert.h>

using namespace frozenbyte;
using namespace frozenbyte::editor;

namespace util {
namespace {

	struct MaterialSorter
	{
		bool operator () (const SoundMaterialParser::SoundMaterial &a, const SoundMaterialParser::SoundMaterial &b) const
		{
			return a.name < b.name;
		}
	};

} // unnamed

SoundMaterialParser::SoundMaterialParser()
{
	// Empty group
	{
		SoundMaterial material;
#ifdef LEGACY_FILES
		material.name = "NoSound";
#else
		material.name = "no_sound";
#endif

		soundMaterials.push_back(material);
	}

	EditorParser parser;
#ifdef LEGACY_FILES
	filesystem::InputStream matfile = filesystem::createInputFileStream("Data/Effects/sound_materials.txt");
#else
	filesystem::InputStream matfile = filesystem::createInputFileStream("data/effect/contact_materials.txt");
#endif
    matfile >> parser;

	const ParserGroup &globals = parser.getGlobals();
	int groups = globals.getSubGroupAmount();

	for(int i = 0; i < groups; ++i)
	{
		SoundMaterial material;
		material.name = globals.getSubGroupName(i);
		if(material.name.empty())
			continue;

		// Properties
		const ParserGroup &group = globals.getSubGroup(material.name);

		const std::string &forceString = group.getValue("required_force");
		material.requiredForce = convertFromString<float> (forceString, SOUNDMATERIALPARSER_DEFAULT_REQUIRED_FORCE);

		const std::string &accelString = group.getValue("required_acceleration");
		material.requiredAcceleration = convertFromString<float> (accelString, SOUNDMATERIALPARSER_DEFAULT_REQUIRED_ACCELERATION);

		const std::string &angAccelString = group.getValue("required_angular_acceleration");
		material.requiredAngularAcceleration = convertFromString<float> (angAccelString, SOUNDMATERIALPARSER_DEFAULT_REQUIRED_ANGULAR_ACCELERATION);

		const std::string &effectForceString = group.getValue("required_effect_force");
		material.requiredEffectForce = convertFromString<float> (effectForceString, SOUNDMATERIALPARSER_DEFAULT_REQUIRED_EFFECT_FORCE);

		const std::string &effectAccelString = group.getValue("required_effect_acceleration");
		material.requiredEffectAcceleration = convertFromString<float> (effectAccelString, SOUNDMATERIALPARSER_DEFAULT_REQUIRED_EFFECT_ACCELERATION);

		const std::string &effectAngAccelString = group.getValue("required_effect_angular_acceleration");
		material.requiredEffectAngularAcceleration = convertFromString<float> (effectAngAccelString, SOUNDMATERIALPARSER_DEFAULT_REQUIRED_EFFECT_ANGULAR_ACCELERATION);

		const std::string &effectMaxRateString = group.getValue("effect_max_rate");
		material.effectMaxRate = convertFromString<int> (effectMaxRateString, SOUNDMATERIALPARSER_DEFAULT_EFFECT_MAX_RATE);

		const std::string &priorityString = group.getValue("priority");
		material.priority = convertFromString<int> (priorityString, SOUNDMATERIALPARSER_DEFAULT_PRIORITY);

		// Sound files
		const ParserGroup &soundGroup = group.getSubGroup("Sounds");
		for(int j = 0; j < soundGroup.getLineCount(); ++j)
			material.sounds.push_back(soundGroup.getLine(j));

		// HACK: Effect files
		const ParserGroup &effectGroup = group.getSubGroup("Effects");
		for(int j = 0; j < effectGroup.getLineCount(); ++j)
			material.effects.push_back(effectGroup.getLine(j));

#ifdef LEGACY_FILES
		//if(material.name == "BaseMaterial")
		// <- this was actually never used???
		if(material.name == "base_material")
#else
		if(material.name == "base_material")
#endif
			continue;

		soundMaterials.push_back(material);
	}

	// Leave NoSound material as first not depending on alphabetical order
	std::sort(soundMaterials.begin() + 1, soundMaterials.end(), MaterialSorter());
}

SoundMaterialParser::~SoundMaterialParser()
{
}

int SoundMaterialParser::getMaterialAmount() const
{
	return soundMaterials.size();
}

const std::string &SoundMaterialParser::getMaterialName(int index) const
{
	assert(index >= 0 && index < getMaterialAmount());
	return soundMaterials[index].name;
}

const int SoundMaterialParser::getMaterialIndexByName(const std::string &name) const
{
	for (int i = 0; i < (int)soundMaterials.size(); i++)
	{
		if (soundMaterials[i].name == name)
		{
			return i;
		}
	}

	Logger::getInstance()->warning("SoundMaterialParser::getMaterialIndexByName - Given material name not found.");
	Logger::getInstance()->debug(name.c_str());

	return SOUNDMATERIALPARSER_NO_SOUND_INDEX;
}

const SoundMaterialParser::SoundMaterialList &SoundMaterialParser::getSoundMaterials() const
{
	return soundMaterials;
}

} // util
