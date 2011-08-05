#ifndef INCLUDED_SOUND_MATERIAL_PARSER_H
#define INCLUDED_SOUND_MATERIAL_PARSER_H

#include <vector>
#include <string>

// (0 index is assumed to be "NoSound" material, use this define instead of value 0 to refer to that) --jpk
#define SOUNDMATERIALPARSER_NO_SOUND_INDEX 0

#define SOUNDMATERIALPARSER_DEFAULT_REQUIRED_FORCE 100.0f
#define SOUNDMATERIALPARSER_DEFAULT_REQUIRED_ACCELERATION 1.0f
#define SOUNDMATERIALPARSER_DEFAULT_REQUIRED_ANGULAR_ACCELERATION 1.0f

#define SOUNDMATERIALPARSER_DEFAULT_REQUIRED_EFFECT_FORCE 100.0f
#define SOUNDMATERIALPARSER_DEFAULT_REQUIRED_EFFECT_ACCELERATION 1.0f
#define SOUNDMATERIALPARSER_DEFAULT_REQUIRED_EFFECT_ANGULAR_ACCELERATION 1.0f
#define SOUNDMATERIALPARSER_DEFAULT_EFFECT_MAX_RATE 200

#define SOUNDMATERIALPARSER_DEFAULT_PRIORITY 5


namespace util {

class SoundMaterialParser
{
public:
	SoundMaterialParser();
	~SoundMaterialParser();

	int getMaterialAmount() const;
	const std::string &getMaterialName(int index) const;

	const int getMaterialIndexByName(const std::string &name) const;

	struct SoundMaterial
	{
		std::string name;

		float requiredForce;
		float requiredAcceleration;
		float requiredAngularAcceleration;

		float requiredEffectForce;
		float requiredEffectAcceleration;
		float requiredEffectAngularAcceleration;
		int effectMaxRate;

		int priority;

		std::vector<std::string> sounds;
		
		// HACK: ...
		std::vector<std::string> effects;

		SoundMaterial()
		:	requiredForce(SOUNDMATERIALPARSER_DEFAULT_REQUIRED_FORCE),
			requiredAcceleration(SOUNDMATERIALPARSER_DEFAULT_REQUIRED_ACCELERATION),
			requiredAngularAcceleration(SOUNDMATERIALPARSER_DEFAULT_REQUIRED_ANGULAR_ACCELERATION),
			requiredEffectForce(SOUNDMATERIALPARSER_DEFAULT_REQUIRED_EFFECT_FORCE),
			requiredEffectAcceleration(SOUNDMATERIALPARSER_DEFAULT_REQUIRED_EFFECT_ACCELERATION),
			requiredEffectAngularAcceleration(SOUNDMATERIALPARSER_DEFAULT_REQUIRED_EFFECT_ANGULAR_ACCELERATION),
			effectMaxRate(SOUNDMATERIALPARSER_DEFAULT_EFFECT_MAX_RATE),
			priority(SOUNDMATERIALPARSER_DEFAULT_PRIORITY)
		{
		}
	};

	typedef std::vector<SoundMaterial> SoundMaterialList;
private:
	SoundMaterialList soundMaterials;

public:
	const SoundMaterialList &getSoundMaterials() const;

};

} // util

#endif
