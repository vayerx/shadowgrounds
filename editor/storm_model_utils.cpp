// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "storm_model_utils.h"
#include <istorm3d.h>
#include <istorm3d_model.h>
#include <istorm3d_bone.h>
#include <istorm3d_mesh.h>
#include <boost/scoped_ptr.hpp>

namespace frozenbyte {
namespace editor {

IStorm3D_Helper_Camera *getBoneHelper(const boost::shared_ptr<IStorm3D_Model> &model, const std::string &name)
{
	boost::scoped_ptr<Iterator<IStorm3D_Helper *> > helperIterator(model->ITHelper->Begin());
	for(; !helperIterator->IsEnd(); helperIterator->Next())
	{
		IStorm3D_Helper *helper = helperIterator->GetCurrent();
		if((!helper) || (helper->GetHelperType() != IStorm3D_Helper::HTYPE_CAMERA))
			continue;

		if(!helper->GetParentBone())
			continue;

		if(name == helper->GetName())
			return static_cast<IStorm3D_Helper_Camera *> (helper);
	}

	return 0;
}

void addCloneModel(const boost::shared_ptr<IStorm3D_Model> &original, boost::shared_ptr<IStorm3D_Model> &clone, const std::string &helperName)
{
	IStorm3D_Helper *helper = getBoneHelper(clone, helperName);

	boost::scoped_ptr<Iterator<IStorm3D_Model_Object *> > objectIterator(original->ITObject->Begin());
	for(; !objectIterator->IsEnd(); objectIterator->Next())
	{
		IStorm3D_Model_Object *object = objectIterator->GetCurrent();
		if(!object)
			continue;

		IStorm3D_Mesh *mesh = object->GetMesh();
		if(!mesh)
			continue;

		IStorm3D_Model_Object *newObject = clone->Object_New(object->GetName());
		newObject->CopyFrom(object, true);
		newObject->SetMesh(mesh);

		if(object->IsLightObject())
			newObject->SetAsLightObject();

		if(helper && helper->GetParentBone())
		{
			Matrix tm = helper->GetTM();
			Vector position = tm.GetTranslation();
			Rotation rotation = tm.GetRotation();

			newObject->SetPosition(position);
			newObject->SetRotation(rotation);

			//VC3 tp = object->GetPosition();
			//QUAT rp = object->GetRotation();
			//newObject->SetPosition(tp);
			//newObject->SetRotation(rp);

			helper->GetParentBone()->AddChild(newObject);
		}
		else
		{
			newObject->SetPosition(object->GetPosition());
			newObject->SetRotation(object->GetRotation());
		}

		newObject->SetNoCollision(object->GetNoCollision());
		newObject->SetNoRender(object->GetNoRender());
	}
}

void addCloneModel(const boost::shared_ptr<IStorm3D_Model> &original, boost::shared_ptr<IStorm3D_Model> &clone)
{
	addCloneModel(original, clone, "");
}

boost::shared_ptr<IStorm3D_Model> createEditorModel(IStorm3D &storm, const std::string &fileName)
{
	boost::shared_ptr<IStorm3D_Model> result(storm.CreateNewModel());
	result->LoadS3D(fileName.c_str());

	// Set editor only objects visible

	boost::scoped_ptr<Iterator<IStorm3D_Model_Object *> > objectIterator(result->ITObject->Begin());
	for(; !objectIterator->IsEnd(); objectIterator->Next())
	{
		IStorm3D_Model_Object *object = objectIterator->GetCurrent();
		if(!object)
			continue;

		std::string name = object->GetName();
		if(name.find("EditorOnly") != name.npos)
		{
			object->SetNoRender(false);
		}
	}

	return result;
}

} // editor
} // frozenbyte
