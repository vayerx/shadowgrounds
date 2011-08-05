// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "storm.h"
#include "mouse.h"
#include <istorm3d.h>
#include "../ui/lightmanager.h"
#include "../util/procedural_properties.h"
#include "../util/procedural_applier.h"
#include "../filesystem/file_package_manager.h"
#include <istorm3d_mesh.h>

using namespace std;
using namespace boost;

extern bool forceLowDetail;

namespace frozenbyte {
namespace editor {

bool editorMode = false;

Storm::Storm(HWND hwnd)
:	storm(0),
	scene(0),
	terrain(0),
	lightManager(0),
	viewerCamera(false),
	floorScene(0)
{
	recreate(hwnd);
}

Storm::~Storm()
{
	delete lightManager;
	delete storm;
}

void Storm::recreate(HWND hwnd, bool disableBuffers)
{
	delete lightManager;
	lightManager = 0;

	if(storm)
		delete storm;

	storm = IStorm3D::Create_Storm3D_Interface(true, &filesystem::FilePackageManager::getInstance());
	if(!forceLowDetail)
	{
		storm->SetShadowQuality(75);
		storm->SetFakeShadowQuality(50);
		storm->EnableGlow(true);
		storm->EnableHighQualityTextures(true);
		storm->SetReflectionQuality(50);

		if(!editorMode)
		{
			storm->SetFakeShadowQuality(-1);
			storm->SetShadowQuality(0);
		}
	}
	else
	{
		storm->SetReflectionQuality(0);
		storm->SetShadowQuality(33);
		storm->SetFakeShadowQuality(33);
		storm->EnableGlow(false);
		storm->SetTextureLODLevel(1);
		storm->EnableHighQualityTextures(false);
		storm->AllocateProceduralTarget(false);
	}

	util::ProceduralProperties prop;
	util::applyStorm(*storm, prop);

/*
	#pragma message("**********************")
	#pragma message("** remove **")
	#pragma message("**********************")
	float ts = .7f;
	storm->addAdditionalRenderTargets(VC2(ts, ts), 1);
*/
	if(!storm->SetWindowedMode(hwnd, disableBuffers))
	{
		std::string error = storm->GetErrorString();
		MessageBox(hwnd, error.c_str(), "Failed to initialize storm.", MB_OK);

		delete storm;
		storm = 0;
		return;
	}

	scene = storm->CreateNewScene();
	scene->GetCamera()->SetFieldOfView(60.f * PI / 180.f);

	util::applyRenderer(*storm, prop);
	floorScene = storm->CreateNewScene();
}

bool Storm::rayTrace(Storm3D_CollisionInfo &info, const Mouse &mouse) const
{
	if(!terrain)
		return false;

	Vector p, d;
	VC2I screen(mouse.getX(), mouse.getY());
	scene->GetEyeVectors(screen, p, d);

	ObstacleCollisionInfo oi;
	terrain->rayTrace(p, d, 200.f, info, oi, true);

	return info.hit;
}

float Storm::getHeight(const VC2 &position) const
{
	if(!terrain)
		return 0.f;

	/*
	scoped_ptr<IStorm3D_Scene> s(storm->CreateNewScene());
	set<weak_ptr<IStorm3D_Model> >::iterator it = floorModels.begin();
	for(; it != floorModels.end(); ++it)
	{
		shared_ptr<IStorm3D_Model> m = it->lock();
		if(!m)
			continue;

		s->AddModel(m.get());
	}
	*/

	Storm3D_CollisionInfo ci;
	VC3 position3(position.x, 500.f, position.y);

	floorScene->RayTrace(position3, VC3(0,-1.f,0), 1000.f, ci, true);
	if(ci.hit)
		return ci.position.y;

	return terrain->getHeight(position);

	/*
	set<weak_ptr<IStorm3D_Model> >::iterator it = floorModels.begin();
	for(; it != floorModels.end(); ++it)
	{
		shared_ptr<IStorm3D_Model> m = it->lock();
		if(!m)
			continue;

		boost::scoped_ptr<Iterator<IStorm3D_Model_Object *> > objectIterator(m->ITObject->Begin());
		for(; !objectIterator->IsEnd(); objectIterator->Next())
		{
			IStorm3D_Model_Object *object = objectIterator->GetCurrent();
			if(!object)
				continue;

			string name = object->GetName();
			if(name.find("BuildingFloor") == name.npos)
				continue;

			bool oldNoCollision = object->GetNoCollision();
			if(oldNoCollision != false)
				object->SetNoCollision(false);

			object->RayTrace(position3, VC3(0,-1.f,0), 1000.f, ci, true);
		}
	}

	if(ci.hit)
		return ci.position.y;

	return terrain->getHeight(position);
	*/
}

bool Storm::onFloor(const VC2 &position) const
{
	/*
	scoped_ptr<IStorm3D_Scene> s(storm->CreateNewScene());
	set<weak_ptr<IStorm3D_Model> >::iterator it = floorModels.begin();
	for(; it != floorModels.end(); ++it)
	{
		shared_ptr<IStorm3D_Model> m = it->lock();
		if(!m)
			continue;

		s->AddModel(m.get());
	}
	*/

	Storm3D_CollisionInfo ci;
	VC3 position3(position.x, 500.f, position.y);

	floorScene->RayTrace(position3, VC3(0,-1.f,0), 1000.f, ci, true);
	if(ci.hit)
		return true;

	return false;

	/*
	set<weak_ptr<IStorm3D_Model> >::iterator it = floorModels.begin();
	for(; it != floorModels.end(); ++it)
	{
		shared_ptr<IStorm3D_Model> m = it->lock();
		if(!m)
			continue;

		boost::scoped_ptr<Iterator<IStorm3D_Model_Object *> > objectIterator(m->ITObject->Begin());
		for(; !objectIterator->IsEnd(); objectIterator->Next())
		{
			IStorm3D_Model_Object *object = objectIterator->GetCurrent();
			if(!object)
				continue;

			string name = object->GetName();
			if(name.find("BuildingFloor") == name.npos)
				continue;

			bool oldNoCollision = object->GetNoCollision();
			if(oldNoCollision != false)
				object->SetNoCollision(false);

			object->RayTrace(position3, VC3(0,-1.f,0), 1000.f, ci, true);
			if(ci.hit)
				return true;
		}
	}

	return false;
	*/
}

} // end of namespace editor
} // end of namespace frozenbyte
