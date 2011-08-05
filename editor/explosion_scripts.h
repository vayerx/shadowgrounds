// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_UNIT_SCRIPTS_H
#define INCLUDED_EDITOR_UNIT_SCRIPTS_H

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif
#ifndef INCLUDED_STRING
#define INCLUDED_STRING
#include <string>
#endif
#ifndef INCLUDED_VECTOR
#define INCLUDED_VECTOR
#include <vector>
#endif

namespace frozenbyte {
namespace editor {

struct ExplosionScript
{
	std::string name;
	std::string script;
};

struct ExplosionProjectile
{
	std::string name;
	std::string projectile;
};

struct ExplosionEffect
{
	std::string name;
	std::string effect;
};

struct ExplosionSound
{
	std::string name;
	std::vector<std::string> sounds;
};

struct Material
{
	std::string name;
	std::string material;
};

struct Animation
{
	std::string name;
	std::string bones;
	std::string idleAnimation;

	std::vector<std::string> animations;
};

typedef std::vector<ExplosionScript> ScriptList;
typedef std::vector<ExplosionProjectile> ProjectileList;
typedef std::vector<ExplosionEffect> EffectList;
typedef std::vector<ExplosionSound> SoundList;
typedef std::vector<Material> MaterialList;
typedef std::vector<Animation> AnimationList;

class ExplosionScripts
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	ExplosionScripts();
	~ExplosionScripts();

	const ScriptList &getScripts() const;
	const ProjectileList &getProjectiles() const;
	const EffectList &getEffects() const;
	const SoundList &getSounds() const;
	const MaterialList &getMaterials() const;
	const AnimationList &getAnimations() const;

	const std::string &findScript(const std::string &name) const;
	const std::string &findProjectile(const std::string &name) const;
	const std::string &findEffect(const std::string &name) const;
	std::vector<std::string> findSounds(const std::string &name) const;
	const std::string &findMaterial(const std::string &name) const;
	const Animation &findAnimation(const std::string &name) const;

	void reload();
};

/*
bool operator < (const ExplosionScript &lhs, const ExplosionScript &rhs);
bool operator < (const ExplosionProjectile &lhs, const ExplosionProjectile &rhs);
bool operator < (const ExplosionEffect &lhs, const ExplosionEffect &rhs);
bool operator < (const ExplosionSound &lhs, const ExplosionSound &rhs);
bool operator < (const ExplosionSound &lhs, const ExplosionSound &rhs);
*/

} // end of namespace editor
} // end of namespace frozenbyte

#endif
