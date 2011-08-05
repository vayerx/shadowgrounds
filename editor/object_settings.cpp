// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "object_settings.h"
#include "storm_geometry.h"
#include "parser.h"
#include "string_conversions.h"
#include "../filesystem/input_file_stream.h"
#include "../filesystem/file_package_manager.h"
#include "common_dialog.h"
#include "file_wrapper.h"
#include "storm.h"
#include <istorm3d_model.h>
#include <istorm3d.h>
#include "../system/Logger.h"

#include <fstream>
#include <map>
#include <vector>
#include <io.h>

namespace frozenbyte {
namespace editor {
namespace {
	std::string fileName = "Editor\\Objects.fbt";
}

struct ObjectSettingsData
{
	typedef std::map<std::string, ObjectData> DataMap;
	DataMap objects;

	typedef std::map<std::string, std::string> ValueMap;
	ValueMap defaultValues;

	Parser parser;
	Storm &storm;

	ObjectSettingsData(Storm &storm_)
	:	parser(false, false),
		storm(storm_)
	{
		defaultValues["trackable_type"] = "";
		defaultValues["tracker_bullet"] = "";
		defaultValues["burning_tracker_script"] = "";
		defaultValues["explosion_bullet"] = "";
		defaultValues["filename_prefix"] = "";
		defaultValues["filename_postfix"] = "";
		defaultValues["custom_break_object"] = "";
		defaultValues["heatradius"] = "";
		defaultValues["burned_break_bullet"] = "";
		defaultValues["ambient_sound"] = "";
		defaultValues["ambient_sound_range"] = "";
		defaultValues["ambient_sound_rolloff"] = "";
	}

	void loadObjects()
	{
		filesystem::InputStream strm = filesystem::FilePackageManager::getInstance().getFile(fileName);
		strm >> parser;
	}

	void saveObjects()
	{
		std::ofstream stream(fileName.c_str());

		exportToStream();
		objects.clear();

		stream << parser;
	}

	void exportToStream()
	{
		for(DataMap::iterator it = objects.begin(); it != objects.end(); ++it)
		{
			assert(!it->first.empty());

			ParserGroup &group = parser.getGlobals().getSubGroup(it->first);
			ObjectData &data = it->second;

			group.setValue("type", convertToString<int>(data.type));
			group.setValue("fall_type", convertToString<int>(data.fallType));
			// useless constantly changing (why!?!) and conflicting crap.
			//group.setValue("x_size", convertToString<float>(data.radiusX));
			//group.setValue("z_size", convertToString<float>(data.radiusZ));
			//group.setValue("height", convertToString<float>(data.height));
			if (data.originalCrapRadiusX != 0.0f)
				group.setValue("x_size", convertToString<float>(data.originalCrapRadiusX));
			if (data.originalCrapRadiusZ != 0.0f)
				group.setValue("z_size", convertToString<float>(data.originalCrapRadiusZ));
			if (data.originalCrapHeight != 0.0f)
				group.setValue("height", convertToString<float>(data.originalCrapHeight));
			group.setValue("fireThrough", convertToString<bool>(data.fireThrough));
			group.setValue("explosion_object", data.explosionObject);
			group.setValue("explosion_effect", data.explosionEffect);
			group.setValue("explosion_projectile", data.explosionProjectile);
			group.setValue("explosion_script", data.explosionScript);
			group.setValue("explosion_sound", data.explosionSound);
			group.setValue("material", data.material);
			group.setValue("animation", data.animation);
			group.setValue("hitpoints", convertToString<int>(data.hitpoints));
			group.setValue("break_texture", convertToString<int>(data.breakTexture));
			group.setValue("physics_type", convertToString<int>(data.physicsType));
			group.setValue("physics_weight", data.physicsWeight);
			group.setValue("physics_material", data.physicsMaterial);
			group.setValue("durability_type", data.durabilityType);

			{
				ParserGroup &metaGroup = group.getSubGroup("MetaValues");

				for(std::map<std::string, std::string>::iterator it = data.metaValues.begin(); it != data.metaValues.end(); ++it)
					metaGroup.setValue(it->first, it->second);
			}
		}
	}

	std::string findNewName(const std::string &fileName) const
	{
		const ParserGroup &group = parser.getGlobals();
		if(group.hasSubGroup(fileName))
			return fileName;

		std::string localFile = getFileName(fileName);
		std::string result = fileName;

		for(int i = 0; i < group.getSubGroupAmount(); ++i)
		{
			std::string newName = group.getSubGroupName(i);
			if(localFile == getFileName(newName))
			{
				if(fileName == newName)
					return fileName;

				result = newName;
			}
		}

		return result;
	}

	void fromStream(const std::string &originalFileName)
	{
		// Test filename - we migh need to take properties from different place if file has moved
		std::string fileName = findNewName(originalFileName);

		const ParserGroup &group = parser.getGlobals().getSubGroup(fileName);
		ObjectData &data = objects[originalFileName];

		data.type = convertFromString<int> (group.getValue("type"), -1);
		data.fallType = convertFromString<int> (group.getValue("fall_type"), 0);
		//data.height = convertFromString<float> (group.getValue("height"), 0);
		data.originalCrapHeight = convertFromString<float> (group.getValue("height"), 0);
		data.fireThrough = convertFromString<bool>(group.getValue("fireThrough"), false);
		data.explosionObject = group.getValue("explosion_object", "");
		data.explosionEffect = group.getValue("explosion_effect", "");
		data.explosionProjectile = group.getValue("explosion_projectile", "");
		data.explosionScript = group.getValue("explosion_script", "");
		data.explosionSound = group.getValue("explosion_sound", "");
		data.material = group.getValue("material", "");
		data.animation = group.getValue("animation", "");
		data.hitpoints = convertFromString<int> (group.getValue("hitpoints"), 5);
		data.breakTexture = convertFromString<int> (group.getValue("break_texture"), 0);
		data.physicsType = convertFromString<int> (group.getValue("physics_type"), 0);
		data.physicsWeight = group.getValue("physics_weight");
		data.physicsMaterial = group.getValue("physics_material");
		data.durabilityType = group.getValue("durability_type");

		{
			const ParserGroup &metaGroup = group.getSubGroup("MetaValues");

			for(int i = 0; i < metaGroup.getValueAmount(); ++i)
			{
				std::string key = metaGroup.getValueKey(i);
				std::string value = metaGroup.getValue(key);

				data.metaValues[key] = value;
			}
		}

#ifdef LEGACY_FILES
		if(!data.explosionObject.empty() && data.explosionObject != "(disappear)" && !fileExists(data.explosionObject))
			data.explosionObject = FileWrapper::resolveModelName("Data\\Models\\Terrain_objects", data.explosionObject);
#else
		// FIXME: file does not necessarily exist if it has @ postfix!!!
		if(!data.explosionObject.empty() && data.explosionObject != "(disappear)" && !fileExists(data.explosionObject))
			data.explosionObject = FileWrapper::resolveModelName("data\\model\\object", data.explosionObject);
#endif

		// Backward compatibility
		//data.radiusX = data.radiusZ = convertFromString<float> (group.getValue("radius"), 0);

		//float xFoo = convertFromString<float> (group.getValue("x_size"), 0);
		//float zFoo = convertFromString<float> (group.getValue("z_size"), 0);

		//if(xFoo > 0)
		//	data.radiusX = xFoo;
		//if(zFoo > 0)
		//	data.radiusZ = zFoo;
		data.originalCrapRadiusX = convertFromString<float> (group.getValue("x_size"), 0);
		data.originalCrapRadiusZ = convertFromString<float> (group.getValue("z_size"), 0);


		{
			IStorm3D_Model *model = storm.storm->CreateNewModel();

			std::string modelFileName = fileName;
			std::string postfix = data.metaValues["filename_postfix"];
			if(!postfix.empty())
			{
				modelFileName += postfix;
			}

			//Logger::getInstance()->info(modelFileName.c_str());
			model->LoadS3D(modelFileName.c_str());

			VC3 size = getSize(model);
			delete model;

			data.radiusX = size.x;
			data.height = size.y;
			data.radiusZ = size.z;
		}
	}
};

ObjectSettings::ObjectSettings(Storm &storm)
{
	boost::scoped_ptr<ObjectSettingsData> tempData(new ObjectSettingsData(storm));
	tempData->loadObjects();
	data.swap(tempData);
}

ObjectSettings::~ObjectSettings()
{
	saveSettings();
}

void ObjectSettings::saveSettings()
{
	data->saveObjects();
}

std::vector<std::string> ObjectSettings::getMetaKeys() const
{
	std::vector<std::string> list;

	for(ObjectSettingsData::ValueMap::iterator it = data->defaultValues.begin(); it != data->defaultValues.end(); ++it)
		list.push_back(it->first);

	return list;
}

ObjectData &ObjectSettings::getSettings(const std::string &fileName)
{
	if(data->objects.find(fileName) == data->objects.end())
		data->fromStream(fileName);

	return data->objects[fileName];
}

const ObjectData &ObjectSettings::getSettings(const std::string &fileName) const
{
	ObjectSettingsData::DataMap::const_iterator it = data->objects.find(fileName);
	if(it != data->objects.end())
		return (*it).second;

	static ObjectData empty;

#ifdef PROJECT_AOV
	empty.metaValues["filename_postfix"] = "@SI";
#endif

	return empty;
}

} // end of namespace editor
} // end of namespace frozenbyte
