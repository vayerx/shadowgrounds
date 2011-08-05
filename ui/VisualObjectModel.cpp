
#include "precompiled.h"

#ifdef _MSC_VER
#pragma warning(disable:4103)
#pragma warning(disable:4786)
#endif

#include <string.h>
#include <assert.h>
#include <Storm3D_UI.h>

// DEBUG
//#include "../system/Timer.h"
//#include "../convert/str2int.h"

#include "../container/LinkedList.h"
#include "../system/Logger.h"
#include "VisualObject.h"
#include "VisualObjectModel.h"

#include "../util/Debug_MemoryManager.h"

namespace ui
{

	IStorm3D *VisualObjectModel::visualStorm = NULL;
	IStorm3D_Scene *VisualObjectModel::visualStormScene = NULL;

	LinkedList visualObjectModelList;

	int visual_object_model_allocations = 0;

	void VisualObjectModel::setVisualStorm(IStorm3D *s3d, IStorm3D_Scene *scene)
	{
		visualStorm = s3d;
		visualStormScene = scene;
	}


	VisualObjectModel::VisualObjectModel(const char *filename)
	{
		if (filename != NULL)
		{
			this->filename = new char[strlen(filename) + 1];
			strcpy(this->filename, filename);
		} else {
			this->filename = NULL;
		}
		sharedModel = NULL;
		refCount = 0;

		visual_object_model_allocations++;
		visualObjectModelList.append(this);
	}


	VisualObjectModel::~VisualObjectModel()
	{
		assert(refCount == 0);

		if (filename != NULL) delete [] filename;

		if (sharedModel != NULL)
		{
			delete sharedModel;
			sharedModel = NULL;
		}

		visual_object_model_allocations--;
		assert(visual_object_model_allocations >= 0);
		visualObjectModelList.remove(this);
	}


	void VisualObjectModel::freeObjectInstance()
	{
		assert(refCount > 0);
		refCount--;
		if (refCount == 0)
		{
			assert(sharedModel != NULL);
			//delete sharedModel;
			//sharedModel = NULL;
		}
	}



	VisualObject *VisualObjectModel::getNewObjectInstance()
	{
		if (visualStorm == NULL)
		{
			// must initialize first
			abort();
		}

		// DEBUG
		//Logger::getInstance()->error("Creating model:");
		//Logger::getInstance()->error(filename);
		//Timer::update();
		//int starttime = Timer::getTime();

		if (sharedModel == NULL)
		{
			Logger::getInstance()->debug("Creating a shared model.");
			Logger::getInstance()->debug(filename);

			sharedModel = visualStorm->CreateNewModel();
			if (filename != NULL)
			{
				// ubermagic...
				int slen = strlen(filename);
				bool hasObjectName = false;
				int objNamePart = 0;
				for (int i = 0; i < slen; i++)
				{
					if (filename[i] == ':')
					{
						hasObjectName = true;
						objNamePart = i;
						break;	
					}
				}

				// do some magic :)
				if (slen > 4 && strcmp(&filename[slen - 4], ".b3d") == 0)
				{
					// bones should not have object name.
					if (hasObjectName)
					{
						Logger::getInstance()->error("VisualObjectModel::getNewObjectInstance - Bonefilename cannot specify object.");
						return NULL;
					}
					if (!sharedModel->LoadBones(filename))
					{
						Logger::getInstance()->error("VisualObjectModel::getNewObjectInstance - Failed to load bones.");
						Logger::getInstance()->debug(filename);
					}
				} else {
					char real_fname[256];
					if (slen < 256)
					{
						strcpy(real_fname, filename);
					}
					if (hasObjectName)
					{
						if (objNamePart > 0)
							real_fname[objNamePart] = '\0';
						// else assert(0);
					}

					if (!sharedModel->LoadS3D(real_fname))
					{
						Logger::getInstance()->error("VisualObjectModel::getNewObjectInstance - Failed to load model.");
						Logger::getInstance()->debug(real_fname);
					}

					if (hasObjectName)
					{
						// delete all objects except the one we want...
						LinkedList objlist;
						Iterator<IStorm3D_Model_Object *> *object_iterator;
						for(object_iterator = sharedModel->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
						{
							IStorm3D_Model_Object *object = object_iterator->GetCurrent();
							objlist.append(object);
						}
						delete object_iterator;
						objlist.resetIterate();
						bool foundObject = false;
						while (objlist.iterateAvailable())
						{
							IStorm3D_Model_Object *object = (IStorm3D_Model_Object *)objlist.iterateNext();
							const char *objname = object->GetName();
							if (objname == NULL
								|| strcmp(&filename[objNamePart + 1], objname) != 0)
							{
								//delete object->GetMesh();
								sharedModel->Object_Delete(object);
							} else {
								if (objname != NULL)
									foundObject = true;
							}
						}
						if (!foundObject)
						{
							Logger::getInstance()->warning("VisualObjectModel::getNewObjectInstance - Model did not contain requested object.");
						}
					}

				}
			} else {
				Logger::getInstance()->debug("VisualObjectModel::getNewObjectInstance - Created empty visual object.");
			}
		}
		refCount++;

		assert(sharedModel != NULL);

		//IStorm3D_Model *model = visualStorm->CreateNewModel();
		IStorm3D_Model *model = NULL;

		// TODO: don't reload the model!! just copy the bones and stuff
		// from the shared model!!!

		// a quick hack method for getting helpers and stuff created...
		// just load the model again, and then clear the meshes...
		if (filename != NULL)
		{
			//Logger::getInstance()->error("Loading model.");
			// do some magic :)
			if (strlen(filename) > 4 
				&& strcmp(&filename[strlen(filename) - 4], ".b3d") == 0)
			{
				/*
				model = visualStorm->CreateNewModel();
				if (!model->LoadBones(filename))
				{
					Logger::getInstance()->error("VisualObjectModel::getNewObjectInstance - Failed to load bones.");
					Logger::getInstance()->debug(filename);
				}
				*/			
				model = sharedModel->GetClone(true, true, true);
			} else {
				//assert(0);
				// FIXME: ????????????????????????????????????????????
				/*
				model = visualStorm->CreateNewModel();
				model->LoadS3D(filename);
				*/
				model = sharedModel->GetClone(true, true, true);
			}
		} else {
			//Logger::getInstance()->debug("VisualObjectModel::getNewObjectInstance - Created empty visual object.");
			model = visualStorm->CreateNewModel();
		}

		assert(model != NULL);

		/*
		// delete loaded objects...
		LinkedList objlist;
		Iterator<IStorm3D_Model_Object *> *object_iterator;
		for(object_iterator = model->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
		{
			IStorm3D_Model_Object *object = object_iterator->GetCurrent();
			assert(object != NULL);
			objlist.append(object);
		}
		delete object_iterator;

		objlist.resetIterate();
		//while (objlist.iterateAvailable())
		while (!objlist.isEmpty())
		{
			//IStorm3D_Model_Object *object = (IStorm3D_Model_Object *)objlist.iterateNext();
			IStorm3D_Model_Object *object = (IStorm3D_Model_Object *)objlist.popLast();
			//delete object->GetMesh();
			model->Object_Delete(object);
		}

		// copy shared data to new model...
		Iterator<IStorm3D_Model_Object *> *del_object_iterator;
		for (del_object_iterator = sharedModel->ITObject->Begin(); !del_object_iterator->IsEnd(); del_object_iterator->Next())
		{
			IStorm3D_Model_Object *object = del_object_iterator->GetCurrent();
			IStorm3D_Mesh *mesh = object->GetMesh();
			if(mesh == NULL)
				continue;

			IStorm3D_Model_Object *objCopy = model->Object_New(object->GetName());
			objCopy->SetNoCollision(object->GetNoCollision());
			objCopy->SetNoRender(object->GetNoRender());
			objCopy->SetMesh(object->GetMesh());
			objCopy->SetPosition(object->GetPosition());
			objCopy->SetRotation(object->GetRotation());
			if(object->IsLightObject())
				objCopy->SetAsLightObject();
		} 		 
		delete del_object_iterator;
		*/

		VisualObject *ret = new VisualObject();
		ret->model = model;
		ret->animation = NULL;
		ret->storm3d = visualStorm;
		ret->scene = visualStormScene;
		ret->visualObjectModel = this;
		model->SetCustomData(ret);

		// DEBUG
		//Timer::update();
		//int timeelapsed = Timer::getTime() - starttime;
		//Logger::getInstance()->error("Time elapsed:");
		//Logger::getInstance()->error(int2str(timeelapsed));

		return ret;
	}


}
