// Copyright 2002-2004 Frozenbyte Ltd.

#include "mapped_object.h"
#include "../util/buildingmap.h"

namespace frozenbyte {
namespace editor {

struct MappedObjectData
{
	boost::scoped_ptr<BuildingMap> building;
};

MappedObject::MappedObject(const std::string &fileName, IStorm3D_Model &model)
{
	boost::scoped_ptr<MappedObjectData> tempData(new MappedObjectData());
	boost::scoped_ptr<BuildingMap> tempBuilding(new BuildingMap(fileName.c_str(), &model));

	data.swap(tempData);
	data->building.swap(tempBuilding);
}

MappedObject::~MappedObject()
{
}

} // end of namespace editor
} // end of namespace frozenbyte
