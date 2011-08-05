// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "explosion_scripts.h"
#include "parser.h"
#include "../filesystem/input_file_stream.h"
#include "../filesystem/file_package_manager.h"
#include <vector>
#include <fstream>
#include <algorithm>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace boost;

namespace frozenbyte {
namespace editor {
namespace {

void parse(ExplosionScript &script, const ParserGroup &group)
{
	script.script = group.getValue("script");
}

void parse(ExplosionProjectile &projectile, const ParserGroup &group)
{
	projectile.projectile = group.getValue("projectile");
}

void parse(ExplosionEffect &effect, const ParserGroup &group)
{
	effect.effect = group.getValue("effect");
}

void parse(ExplosionSound &sound, const ParserGroup &group)
{
	for(int i = 0; i < group.getLineCount(); ++i)
		sound.sounds.push_back(group.getLine(i));
}

void parse(Material &material, const ParserGroup &group)
{
	material.material = group.getValue("material");
}

void parse(Animation &animation, const ParserGroup &group)
{
	animation.bones = group.getValue("bones");
	animation.idleAnimation = group.getValue("idle_animation");

	const ParserGroup &a = group.getSubGroup("Animation");
	for(int i = 0; i < a.getLineCount(); ++i)
		animation.animations.push_back(a.getLine(i));
}

template<typename T>
struct NameSorter: public binary_function<T, T, bool>
{
	bool operator () (const T &a, const T &b) const
	{
		return a.name < b.name;
	}
};

template<typename T>
void parse(vector<T> &list, const ParserGroup &group, const string &base)
{
	list.resize(1);
	list[0].name = "(empty)";

	for(int i = 0; ; ++i)
	{
		string groupName = base;
		groupName += boost::lexical_cast<string> (i + 1);

		const ParserGroup &subGroup = group.getSubGroup(groupName);

		T entity;
		entity.name = subGroup.getValue("name");
		if(entity.name.empty())
			break;

		parse(entity, subGroup);
		list.push_back(entity);
	}

	sort(list.begin(), list.end(), NameSorter<T> ());
}

} // unnamed

struct ExplosionScripts::Data
{
	ScriptList scripts;
	ProjectileList projectiles;
	EffectList effects;
	SoundList sounds;
	MaterialList materials;
	AnimationList animations;

	void reload()
	{
		Parser sParser(true, false);
		Parser pParser(true, false);
		Parser eParser(true, false);
		Parser soParser(true, false);
		Parser mParser(true, false);
		Parser aParser(true, false);
		//ifstream("Editor/ExplosionScripts.fbt") >> sParser;
		//ifstream("Editor/ExplosionProjectiles.fbt") >> pParser;
		//ifstream("Editor/ExplosionEffects.fbt") >> eParser;
		//ifstream("Editor/ExplosionSounds.fbt") >> soParser;
		//ifstream("Editor/Materials.fbt") >> mParser;
		//ifstream("Editor/Animations.fbt") >> aParser;

		filesystem::FilePackageManager::getInstance().getFile("Editor/ExplosionScripts.fbt") >> sParser;
		filesystem::FilePackageManager::getInstance().getFile("Editor/ExplosionProjectiles.fbt") >> pParser;
		filesystem::FilePackageManager::getInstance().getFile("Editor/ExplosionEffects.fbt") >> eParser;
		filesystem::FilePackageManager::getInstance().getFile("Editor/ExplosionSounds.fbt") >> soParser;
		filesystem::FilePackageManager::getInstance().getFile("Editor/Materials.fbt") >> mParser;
		filesystem::FilePackageManager::getInstance().getFile("Editor/Animations.fbt") >> aParser;

		parse(scripts, sParser.getGlobals(), "Script");
		parse(projectiles, pParser.getGlobals(), "Projectile");
		parse(effects, eParser.getGlobals(), "Effect");
		parse(sounds, soParser.getGlobals(), "Sound");
		parse(materials, mParser.getGlobals(), "Material");
		parse(animations, aParser.getGlobals(), "Animation");
	}
};

ExplosionScripts::ExplosionScripts()
{
	scoped_ptr<Data> tempData(new Data());
	data.swap(tempData);
}

ExplosionScripts::~ExplosionScripts()
{
}

const ScriptList &ExplosionScripts::getScripts() const
{
	return data->scripts;
}

const ProjectileList &ExplosionScripts::getProjectiles() const
{
	return data->projectiles;
}

const EffectList &ExplosionScripts::getEffects() const
{
	return data->effects;
}

const SoundList &ExplosionScripts::getSounds() const
{
	return data->sounds;
}

const MaterialList &ExplosionScripts::getMaterials() const
{
	return data->materials;
}

const AnimationList &ExplosionScripts::getAnimations() const
{
	return data->animations;
}

const std::string &ExplosionScripts::findScript(const std::string &name) const
{
	if(name != "(empty)")
	{
		ScriptList::iterator it = data->scripts.begin();
		for(; it != data->scripts.end(); ++it)
		{
			if(it->name == name)
				return it->script;
		}
	}

	static string empty;
	return empty;
}

const std::string &ExplosionScripts::findProjectile(const std::string &name) const
{
	if(name != "(empty)")
	{
		ProjectileList::iterator it = data->projectiles.begin();
		for(; it != data->projectiles.end(); ++it)
		{
			if(it->name == name)
				return it->projectile;
		}
	}

	static string empty;
	return empty;
}

const std::string &ExplosionScripts::findEffect(const std::string &name) const
{
	if(name != "(empty)")
	{
		EffectList::iterator it = data->effects.begin();
		for(; it != data->effects.end(); ++it)
		{
			if(it->name == name)
				return it->effect;
		}
	}

	static string empty;
	return empty;
}

std::vector<std::string> ExplosionScripts::findSounds(const std::string &name) const
{
	if(name != "(empty)")
	{
		SoundList::iterator it = data->sounds.begin();
		for(; it != data->sounds.end(); ++it)
		{
			if(it->name == name)
				return it->sounds;
		}
	}

	vector<string> empty;
	return empty;
}

const std::string &ExplosionScripts::findMaterial(const std::string &name) const
{
	if(name != "(empty)")
	{
		MaterialList::iterator it = data->materials.begin();
		for(; it != data->materials.end(); ++it)
		{
			if(it->name == name)
				return it->material;
		}
	}

	static string empty;
	return empty;
}

const Animation &ExplosionScripts::findAnimation(const std::string &name) const
{
	if(name != "(empty)")
	{
		AnimationList::iterator it = data->animations.begin();
		for(; it != data->animations.end(); ++it)
		{
			if(it->name == name)
				return *it;
		}
	}

	static Animation empty;
	return empty;
}

void ExplosionScripts::reload()
{
	data->reload();
}

} // editor
} // frozenbyte
