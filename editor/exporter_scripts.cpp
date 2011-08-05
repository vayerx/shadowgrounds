// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "exporter_scripts.h"
#include "export_options.h"
#include "unit_properties.h"
#include "string_properties.h"
#include "string_replace.h"
#include <fstream>
#include <map>

namespace frozenbyte {
namespace editor {
namespace {
	struct SubScript
	{
		std::vector<std::string> script;
	};

	struct Path
	{
		std::vector<VC2> points;
	};

	struct Script
	{
		std::map<std::string, SubScript> subScripts;
		std::map<std::string, Path> paths;

		std::vector<std::string> initialization;
		bool noScript;

		UnitProperties properties;
		StringProperties stringProperties;
	};
} // unnamed

struct ExporterScriptsData
{
	std::vector<Script> scripts;
	std::map<std::string, int> names; // name to index

	std::string getSubString(ExporterScripts::SubType type)
	{
		if(type == ExporterScripts::Main)
			return "main";
		if(type == ExporterScripts::Spotted)
			return "spotted";
		if(type == ExporterScripts::Alerted)
			return "alerted";
		if(type == ExporterScripts::Hit)
			return "hit";
		if(type == ExporterScripts::Hitmiss)
			return "hitmiss";
		if(type == ExporterScripts::Noise)
			return "hearnoise";
		if(type == ExporterScripts::Execute)
			return "execute";
		if(type == ExporterScripts::Special)
			return "special";
		if(type == ExporterScripts::Pointed)
			return "pointed";

		assert(!"Whoops");
		return "";
	}

	std::ofstream &exportPoint(std::ofstream &stream, const VC2 &point)
	{
		stream << "s," << point.x << "," << point.y;
		return stream;
	}

	void exportPaths(std::ofstream &stream, const Script &script)
	{
		std::map<std::string, Path>::const_iterator it = script.paths.begin();
		if(it == script.paths.end())
		{
			stream << std::endl;
			return;
		}

		stream << std::endl << "   storePath 0";
		bool exportSomething = false;

		for(; it != script.paths.end(); ++it)
		{
			const std::vector<VC2> &points = it->second.points;
			if(points.empty())
				continue;

			exportSomething = true;

			stream << std::endl;
			stream << "   pathName " << it->first << std::endl;

			stream << "   " << "pathStart "; 
			exportPoint(stream, points[0]) << std::endl;

			stream << "   storeNextPath" << std::endl;

			for(unsigned int i = 0; i < points.size(); ++i)
			{
				stream << "   " << "pathTo ";
				exportPoint(stream, points[i]) << std::endl;

				stream << "   storeNextPath" << std::endl;
			}

			stream << "   " << "pathEnd ";
			exportPoint(stream, points[points.size() - 1]) << std::endl;
			stream << "   storeNextPath" << std::endl;
		}

		if(!exportSomething)
			stream << std::endl;
	}

	std::ofstream &exportSubScript(std::ofstream &stream, Script &script, const std::string &subName)
	{
		SubScript &subScript = script.subScripts[subName];

		if(subName == "execute"  && subScript.script.empty())
			return stream;

		// HACK: special sub is not actually a sub... -jpk
		if (subName != "special")
		{
			stream << "sub " << subName << std::endl;
		}

		if(subName == "main")
		{
			stream << "   call createpaths" << std::endl;

			for(unsigned int i = 0; i < script.initialization.size(); ++i)
				stream << "   " << replaceString(script.initialization[i], script.properties.strings, script.stringProperties.defaults) << std::endl;
		}

		for(unsigned int i = 0; i < subScript.script.size(); ++i)
			stream << "   " << replaceString(subScript.script[i], script.properties.strings, script.stringProperties.defaults) << std::endl;

		// HACK: special sub is not actually a sub... -jpk
		if (subName != "special")
		{
			stream << "endSub" << std::endl;
		}

		return stream;
	}

	void save(const std::string &fileName, const std::string &id)
	{
		std::ofstream stream(fileName.c_str());

		std::map<std::string, int>::iterator it = names.begin();
		for(; it != names.end(); ++it)
		{
			Script &script = scripts[it->second];

			if (!script.noScript)
			{
				if(it != names.begin())
					stream << std::endl;

				stream << "// ---------------------------------------------------------" << std::endl << std::endl;
				stream << "script " << id << "_" << it->first << std::endl << std::endl;;

				stream << "sub createpaths";
				exportPaths(stream, script);
				stream << "endSub" << std::endl << std::endl;

				// NOTE: special must be written first.
				exportSubScript(stream, script, "special") << std::endl;

				exportSubScript(stream, script, "main") << std::endl;
				exportSubScript(stream, script, "spotted") << std::endl;
				exportSubScript(stream, script, "alerted") << std::endl;
				exportSubScript(stream, script, "hit") << std::endl;
				exportSubScript(stream, script, "hitmiss") << std::endl;
				exportSubScript(stream, script, "hearnoise") << std::endl;
				exportSubScript(stream, script, "execute") << std::endl;
				exportSubScript(stream, script, "pointed") << std::endl;

				stream << "endScript" << std::endl;
			}
		}
	}
};

ExporterScripts::ExporterScripts()
{
	boost::scoped_ptr<ExporterScriptsData> tempData(new ExporterScriptsData());
	data.swap(tempData);
}

ExporterScripts::~ExporterScripts()
{
}

int ExporterScripts::addScript(const std::string &name)
{
	int id = data->scripts.size();
	data->scripts.resize(id + 1);

	Script &script = data->scripts[id];
	data->names[name] = id;

	return id;
}

void ExporterScripts::setScript(int id, SubType type, const std::vector<std::string> &script)
{
	SubScript &subScript = data->scripts[id].subScripts[data->getSubString(type)];
	subScript.script = script;
}

void ExporterScripts::setNoScript(int id, bool noScript)
{
	data->scripts[id].noScript = noScript;
}

void ExporterScripts::setInitialization(int id, const std::vector<std::string> &initialization)
{
	data->scripts[id].initialization = initialization;
}

void ExporterScripts::addPath(int id, const std::string &name, const std::vector<VC2> &points)
{
	Script &script = data->scripts[id];
	Path &path = script.paths[name];

	for(unsigned int i = 0; i < points.size(); ++i)
		path.points.push_back(points[i]);
}

void ExporterScripts::setProperties(int id, const UnitProperties &properties)
{
	Script &script = data->scripts[id];
	script.properties = properties;
}

void ExporterScripts::setProperties(int id, const StringProperties &properties)
{
	Script &script = data->scripts[id];
	script.stringProperties = properties;
}

void ExporterScripts::save(const ExportOptions &options) const
{
	std::string fileName = options.fileName + std::string("\\") + options.id + std::string("_ai.dhs");
	data->save(fileName, options.id);
}

} // end of namespace editor
} // end of namespace frozenbyte
