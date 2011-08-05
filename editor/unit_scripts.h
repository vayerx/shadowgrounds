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

#include "string_properties.h"

namespace frozenbyte {
namespace editor {

struct Unit
{
	std::string name;
	std::string type;

	std::vector<std::string> spawnText;
	std::vector<std::string> initialization;

	std::string model;
	std::string bones;
	std::string weapon_model;

	bool noScript;
	StringProperties stringProperties;

	int defaultSide;
	bool disableRotation;
	std::string defaultConfiguration;

	std::string primaryGroup;
	std::string secondaryGroup;

	Unit()
	:	noScript(false),
		defaultSide(0),
		disableRotation(false)
	{
	}
};

struct ScriptHelper
{
	enum Type 
	{
		Single = 0,
		Path = 1
	};

	std::string name;
	Type type;

	std::string exportName;

	ScriptHelper()
	:	type(Single)
	{
	}
};

struct Script
{
	std::string name;

	std::vector<std::string> script;
	std::vector<ScriptHelper> helpers;

	StringProperties stringProperties;
};

struct ScriptConfig
{
	std::string name;
	std::string type;

	std::string mainScript;
	std::string spottedScript;
	std::string alertedScript;
	std::string hitScript;
	std::string hitmissScript;
	std::string noiseScript;
	std::string executeScript;
	std::string specialScript;
	std::string pointedScript;
};

struct UnitScriptsData;

class UnitScripts
{
	boost::scoped_ptr<UnitScriptsData> data;

public:
	UnitScripts();
	~UnitScripts();

	void reload();

	int getUnitCount() const;
	int getUnitIndex(const std::string &name) const;
	const Unit &getUnit(int index) const;

	int getScriptConfigCount() const;
	const ScriptConfig &getScriptConfig(int index) const;

	enum ScriptType
	{
		Main = 0,
		Spotted = 1,
		Alerted = 2,
		Hit = 3,
		Hitmiss = 4,
		Noise = 5,
		Execute = 6,
		Special = 7,
		Pointed = 8
	};

	int getScriptCount(ScriptType type) const;
	const Script &getScript(ScriptType type, int index) const;

	const Script *findScript(ScriptType type, const std::string &name) const;
	std::vector<std::string> getTypes() const;
};

bool operator < (const Unit &lhs, const Unit &rhs);
bool operator < (const ScriptConfig &lhs, const ScriptConfig &rhs);
bool operator < (const Script &lhs, const Script &rhs);
bool operator < (const ScriptHelper &lhs, const ScriptHelper &rhs);

} // end of namespace editor
} // end of namespace frozenbyte

#endif
