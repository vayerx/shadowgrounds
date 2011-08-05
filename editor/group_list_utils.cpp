// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "group_list_utils.h"
#include "align_units.h"
#include "object_settings.h"
#include "storm.h"
#include <istorm3d.h>

using namespace boost;

namespace frozenbyte {
namespace editor {

ModelGroup::ModelGroup()
:	rotationEul(0,0,0),
heightOffset(0.0f)
{
}

VC3 ModelGroup::getPosition(Storm &storm, int index) const
{
	return models[index]->GetPosition();
}

VC3 ModelGroup::getRotation(Storm &storm, int index) const
{
	assert(index >= 0 && index < int(objectGroup.instances.size()));
	VC3 result = objectGroup.instances[index].rotation;

	result.y += rotationEul.y;

	// oops. should rotate around the group tool pivot. damn...
	//QUAT q = frozenbyte::editor::getRotation(rotationEul);
	//QUAT qobj = frozenbyte::editor::getRotation(result);
	//qobj = qobj * q;
	//result = getEulerAngles(qobj);

	return result;
}

void ModelGroup::create(Storm &storm, const GroupList::ObjectGroup &group, ObjectSettings &objectSettings)
{
	models.clear();
	objectGroup = group;

	for(int i = 0; i < int(group.instances.size()); ++i)
	{
		const GroupList::Instance &instance = group.instances[i];

		std::string loadFileName = instance.model;
		ObjectData &objectDataTmp = objectSettings.getSettings(loadFileName);
		std::string postfix = objectDataTmp.metaValues["filename_postfix"];
		if(!postfix.empty())
		{
			loadFileName += postfix;
		}

		shared_ptr<IStorm3D_Model> model(storm.storm->CreateNewModel());
		model->LoadS3D(loadFileName.c_str());

		models.push_back(model);
	}
}

void ModelGroup::update(Storm &storm, const VC3 &position)
{
	QUAT quat;
	//quat.MakeFromAngles(0, rotation, 0);
	quat = frozenbyte::editor::getRotation(rotationEul);
	MAT tm;
	tm.CreateRotationMatrix(quat);

	for(int i = 0; i < int(models.size()); ++i)
	{
		if(!models[i])
			continue;

		const GroupList::Instance &instance = objectGroup.instances[i];

		VC3 pos = instance.position;
		tm.RotateVector(pos);
		pos += position;
		pos.y = storm.getHeight(VC2(pos.x, pos.z)) + heightOffset;
		pos.y += objectGroup.instances[i].position.y;
		models[i]->SetPosition(pos);

		VC3 angles = getRotation(storm, i);
		quat = frozenbyte::editor::getRotation(angles);

		//QUAT q;
		//q.MakeFromAngles(0, rotation, 0);
		//quat = quat * q;

		models[i]->SetRotation(quat);
	}
}

} // editor
} // frozenbyte
