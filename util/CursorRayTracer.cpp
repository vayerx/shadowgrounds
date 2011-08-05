
#include "precompiled.h"


// ffs
#ifdef _DEBUG
#undef new
#endif

#include "../system/Logger.h"
#include "CursorRayTracer.h"
#include "../util/Debug_MemoryManager.h"
#include <IStorm3D_Scene.h>

#define CURSOR_RAY_MAX_LENGTH 200.0f

static const float NEAR_Z = 1.0f;
//#define FAR_Z		100.0f

//IStorm3D_Model *foo;
//IStorm3D_Model *bar;


CursorRayTracer::CursorRayTracer(IStorm3D *s3d, IStorm3D_Scene *scene, 
  IStorm3D_Terrain *terrain, int screenSizeX, int screenSizeY)
{
  this->screenSizeX = screenSizeX;
  this->screenSizeY = screenSizeY;
  this->s3d = s3d;
  this->scene = scene;
  this->terrain = terrain;

  //foo = s3d->CreateNewModel();
  //foo->LoadS3D("Data/model.s3d");
  //foo->SetScale(VC3(10.0f, 100.0f, 100.0f));
  //bar = s3d->CreateNewModel();
  //bar->LoadS3D("Data/model.s3d");
  //bar->SetScale(VC3(0.1f, 0.1f, 0.1f));
  //scene->AddModel(foo);
  //scene->AddModel(bar);
  //Iterator<IStorm3D_Model_Object *> *object_iterator;
  //for(object_iterator = foo->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
  //{
	//  IStorm3D_Model_Object *object = object_iterator->GetCurrent();
  //  IStorm3D_Mesh *mesh = object->GetMesh();
  //  mesh->UpdateCollisionTable();
	  //if(mesh == NULL)
	  //continue;

    //Logger::getInstance()->debug("got here.");
    //mesh->UpdateCollisionTable();
  //}
  //delete object_iterator;
  //Storm3D_CollisionInfo cinfo;
  //scene->RayTrace(VC3(0,1,0), VC3(1,0,0), 1000.0f, cinfo);
  //if (cinfo.hit) abort();

  //foo->SphereCollision(VC3(0,1,0), 1000.0f, cinfo);
  //if (cinfo.hit) abort();
}

CursorRayTracer::~CursorRayTracer()
{
  // nop
}

void CursorRayTracer::rayTrace(int x, int y, Storm3D_CollisionInfo &cinfo, bool sideways, bool terrainOnly, bool accurate)
{
	VC3 rayDir;
	VC3 rayOrigin;
	scene->GetCamera()->getRayVector(x, y, rayDir, rayOrigin, NEAR_Z);

	VC3 rayDirNorm = rayDir.GetNormalized();

	assert(!cinfo.hit);

  // do we really have to iterate through all models??? i guess so...
  /*
  Iterator<IStorm3D_Model *> *model_iterator;
  for(model_iterator = scene->ITModel->Begin(); !model_iterator->IsEnd(); model_iterator->Next())
  {
    IStorm3D_Model *model = model_iterator->GetCurrent();
    Iterator<IStorm3D_Model_Object *> *object_iterator;
    for(object_iterator = model->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
    {
	    IStorm3D_Model_Object *object = object_iterator->GetCurrent();
      IStorm3D_Mesh *mesh = object->GetMesh();
      if (mesh != NULL)
      {
        mesh->UpdateCollisionTable();
        //Logger::getInstance()->debug("updating...");
        //if (mesh->GetRadius() == 0) abort();
      }
    }
  	delete object_iterator;
  }
  delete model_iterator;
  */

  /*
  scene->RayTrace(rayOrigin, rayDirNorm, 
    CURSOR_RAY_MAX_LENGTH, cinfo);

  terrain->RayTrace(rayOrigin, rayDirNorm, 
    CURSOR_RAY_MAX_LENGTH, cinfo);
  */

  Storm3D_CollisionInfo sceneColl = Storm3D_CollisionInfo();
  Storm3D_CollisionInfo terrainColl = Storm3D_CollisionInfo();
  ObstacleCollisionInfo obstacleColl = ObstacleCollisionInfo();

	if (!terrainOnly)
	{
		scene->RayTrace(rayOrigin, rayDirNorm, 
			CURSOR_RAY_MAX_LENGTH, sceneColl, accurate);
	}

  terrain->rayTrace(rayOrigin, rayDirNorm, 
    CURSOR_RAY_MAX_LENGTH, terrainColl, obstacleColl, true);

	if (terrainOnly)
	{
		obstacleColl.hit = false;
	}

  if (sceneColl.hit && terrainColl.hit)
  {
    // both collided, choose closest
    // scenecollision is given preference (by 3 meters)
    if (sceneColl.range <= terrainColl.range + 3)
    {
      cinfo = sceneColl;
    } else {
      cinfo = terrainColl;
    }
  } else {
    if (sceneColl.hit && !terrainColl.hit)
    {
      // scene (model) collision
      cinfo = sceneColl;
    } else {

			// MODIFIED: ignore obstacle collision for cursor raytraces.

      // (obstacle collision is closer than possible terrain collision.)
			/*
      if (obstacleColl.hit)
      {
        cinfo.hit = true;
        cinfo.model = NULL;
        cinfo.object = NULL;
        cinfo.position = obstacleColl.position;
        cinfo.range = obstacleColl.range;
      } else {
			*/
        if (!sceneColl.hit && terrainColl.hit)
        {
          // terrain collision
          cinfo = terrainColl;
        } else {
          // no collision
          cinfo.hit = false;
          cinfo.range = 999999;
        }
			/*
      }
			*/
    }
  }

  //if (cinfo.hit)
  //{
  //  Logger::getInstance()->debug("hit");
    //abort();
  //} else {
    //Logger::getInstance()->debug("miss");
  //}
}

