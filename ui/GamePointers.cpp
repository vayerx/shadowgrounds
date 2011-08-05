
#include "precompiled.h"

// UNPREFERRED DEPENDENCY! 
// (due to directly modifying the visualobject models)
#include <Storm3D_UI.h>


#include "../container/LinkedList.h"
#include "../system/Logger.h"
#include "GamePointers.h"
#include "IPointableObject.h"
#include "VisualObject.h"
#include "VisualObjectModel.h"
#include "../game/GameScene.h"
#include "../game/GameMap.h"
#include "../util/ObjectStretchingCalculator.h"

#include "../util/Debug_MemoryManager.h"


namespace ui
{

  // internal class for holding individual game pointers
  class GamePointerImpl
  {
  private:
    VisualObject *visualObject;
    const IPointableObject *lockedTo;

		// replaced with the Storm3d_Line...
		IStorm3D_Line *lineObject;
		IStorm3D_Line *outRangeLineObject;
    const IPointableObject *fromLockedTo;

    friend class GamePointers;
  };


  GamePointers::GamePointers(game::GameScene *gameScene)
  {
    this->gameScene = gameScene;

    models = new VisualObjectModel *[GPOINTER_AMOUNT];
    models[GPOINTER_DUMMY] = NULL;
#ifdef LEGACY_FILES
    models[GPOINTER_WAYPOINT] = new VisualObjectModel("Data/Models/Pointers/waypointcross.s3d");
    models[GPOINTER_FINALPOINT] = new VisualObjectModel("Data/Models/Pointers/destination.s3d");
    models[GPOINTER_TARGET] = new VisualObjectModel("Data/Models/Pointers/cone.s3d");
    models[GPOINTER_GROUNDTARGET] = new VisualObjectModel("Data/Models/Pointers/areacone.s3d");
    models[GPOINTER_SELECTED] = new VisualObjectModel("Data/Models/Pointers/friendlytoroid.s3d");
    models[GPOINTER_UNSELECTED] = new VisualObjectModel("Data/Models/Pointers/friendlytoroid.s3d");
    models[GPOINTER_ENEMY] = new VisualObjectModel("Data/Models/Pointers/enemytoroid.s3d");
    models[GPOINTER_FRIENDLY] = new VisualObjectModel("Data/Models/Pointers/friendlytoroid.s3d");
    models[GPOINTER_UNSEL_TARGET] = new VisualObjectModel("Data/Models/Pointers/cone.s3d");
    models[GPOINTER_UNSEL_GROUNDTARGET] = new VisualObjectModel("Data/Models/Pointers/areacone.s3d");
    models[GPOINTER_UNSEL_FINALPOINT] = new VisualObjectModel("Data/Models/Pointers/destination.s3d");
    models[GPOINTER_UNSEL_SNEAKPOINT] = new VisualObjectModel("Data/Models/Pointers/sneakdestination.s3d");
    models[GPOINTER_SNEAKPOINT] = new VisualObjectModel("Data/Models/Pointers/sneakdestination.s3d");
    models[GPOINTER_HOSTILE_SIGHT] = new VisualObjectModel("Data/Models/Pointers/sighttoroid.s3d");
    models[GPOINTER_UNSEL_SPRINTPOINT] = new VisualObjectModel("Data/Models/Pointers/sprintdestination.s3d");
    models[GPOINTER_SPRINTPOINT] = new VisualObjectModel("Data/Models/Pointers/sprintdestination.s3d");
#else
    models[GPOINTER_WAYPOINT] = new VisualObjectModel("data/model/pointer/waypointcross.s3d");
    models[GPOINTER_FINALPOINT] = new VisualObjectModel("data/model/pointer/destination.s3d");
    models[GPOINTER_TARGET] = new VisualObjectModel("data/model/pointer/cone.s3d");
    models[GPOINTER_GROUNDTARGET] = new VisualObjectModel("data/model/pointer/areacone.s3d");
    models[GPOINTER_SELECTED] = new VisualObjectModel("data/model/pointer/friendlytoroid.s3d");
    models[GPOINTER_UNSELECTED] = new VisualObjectModel("data/model/pointer/friendlytoroid.s3d");
    models[GPOINTER_ENEMY] = new VisualObjectModel("data/model/pointer/enemytoroid.s3d");
    models[GPOINTER_FRIENDLY] = new VisualObjectModel("data/model/pointer/friendlytoroid.s3d");
    models[GPOINTER_UNSEL_TARGET] = new VisualObjectModel("data/model/pointer/cone.s3d");
    models[GPOINTER_UNSEL_GROUNDTARGET] = new VisualObjectModel("data/model/pointer/areacone.s3d");
    models[GPOINTER_UNSEL_FINALPOINT] = new VisualObjectModel("data/model/pointer/destination.s3d");
    models[GPOINTER_UNSEL_SNEAKPOINT] = new VisualObjectModel("data/model/pointer/sneakdestination.s3d");
    models[GPOINTER_SNEAKPOINT] = new VisualObjectModel("data/model/pointer/sneakdestination.s3d");
    models[GPOINTER_HOSTILE_SIGHT] = new VisualObjectModel("data/model/pointer/sighttoroid.s3d");
    models[GPOINTER_UNSEL_SPRINTPOINT] = new VisualObjectModel("data/model/pointer/sprintdestination.s3d");
    models[GPOINTER_SPRINTPOINT] = new VisualObjectModel("data/model/pointer/sprintdestination.s3d");
#endif

    objects = new VisualObject *[GPOINTER_AMOUNT];
    int i;
    for (i = 0; i < GPOINTER_AMOUNT; i++)
    {
      if (models[i] != NULL)
      {
        objects[i] = models[i]->getNewObjectInstance();
      } else {
        objects[i] = NULL;
      }
    }

    for (i = 0; i < GPOINTER_AMOUNT; i++)
    {
      if (objects[i] != NULL)
      {
        VisualObject *vo = objects[i];
        IStorm3D_Model *srcModel = vo->model;
        srcModel->SetNoCollision(true);
        Iterator<IStorm3D_Model_Object *> *object_iterator;
        for(object_iterator = srcModel->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
        {
	        IStorm3D_Model_Object *object = object_iterator->GetCurrent();
          //object->SetNoCollision(true);
	        IStorm3D_Mesh *mesh = object->GetMesh();
	        if (mesh == NULL) continue;

          IStorm3D_Material *mat = mesh->GetMaterial();
          if (mat != NULL)
          {
            if (i == GPOINTER_SPRINTPOINT || i == GPOINTER_UNSEL_SPRINTPOINT)
						{
              mat->SetColor(Color(0.1f, 0.93f, 0.8f));
              mat->SetSelfIllumination(Color(0.1f, 0.1f, 1.0f));
						} 
            else if (i == GPOINTER_SNEAKPOINT || i == GPOINTER_UNSEL_SNEAKPOINT)
						{
              mat->SetColor(Color(0.8f, 0.93f, 0.1f));
              mat->SetSelfIllumination(Color(1.0f, 1.0f, 0.1f));
						} 
            else if (i == GPOINTER_TARGET || i == GPOINTER_GROUNDTARGET
              || i == GPOINTER_ENEMY
							|| i == GPOINTER_UNSEL_TARGET || i == GPOINTER_UNSEL_GROUNDTARGET
							|| i == GPOINTER_HOSTILE_SIGHT)
            {
              mat->SetColor(Color(1.0f, 0.1f, 0.1f));
              mat->SetSelfIllumination(Color(1.0f, 0.1f, 0.1f));
            } else {
              mat->SetColor(Color(0.1f, 1.0f, 0.1f));
              mat->SetSelfIllumination(Color(0.1f, 1.0f, 0.1f));
            }
            mat->SetSpecular(Color(1, 1, 1), 5);
            mat->SetAlphaType(IStorm3D_Material::ATYPE_USE_TRANSPARENCY);
						if (i == GPOINTER_HOSTILE_SIGHT)
						{
              mat->SetTransparency(0.3f);
						}
            else if (i == GPOINTER_UNSELECTED
							|| i == GPOINTER_UNSEL_TARGET || i == GPOINTER_UNSEL_GROUNDTARGET
							|| i == GPOINTER_UNSEL_FINALPOINT || i == GPOINTER_UNSEL_SNEAKPOINT
							|| i == GPOINTER_UNSEL_SPRINTPOINT)
            {
              mat->SetTransparency(0.8f);
            } else {
              mat->SetTransparency(0.3f);
            }
          }
        }
        delete object_iterator;
      }
    }

    pointers = new LinkedList();
  }


  GamePointers::~GamePointers()
  {
    clearPointers();
    delete pointers;
    int i;
    for (i = 0; i < GPOINTER_AMOUNT; i++)
    {
      if (objects[i] != NULL)
        delete objects[i];
    }
    delete [] objects;
    for (i = 0; i < GPOINTER_AMOUNT; i++)
    {
      if (models[i] != NULL)
        delete models[i];
    }
    delete [] models;
    /*
    for (i = 0; i < GPOINTER_AMOUNT; i++)
    {
      if (materials[i] != NULL)
        delete materials[i];
    }
    delete [] materials;
    */
  }


  void GamePointers::addPointer(const VC3 &position, int pointerType, 
    const IPointableObject *lockedTo, const IPointableObject *lineFrom,
		float maxDistance)
  {
    if (pointerType < 0 || pointerType >= GPOINTER_AMOUNT)
    {
      Logger::getInstance()->error("GamePointers::addPointer - Pointer type out of range.");
      return;
    }

    IStorm3D_Model *copyModel = ui::VisualObjectModel::visualStorm->CreateNewModel();

    IStorm3D_Model *srcModel = objects[pointerType]->model;
    copyModel->SetNoCollision(true);
    Iterator<IStorm3D_Model_Object *> *object_iterator;
    for(object_iterator = srcModel->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
    {
	    IStorm3D_Model_Object *object = object_iterator->GetCurrent();
	    IStorm3D_Mesh *mesh = object->GetMesh();
	    if (mesh == NULL) continue;
      IStorm3D_Model_Object *copyObj = copyModel->Object_New("");
      copyObj->SetMesh(mesh);
      //copyObj->SetNoCollision(true);
    }

    delete object_iterator;

    VisualObject *vo = new VisualObject();
    vo->model = copyModel;
    vo->animation = NULL;
    vo->storm3d = ui::VisualObjectModel::visualStorm;
    vo->scene = ui::VisualObjectModel::visualStormScene;

    //VisualObject *vo = models[pointerType]->getNewObjectInstance();
/*
    IStorm3D_Model *srcModel = vo->model;
    Iterator<IStorm3D_Model_Object *> *object_iterator;
    for(object_iterator = srcModel->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
    {
	    IStorm3D_Model_Object *object = object_iterator->GetCurrent();
      object->SetNoCollision(true);
	    IStorm3D_Mesh *mesh = object->GetMesh();
	    if (mesh == NULL) continue;
      if (materials[pointerType] != NULL)
        mesh->UseMaterial(materials[pointerType]);
    }
  	delete object_iterator;
*/
    vo->setPosition(position);
    vo->setInScene(true);

    // need this or the pointers will "flicker" (...why...)
    vo->prepareForRender();

    GamePointerImpl *gp = new GamePointerImpl();
    gp->visualObject = vo;
    gp->lockedTo = lockedTo;
		gp->fromLockedTo = lineFrom;
		if (lineFrom != NULL)
		{
			gp->lineObject = ui::VisualObjectModel::visualStorm->CreateNewLine();
			gp->outRangeLineObject = NULL;
			// BAD, BAD, D3D CODE!
			//gp->lineObject->SetColor(D3DCOLOR_RGBA(0,255,0,255));
			unsigned int alpha = 0x80000000;
			unsigned int color = 0x00000000;
			float thickness = 0.4f;
      if (pointerType == GPOINTER_UNSEL_TARGET 
				|| pointerType == GPOINTER_UNSEL_GROUNDTARGET
				|| pointerType == GPOINTER_UNSEL_FINALPOINT
				|| pointerType == GPOINTER_UNSEL_SNEAKPOINT
				|| pointerType == GPOINTER_UNSEL_SPRINTPOINT
				|| pointerType == GPOINTER_HOSTILE_SIGHT)
			{
				thickness = 0.1f;
  			alpha = 0x30000000;
			}
		  gp->lineObject->SetThickness(thickness);

      if (pointerType == GPOINTER_SNEAKPOINT
				|| pointerType == GPOINTER_UNSEL_SNEAKPOINT)
			{
				color = 0x00ccee00;
			} 
      else if (pointerType == GPOINTER_SPRINTPOINT
				|| pointerType == GPOINTER_UNSEL_SPRINTPOINT)
			{
				color = 0x0000eecc;
			}
      else if (pointerType == GPOINTER_TARGET 
				|| pointerType == GPOINTER_GROUNDTARGET
				|| pointerType == GPOINTER_HOSTILE_SIGHT
				|| pointerType == GPOINTER_UNSEL_TARGET
				|| pointerType == GPOINTER_UNSEL_GROUNDTARGET)
			{
				color = 0x00ff0000;
			} else {
				color = 0x0000ff00;
			}
      gp->lineObject->SetColor(color | alpha);

			ui::VisualObjectModel::visualStormScene->AddLine(gp->lineObject, true);
			gp->lineObject->AddPoint(lineFrom->getPointerPosition() + VC3(0,0.3f,0));
			VC3 startPos = lineFrom->getPointerPosition();
			VC3 distVector = position - startPos;
			float dist = distVector.GetLength();
			bool split = false;
			for (float p = 0; p < dist; p += 1.0f)
			{
				VC3 pmid = startPos + (distVector * (p / dist));
				pmid.y = gameScene->getGameMap()->getScaledHeightAt(pmid.x, pmid.z) 
					+ 0.3f;
				if (split)
					gp->outRangeLineObject->AddPoint(pmid);
				else
					gp->lineObject->AddPoint(pmid);

				if (maxDistance > 0 && p >= maxDistance-1.0f && !split)
				{
					gp->outRangeLineObject = ui::VisualObjectModel::visualStorm->CreateNewLine();
					gp->outRangeLineObject->SetThickness(thickness);
					gp->outRangeLineObject->SetColor(color | 0x10000000);
					ui::VisualObjectModel::visualStormScene->AddLine(gp->outRangeLineObject, true);
					gp->outRangeLineObject->AddPoint(pmid);
					split = true;
				}
			}
			if (split)
				gp->outRangeLineObject->AddPoint(position + VC3(0,0.3f,0));
			else
				gp->lineObject->AddPoint(position + VC3(0,0.3f,0));
		} else {
      gp->lineObject = NULL;
			gp->outRangeLineObject = NULL;
		}
    pointers->append(gp);
  }


  void GamePointers::clearPointers()
  {
    while (!pointers->isEmpty())
    {
      GamePointerImpl *gp = (GamePointerImpl *)pointers->popLast();
      VisualObject *vo = gp->visualObject;
      if (vo != NULL)
        delete vo;
      IStorm3D_Line *lo = gp->lineObject;
      if (lo != NULL)
			{
			  ui::VisualObjectModel::visualStormScene->RemoveLine(lo);
				while (lo->GetPointCount() > 0)
				{
					lo->RemovePoint(0);
				}
        delete lo;
			}
      IStorm3D_Line *lo2 = gp->outRangeLineObject;
      if (lo2 != NULL)
			{
			  ui::VisualObjectModel::visualStormScene->RemoveLine(lo2);
				while (lo2->GetPointCount() > 0)
				{
					lo2->RemovePoint(0);
				}
        delete lo2;
			}
      delete gp;
    }
  }

  void GamePointers::updatePositions()
  {
    LinkedListIterator iter = LinkedListIterator(pointers);
    while (iter.iterateAvailable())
    {
      GamePointerImpl *gp = (GamePointerImpl *)iter.iterateNext();
      if (gp->lockedTo != NULL)
      {
				VC3 pos = gp->lockedTo->getPointerPosition();
        gp->visualObject->setPosition(pos);
      }
			if (gp->fromLockedTo != NULL)
			{
				// no need for this, as the lines are never visible while
				// units moving (non-tactical mode)
				/*
				while (gp->lineObject->GetPointCount() > 0)
				{
					gp->lineObject->RemovePoint(0);
				}
				// WARNING: a bit haxored way to do this...
				gp->lineObject->AddPoint(gp->visualObject->model->GetPosition() + VC3(0,0.3f,0));
				gp->lineObject->AddPoint(gp->fromLockedTo->getPointerPosition() + VC3(0,0.3f,0));
				*/
			}
    }
  }

  void GamePointers::prepareForRender()
  {
    LinkedListIterator iter = LinkedListIterator(pointers);
    while (iter.iterateAvailable())
    {
      GamePointerImpl *gp = (GamePointerImpl *)iter.iterateNext();
      gp->visualObject->prepareForRender();
    }
  }

}

