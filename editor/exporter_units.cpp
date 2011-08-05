// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "exporter_units.h"
#include "export_options.h"
#include "unit_properties.h"
#include "string_properties.h"
#include "string_replace.h"
#include <fstream>

namespace frozenbyte {
namespace editor {
namespace {
	struct Unit
	{
		VC2 position;
		float yRotation;
		float height;

		std::vector<std::string> spawnText;
		std::string script;

		UnitProperties properties;
		StringProperties stringProperties;

		Unit()
		:	yRotation(0),
			height(0)
		{
		}
	};
} // unnamed

struct ExporterUnitsData
{
	std::vector<Unit> hostileUnits;
	std::vector<Unit> neutralUnits;
	std::vector<Unit> allyUnits;

	// Units saved to special files
	std::map<std::string, std::vector<Unit> > fileUnits;

	VC2I spawnPosition;

	void exportUnits(std::ofstream &stream, const std::vector<Unit> &units, const std::string &id)
	{
		for(unsigned int i = 0; i < units.size(); ++i)
		{
			stream << std::endl;
			const Unit &unit = units[i];

			int rotation = int((unit.yRotation / PI * 180.f) + 0.5f);
			while(rotation < 0)
				rotation += 360;

			rotation %= 360;
			bool addParamsAdded = false;

			int difficulty = unit.properties.difficulty;
			if(difficulty != UnitProperties::All)
			{
				stream << "   if" << std::endl;
				stream << "    getVariable hostile_amount_level" << std::endl;
				
				std::string value;
				if(difficulty == UnitProperties::EasyOnly)
					value = "valueLessThan 50";
				else if(difficulty == UnitProperties::HardOnly)
					value = "valueGreaterThan 74";

				stream << "    " << value << std::endl;
				stream << "   then" << std::endl;
			}

			int layout = unit.properties.layout;
			if(layout > 0)
			{
				stream << "   if" << std::endl;
				if(layout >= 1 && layout <= 2)
					stream << "    getVariable random_layout_a" << std::endl;
				else 
					stream << "    getVariable random_layout_b" << std::endl;
				
				if(layout == 1 || layout == 3)
					stream << "   notValue" << std::endl;

				stream << "   then" << std::endl;
			}

			for(unsigned int j = 0; j < unit.spawnText.size(); ++j)
			{
				bool printLineUnconverted = true;
				bool convertToParams = false;
				bool convertToItemParams = false;
				bool convertToLightParams = false;
				bool convertToDecorParams = false;
				bool convertToWaterParams = false;
				bool convertToParticleParams = false;
				bool convertToMapParams = false;

				if (strcmp(unit.spawnText[j].c_str(), "<UNIT_ADD_PARAMETERS>") == 0)
				{
					convertToParams = true;
					printLineUnconverted = false;
				}
				if (strcmp(unit.spawnText[j].c_str(), "<ITEM_ADD_PARAMETERS>") == 0)
				{
					convertToItemParams = true;
					printLineUnconverted = false;
				}
				if (strcmp(unit.spawnText[j].c_str(), "<DECOR_ADD_PARAMETERS>") == 0)
				{
					convertToDecorParams = true;
					printLineUnconverted = false;
				}
				if (strcmp(unit.spawnText[j].c_str(), "<WATER_ADD_PARAMETERS>") == 0)
				{
					convertToWaterParams = true;
					printLineUnconverted = false;
				}
				if (strcmp(unit.spawnText[j].c_str(), "<LIGHT_ADD_PARAMETERS>") == 0)
				{
					convertToLightParams = true;
					printLineUnconverted = false;
				}
				if (strcmp(unit.spawnText[j].c_str(), "<PARTICLE_ADD_PARAMETERS>") == 0)
				{
					convertToParticleParams = true;
					printLineUnconverted = false;
				}
				if(unit.spawnText[j] == "<MAP_ADD_PARAMETERS>")
				{
					convertToMapParams = true;
					printLineUnconverted = false;
				}

				// ToDo: string replace
				if (printLineUnconverted)
					stream << "   " << replaceString(unit.spawnText[j], unit.properties.strings, unit.stringProperties.defaults) << std::endl;
				
				if (convertToItemParams || convertToLightParams
					|| convertToParticleParams)
				{
					addParamsAdded = true;
					stream << "   setPosition " << "s," << unit.position.x << "," << unit.position.y << std::endl;
					stream << "   setPositionHeight " << unit.height << std::endl;
					stream << "   setValue " << rotation << std::endl;
				}
				if (convertToDecorParams)
				{
					addParamsAdded = true;
					stream << "   setDecorPosition " << "s," << unit.position.x << "," << unit.position.y << std::endl;
					stream << "   setDecorHeight " << unit.height << std::endl;
					stream << "   setValue " << rotation << std::endl;
				}
				if (convertToWaterParams)
				{
					addParamsAdded = true;
					stream << "   setWaterPosition " << "s," << unit.position.x << "," << unit.position.y << std::endl;
					stream << "   setWaterHeight " << unit.height << std::endl;
					stream << "   setValue " << rotation << std::endl;
				}
				if (convertToMapParams)
				{
					addParamsAdded = true;
					std::string id;

					StringMap::const_iterator idIterator = unit.properties.strings.find("map_layer_id");
					if(idIterator != unit.properties.strings.end())
						id = idIterator->second;
					if(id.empty())
						id = unit.stringProperties.getDefault("map_layer_id");
					if(!id.empty())
						stream << "   setMapLayerId " << id << std::endl;

					stream << "   setPosition " << "s," << unit.position.x << "," << unit.position.y << std::endl;
				}
				if (convertToParams
					|| (j == unit.spawnText.size() - 1 && !addParamsAdded))
				{
					addParamsAdded = true;

					StringMap::const_iterator idIterator = unit.properties.strings.find("id");
					if(idIterator != unit.properties.strings.end())
					{
						const std::string &id = idIterator->second;

						if(!id.empty())
							stream << "   setUnitIdString " << id << std::endl;
					}

					stream << "   setUnitSpawnCoordinates " << "s," << unit.position.x << "," << unit.position.y << std::endl;
					stream << "   setUnitAngle " << rotation << std::endl;
					//stream << "   setUnitHeight " << unit.height << std::endl;
					stream << "   setUnitScript " << id << "_" << unit.script << std::endl;					
				}
			}

			if(layout > 0)
				stream << "   endif" << std::endl;

			if(difficulty != UnitProperties::All)
				stream << "   endif" << std::endl;
		}
	}

	void save(const std::string &fileName, const std::string &id)
	{
		std::ofstream stream(fileName.c_str());

		stream << "script " << id << "_units" << std::endl;
		stream << "sub addunits" << std::endl;

		// HC player
		stream << "   // Player" << std::endl;
		stream << "   setPlayer 0" << std::endl;
		stream << "   setHostile 1" << std::endl;
		stream << "   setFriendly 2" << std::endl;
		stream << "   setFriendly 3" << std::endl;
		stream << "   setSpawn " << spawnPosition.x << "," << spawnPosition.y << std::endl;
		stream << std::endl;

		stream << "   // Enemies" << std::endl;
		stream << "   setPlayer 1" << std::endl;
		stream << "   setHostile 0" << std::endl;
		stream << "   setFriendly 2" << std::endl;
		stream << "   setHostile 3" << std::endl;
		exportUnits(stream, hostileUnits, id);
		stream << std::endl;

		stream << "   // Neutral" << std::endl;
		stream << "   setPlayer 2" << std::endl;
		stream << "   setFriendly 0" << std::endl;
		stream << "   setFriendly 1" << std::endl;
		stream << "   setFriendly 3" << std::endl;
		exportUnits(stream, neutralUnits, id);
		stream << std::endl;

		stream << "   // Ally" << std::endl;
		stream << "   setPlayer 3" << std::endl;
		stream << "   setFriendly 0" << std::endl;
		stream << "   setHostile 1" << std::endl;
		stream << "   setFriendly 2" << std::endl;
		exportUnits(stream, allyUnits, id);

		stream << "endSub" << std::endl;
		stream << "endScript" << std::endl;
	}
};

ExporterUnits::ExporterUnits()
{
	boost::scoped_ptr<ExporterUnitsData> tempData(new ExporterUnitsData());
	data.swap(tempData);
}

ExporterUnits::~ExporterUnits()
{
}

void ExporterUnits::addUnit(const std::vector<std::string> &spawnText, const VC2 &position, float yRotation, float height, const std::string &scriptName, int side, const UnitProperties &properties, const StringProperties &stringProperties)
{
	Unit u;
	u.spawnText = spawnText;
	u.script = scriptName;
	u.position = position;
	u.yRotation = yRotation;
	u.height = height;
	u.properties = properties;
	u.stringProperties = stringProperties;

	// Store it to special file units if has export_file_name defined
	{
		std::string exportFile;
		StringMap::const_iterator it = properties.strings.find("export_file_name");
		if(it != properties.strings.end())
			exportFile = it->second;
		if(exportFile.empty())
			exportFile = stringProperties.getDefault("export_file_name");
		if(!exportFile.empty())
		{
			data->fileUnits[exportFile].push_back(u);
			return;
		}
	}

	std::vector<Unit> *units = 0;
	if(side == 0)
		units = &data->hostileUnits;
	else if(side == 1)
		units = &data->neutralUnits;
	else if(side == 2)
		units = &data->allyUnits;
	else
		assert(!"whoops");

	if(!units)
		return;

	units->resize(units->size() + 1);
	(*units)[units->size() - 1] = u;
	/*
	Unit &unit = (*units)[units->size() - 1];

	unit.spawnText = spawnText;
	unit.script = scriptName;
	unit.position = position;
	unit.yRotation = yRotation;
	unit.height = height;
	unit.properties = properties;
	unit.stringProperties = stringProperties;
	*/
}

void ExporterUnits::setSpawn(const VC2I &position)
{
	data->spawnPosition = position;
}

void ExporterUnits::save(const ExportOptions &options) const
{
	std::string fileName = options.fileName + std::string("\\") + options.id + std::string("_units.dhs");
	data->save(fileName, options.id);

	std::map<std::string, std::vector<Unit> >::iterator it = data->fileUnits.begin();
	for(; it != data->fileUnits.end(); ++it)
	{
		std::string fileName = options.fileName + std::string("\\") + options.id;
		fileName += "_";
		fileName += it->first;
		fileName += ".dhs";

		std::ofstream stream(fileName.c_str());
		std::string script = "script ";
		script += options.id;
		script += "_";
		script += it->first;

		stream << script.c_str() << std::endl;
		stream << "sub hax" << std::endl;

		data->exportUnits(stream, it->second, options.id);	

		stream << "endSub" << std::endl;
		stream << "endScript" << std::endl;
	}
}

} // end of namespace editor
} // end of namespace frozenbyte
