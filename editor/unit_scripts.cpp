// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "unit_scripts.h"
#include "string_conversions.h"
#include "parser.h"
#include "../filesystem/input_file_stream.h"
#include "../filesystem/file_package_manager.h"
#include <istorm3d_model.h>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <map>
#include <fstream>

#include "../system/Logger.h"

namespace frozenbyte {
namespace editor {
namespace {
#ifdef LEGACY_FILES
	const char *unitName = "Editor\\Units.fbpt";
	const char *scriptName = "Editor\\Scripts.fbt";
	const char *configName = "Editor\\Configurations.fbt";
#else
	const char *unitName = "editor/units.fbpt";
	const char *scriptName = "editor/scripts.fbpt";
	const char *configName = "editor/configurations.fbpt";
#endif

	void parseBlock(const ParserGroup &parserGroup, Unit &unit);
	void parseBlock(const ParserGroup &parserGroup, ScriptConfig &config);
	void parseBlock(const ParserGroup &parserGroup, Script &script);
	void parseBlock(const ParserGroup &parserGroup, ScriptHelper &helper);

	template<class T>
	void parseBlocks(const ParserGroup &parserGroup, std::vector<T> &results, const std::string &prefix)
	{
		std::string groupName;
		std::string numString;
		int index = 1;

		for(;;)
		{
			// no longer read in 1-999 format, now in 001-999 format -jpk
			numString = convertToString<int> (index++);

			groupName = prefix + numString;
			if (!parserGroup.hasSubGroup(groupName))
			{
				if (numString.length() == 1)
					numString = "00" + numString;
				else if (numString.length() == 2)
					numString = "0" + numString;
			}

			groupName = prefix + numString;
			const ParserGroup &subGroup = parserGroup.getSubGroup(groupName);

			T result;
			parseBlock(subGroup, result);

			if(!result.name.empty())
			{
				results.push_back(result);
			} else {
				Logger::getInstance()->debug("unit_scripts - end of parse block.");
				Logger::getInstance()->debug(numString.c_str());
				break;
			}
		}

		std::sort(results.begin(), results.end());
	}

	void parseBlock(const ParserGroup &parserGroup, Unit &unit)
	{
		unit.name = parserGroup.getValue("name");
		unit.name += parserGroup.getValue("name_extend1");
		unit.name += parserGroup.getValue("name_extend2");
		unit.type = parserGroup.getValue("type");
		unit.model = parserGroup.getValue("model");
		unit.bones = parserGroup.getValue("bones");
		unit.weapon_model = parserGroup.getValue("weapon_model");
		unit.defaultConfiguration = parserGroup.getValue("default_configuration");
		unit.defaultSide = convertFromString<int> (parserGroup.getValue("default_side"), 0);
		unit.disableRotation = convertFromString<bool> (parserGroup.getValue("disable_rotation"), 0);

		unit.primaryGroup = parserGroup.getValue("primary_group");
		unit.secondaryGroup = parserGroup.getValue("secondary_group");


		// on some special cases, there will be no script
		// (items, decors, etc.)
		unit.noScript = false;

		const ParserGroup &spawnGroup = parserGroup.getSubGroup("Spawn");
		for(int i = 0; i < spawnGroup.getLineCount(); ++i)
			unit.spawnText.push_back(spawnGroup.getLine(i));

		const ParserGroup &initialization = parserGroup.getSubGroup("Initialize");
		for(int j = 0; j < initialization.getLineCount(); ++j)
		{
			if (initialization.getLine(j) == "<DUMMY>")
			{
				unit.noScript = true;
			} else {
				unit.initialization.push_back(initialization.getLine(j));
			}
		}

		const ParserGroup &propertiesGroup = parserGroup.getSubGroup("StringProperties");
		for(int i = 0; i < propertiesGroup.getLineCount(); ++i)
			unit.stringProperties.addProperty(propertiesGroup.getLine(i));

		for(int i = 0; i < propertiesGroup.getValueAmount(); ++i)
		{
			const std::string &key = propertiesGroup.getValueKey(i);
			std::string value = propertiesGroup.getValue(key);

			unit.stringProperties.addProperty(key, value);
		}
	}

	void parseBlock(const ParserGroup &parserGroup, ScriptConfig &config)
	{
		config.name = parserGroup.getValue("name");
		config.type = parserGroup.getValue("type");

		config.mainScript = parserGroup.getValue("main");
		config.spottedScript = parserGroup.getValue("spotted");
		config.alertedScript = parserGroup.getValue("alerted");
		config.hitScript = parserGroup.getValue("hit");
		config.hitmissScript = parserGroup.getValue("hitmiss");
		config.noiseScript = parserGroup.getValue("noise");
		config.executeScript = parserGroup.getValue("execute");
		config.specialScript = parserGroup.getValue("special");
		config.pointedScript = parserGroup.getValue("pointed");
	}

	void parseBlock(const ParserGroup &parserGroup, Script &script)
	{
		script.name = parserGroup.getValue("name");

		const ParserGroup &scriptGroup = parserGroup.getSubGroup("Script");
		for(int i = 0; i < scriptGroup.getLineCount(); ++i)
			script.script.push_back(scriptGroup.getLine(i));

		/*
		const ParserGroup &propertiesGroup = parserGroup.getSubGroup("StringProperties");
		for(i = 0; i < propertiesGroup.getLineCount(); ++i)
			script.stringProperties.push_back(propertiesGroup.getLine(i));
		*/
		const ParserGroup &propertiesGroup = parserGroup.getSubGroup("StringProperties");
		for(int i = 0; i < propertiesGroup.getLineCount(); ++i)
			script.stringProperties.addProperty(propertiesGroup.getLine(i));

		for(int i = 0; i < propertiesGroup.getValueAmount(); ++i)
		{
			const std::string &key = propertiesGroup.getValueKey(i);
			std::string value = propertiesGroup.getValue(key);

			script.stringProperties.addProperty(key, value);
		}

		parseBlocks(parserGroup, script.helpers, "Helper");
	}

	void parseBlock(const ParserGroup &parserGroup, ScriptHelper &helper)
	{
		helper.name = parserGroup.getValue("name");
		const std::string &typeString = parserGroup.getValue("type");

		if(typeString == "Path")
			helper.type = ScriptHelper::Path;
		else
			helper.type = ScriptHelper::Single;

		helper.exportName = parserGroup.getValue("export_name");
	}

	std::string getTypeString(UnitScripts::ScriptType type)
	{
		if(type == UnitScripts::Main)
			return "Main";
		else if(type == UnitScripts::Alerted)
			return "Alerted";
		else if(type == UnitScripts::Spotted)
			return "Spotted";
		else if(type == UnitScripts::Hit)
			return "Hit";
		else if(type == UnitScripts::Hitmiss)
			return "Hitmiss";
		else if(type == UnitScripts::Noise)
			return "Noise";
		else if(type == UnitScripts::Execute)
			return "Execute";
		else if(type == UnitScripts::Special)
			return "Special";
		else if(type == UnitScripts::Pointed)
			return "Pointed";
		else
		{
			assert(!"Whoops");
			return "";
		}
	}

	struct TypeFinder: public std::unary_function<std::string, bool>
	{
		const std::string &ref;

		explicit TypeFinder(const std::string &ref_)
		:	ref(ref_)
		{
		}

		bool operator() (const std::string &type) const
		{
			if(type.empty())
				return false;

			return ref == type;
		}
	};

} // unnamed

bool operator < (const Unit &lhs, const Unit &rhs)
{
	return lhs.name < rhs.name;
}

bool operator < (const ScriptConfig &lhs, const ScriptConfig &rhs)
{
	return lhs.name < rhs.name;
}

bool operator < (const Script &lhs, const Script &rhs)
{
	return lhs.name < rhs.name;
}

bool operator < (const ScriptHelper &lhs, const ScriptHelper &rhs)
{
	return lhs.name < rhs.name;
}

struct UnitScriptsData
{
	std::vector<Unit> units;
	std::vector<ScriptConfig> scriptConfigs;
	std::map<std::string, std::vector<Script> > scripts;

	void parseUnits(const Parser &parser)
	{
		parseBlocks(parser.getGlobals(), units, "Unit");
	}

	void parseConfigs(const Parser &parser)
	{
		parseBlocks(parser.getGlobals(), scriptConfigs, "Config");
	}

	void parseScripts(const Parser &parser)
	{
		const ParserGroup &parserGroup = parser.getGlobals();

		parseBlocks(parserGroup.getSubGroup("Main"), scripts["Main"], "Script");
		parseBlocks(parserGroup.getSubGroup("Spotted"), scripts["Spotted"], "Script");
		parseBlocks(parserGroup.getSubGroup("Alerted"), scripts["Alerted"], "Script");
		parseBlocks(parserGroup.getSubGroup("Hit"), scripts["Hit"], "Script");
		parseBlocks(parserGroup.getSubGroup("Hitmiss"), scripts["Hitmiss"], "Script");
		parseBlocks(parserGroup.getSubGroup("Noise"), scripts["Noise"], "Script");
		parseBlocks(parserGroup.getSubGroup("Execute"), scripts["Execute"], "Script");
		parseBlocks(parserGroup.getSubGroup("Special"), scripts["Special"], "Script");
		parseBlocks(parserGroup.getSubGroup("Pointed"), scripts["Pointed"], "Script");
	}

	void reload()
	{
		units.clear();
		scripts.clear();
		scriptConfigs.clear();

		Parser unitParser(true, false);
		Parser configParser(true, false);
		Parser scriptParser(true, false);

		//std::ifstream(unitName) >> unitParser;
		//std::ifstream(configName) >> configParser;
		//std::ifstream(scriptName) >> scriptParser;
		filesystem::FilePackageManager::getInstance().getFile(unitName) >> unitParser;
		filesystem::FilePackageManager::getInstance().getFile(configName) >> configParser;
		filesystem::FilePackageManager::getInstance().getFile(scriptName) >> scriptParser;

		parseUnits(unitParser);
		parseConfigs(configParser);
		parseScripts(scriptParser);
	}
};

UnitScripts::UnitScripts()
{
	boost::scoped_ptr<UnitScriptsData> tempData(new UnitScriptsData());
	data.swap(tempData);
}

UnitScripts::~UnitScripts()
{
}

void UnitScripts::reload()
{
	data->reload();
}

int UnitScripts::getUnitCount() const
{
	return data->units.size();
}

int UnitScripts::getUnitIndex(const std::string &name) const
{
	for(unsigned int i = 0; i < data->units.size(); ++i)
	{
		if(data->units[i].name == name)
			return i;
	}

	return -1;
}

const Unit &UnitScripts::getUnit(int index) const
{
	return data->units[index];
}

int UnitScripts::getScriptConfigCount() const
{
	return data->scriptConfigs.size();
}

const ScriptConfig &UnitScripts::getScriptConfig(int index) const
{
	return data->scriptConfigs[index];
}

int UnitScripts::getScriptCount(ScriptType type) const
{
	return data->scripts[getTypeString(type)].size();
}

const Script &UnitScripts::getScript(ScriptType type, int index) const
{
	return data->scripts[getTypeString(type)][index];
}

const Script *UnitScripts::findScript(ScriptType type, const std::string &name) const
{
	const std::vector<Script> &scripts = data->scripts[getTypeString(type)];
	for(unsigned int i = 0; i < scripts.size(); ++i)
	{
		if(name == scripts[i].name)
			return &scripts[i];
	}

	return 0;
}

std::vector<std::string> UnitScripts::getTypes() const
{
	// Find all different types
	
	std::vector<std::string> result;

	const std::vector<Unit> &units = data->units;
	for(unsigned int i = 0; i < units.size(); ++i)
	{
		const std::string &type = units[i].type;
		if(!units[i].type.empty() && std::find(result.begin(), result.end(), type) == result.end())
			result.push_back(type);
	}

	const std::vector<ScriptConfig> &configs = data->scriptConfigs;
	for(unsigned int j = 0; j < configs.size(); ++j)
	{
		const std::string &type = configs[j].type;
		if(!configs[j].type.empty() && std::find(result.begin(), result.end(), type) == result.end())
			result.push_back(type);
	}

	std::sort(result.begin(), result.end());
	return result;
}

} // end of namespace editor
} // end of namespace frozenbyte
