
#include "precompiled.h"

#ifdef _MSC_VER
#pragma warning(disable:4103)
#pragma warning(disable:4786)
#endif

#include <c2_common.h>
#include "VisualObject.h"

#include <Storm3D_UI.h>
#include <IStorm3D_Bone.h>
#include "../container/LinkedList.h"
#include "../system/Logger.h"
#include "../system/SystemRandom.h"
#include "../system/Timer.h"
#include "../convert/str2int.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_graphics.h"
#include "../util/AngleRotationCalculator.h"
#include "../util/TextureCache.h"
#include "../game/scaledefs.h"
#include "../util/fb_assert.h"

// UNWANTED DEPENDENCY, DUE TO SHARED MODEL AND IT'S REFERENCE COUNT
#include "VisualObjectModel.h"

// ANOTHER BAD DEPENDENCY FOR EFFECT DURATIONS...
#include "../game/gamedefs.h"

#include "../util/Debug_MemoryManager.h"

namespace ui
{

  const int visualObjectID = 0;

	static frozenbyte::TextureCache *visualobject_textureCache = NULL;

	int visual_object_allocations = 0;

  VisualObject::VisualObject()
  {
    model = NULL;
    visualObjectModel = NULL;
    objectName = NULL;
    animation = NULL;
    scene = NULL;
    inScene = false;
    visible = true;
    collidable = true;
		forcedNoCollision = false;
    sphereCollisionOnly = false;
    renderNeedRotationUpdate = false;
    renderNeedPositionUpdate = false;
    renderXAngle = 0;
    renderYAngle = 0;
    renderZAngle = 0;
    // just an "invalid" value, so it gets updated on next setPosition.
    renderPosition = VC3(-999999,-999999,-999999);
		interpolatedRenderPosition = renderPosition;
		lastRenderPosition = renderPosition;
		interpolatePerFrame = false;
		dataObject = NULL;
		effects = 0;
		effectDuration = 0;
		staticRotationYAngle = 0.0f;

		renderVisibility = 1.0f;
		renderLighting = 1.0f;

		positionInterpolationAmount = 0;
		positionInterpolationTreshold = 0.0f;
		rotationInterpolationAmount = 0;
		rotationInterpolationTreshold = 0.0f;

		int i;
		atPositionHistory = 0;
		for (i = 0; i < VISUALOBJECT_MAX_POSITION_INTERPOLATION; i++)
		{
			positionHistory[i] = renderPosition;
		}
		atRotationHistory = 0;
		for (i = 0; i < VISUALOBJECT_MAX_ROTATION_INTERPOLATION; i++)
		{
			rotationYHistory[i] = 0;
		}
		this->interpolatedRotateBone = NULL;
		this->interpolatedBoneAngle = 0;
		this->interpolatedBoneBetaAngle = 0;

		this->burnedCrispyAmount = 0;

		this->sideways = false;
		this->skyModel = false;

		visual_object_allocations++;

		needQuatRotation = false;

		reflected = false;
		motionBlurred = false;
		motionBlurLength = 0.0f;
  }

  VisualObject::~VisualObject()
  {
    // not like this, the animations are shared!
    //if (animation != NULL) 
    //{
    //  animation->Release();
    //}
    if (objectName != NULL) 
    {
      delete [] objectName;
    }
    if (model != NULL) 
    {
      if (inScene)
      {
        scene->RemoveModel(model);
      }
      delete model;
    }
    if (visualObjectModel != NULL)
    {
      visualObjectModel->freeObjectInstance();
    }
		visual_object_allocations--;
		assert(visual_object_allocations >= 0);
  }

  void *VisualObject::GetID()
  {
    return (void *)&visualObjectID;
  }

  // NOTICE: affects ray trace also! (as the model is removed from the scene)
  void VisualObject::setInScene(bool inScene)
  {
    if (inScene)
    {
      if (scene != NULL)
      {
        scene->AddModel(model); 
      }
      this->inScene = true;
    } else {
      if (scene != NULL)
      {
        scene->RemoveModel(model);
      }
      this->inScene = false;
    }
  }

  IVisualObjectData *VisualObject::getDataObject()
  {
    return dataObject;
  }

  void VisualObject::setDataObject(IVisualObjectData *dataObject)
  {
    this->dataObject = dataObject;
  }

	void VisualObject::setSideways(bool sideways)
	{
		this->sideways = sideways;
	}

  void VisualObject::setVisible(bool visible)
  {
    this->visible = visible;

    Iterator<IStorm3D_Model_Object *> *object_iterator;
    for (object_iterator = model->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
    {
	    IStorm3D_Model_Object *object = object_iterator->GetCurrent();

			// HACK: solve if collision model (if so, never set the visibility on)
			// FIXME: assumes that the name ends with "Collision", this may not always be the case for collision models
			bool isCollModel = false;
			if (object->GetName() != NULL)
			{
				int slen = strlen(object->GetName());
				if (slen >= 9)
				{
					if (strcmp(&object->GetName()[slen - 9], "Collision") == 0)
					{
						isCollModel = true;
					}
				}
			}

			if (isCollModel)
				object->SetNoRender(true);
			else
				object->SetNoRender(!visible);
    }
    delete object_iterator;
  }


  void VisualObject::setSphereCollisionOnly(bool sphereCollisionOnly)
  {
    this->sphereCollisionOnly = sphereCollisionOnly;

    Iterator<IStorm3D_Model_Object *> *object_iterator;
    for (object_iterator = model->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
    {
	    IStorm3D_Model_Object *object = object_iterator->GetCurrent();
      object->SetSphereCollisionOnly(sphereCollisionOnly);
    }
    delete object_iterator;
  }


  void VisualObject::setCollidable(bool collidable)
  {
		if (forcedNoCollision)
		{
			collidable = false;
		}
    this->collidable = collidable;

    model->SetNoCollision(!collidable);
    /*
    Iterator<IStorm3D_Model_Object *> *object_iterator;
    for (object_iterator = model->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
    {
	    IStorm3D_Model_Object *object = object_iterator->GetCurrent();
		object->SetNoCollision(!collidable);
    }
    delete object_iterator;
	*/
  }


  void VisualObject::prepareForRender(float interpolate_factor)
  {

/*
int foo1 = 0;
int foo2 = 0;
Iterator<IStorm3D_Model_Object *> *object_iterator;
for (object_iterator = model->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
{
	IStorm3D_Model_Object *object = object_iterator->GetCurrent();
	if (strcmp(object->GetName(), "effect_layer") == 0)
	{
		foo1++;
	}
	if (strcmp(object->GetName(), "effect_2nd_layer") == 0)
	{
		foo2++;
	}
}

if (foo1 > 1 || foo2 > 1)
{
	fb_assert(0);
}
Logger::getInstance()->error(int2str(foo1));
Logger::getInstance()->error(int2str(foo2));
*/


    if (renderNeedPositionUpdate)
    {
      renderNeedPositionUpdate = false;

			interpolatedRenderPosition = renderPosition;

			if (sideways)
			{
				VC3 sidewpos = renderPosition;
				sidewpos.y += 2.0f;
				sidewpos.z -= 0.5f;
				model->SetPosition(sidewpos);
			} else {

				if(interpolatePerFrame)
				{
					interpolatedRenderPosition.x = renderPosition.x * (interpolate_factor) + lastRenderPosition.x * (1 - interpolate_factor);
					interpolatedRenderPosition.y = renderPosition.y * (interpolate_factor) + lastRenderPosition.y * (1 - interpolate_factor);
					interpolatedRenderPosition.z = renderPosition.z * (interpolate_factor) + lastRenderPosition.z * (1 - interpolate_factor);
					renderNeedPositionUpdate = true;
				}

	      model->SetPosition(interpolatedRenderPosition);
			}

    }
    if (renderNeedRotationUpdate)
    {
      renderNeedRotationUpdate = false;
			QUAT rot;

			if (sideways)
			{
				if (renderYAngle < 180)
				{
					rot = QUAT(
						UNIT_ANGLE_TO_RAD(90), 
						UNIT_ANGLE_TO_RAD(0), 
						UNIT_ANGLE_TO_RAD(270));
				} else {
					rot = QUAT(
						UNIT_ANGLE_TO_RAD(90), 
						UNIT_ANGLE_TO_RAD(0), 
						UNIT_ANGLE_TO_RAD(90));
				}
			} else {
	      rot = QUAT(
	        UNIT_ANGLE_TO_RAD(renderXAngle), 
	        UNIT_ANGLE_TO_RAD(renderYAngle), 
	        UNIT_ANGLE_TO_RAD(renderZAngle));
			}

			if (staticRotationYAngle != 0.0f)
			{
				QUAT rot2 = QUAT(
					0, 
					UNIT_ANGLE_TO_RAD(staticRotationYAngle), 
					0);

				rot = rot2 * rot;
			}

			if( needQuatRotation )
			{
				needQuatRotation = false;
				rot = quatRotation;
			}

      model->SetRotation(rot);
    }

		if (effects & VISUALOBJECTMODEL_EFFECT_ELECTRIC || effects & VISUALOBJECTMODEL_EFFECT_PROTECTIVESKIN)
		{
			int rand1, rand2, rand3;
			rand1 = (SystemRandom::getInstance()->nextInt() & 3);
			rand2 = (SystemRandom::getInstance()->nextInt() & 3);
			rand3 = (SystemRandom::getInstance()->nextInt() & 3);

			Iterator<IStorm3D_Model_Object *> *object_iterator;
			for (object_iterator = model->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
			{
				IStorm3D_Model_Object *object = object_iterator->GetCurrent();
				IStorm3D_Mesh *mesh = object->GetMesh();
				if(mesh == NULL)
					continue;

				IStorm3D_Material *mat = mesh->GetMaterial();

				if (strcmp(object->GetName(), "effect_layer") == 0)
				{
					if (rand1 == 0)
					{
						mat->SetScrollSpeed(VC2(4.35f, 7.29f));
					} else {
						mat->SetScrollSpeed(VC2(0.9f, 0.4f));
					}

					float val = 0.0f;
					if (rand2 == 0)
					{
						val = 0.7f + (float)rand3 * 0.1f;
					}
					mat->SetTransparency(val);
					if (game::SimpleOptions::getBool(DH_OPT_B_RENDER_GLOW))
					{
						mat->SetGlow(0.75f - (val * 0.75f));
					}
				}
			}
			delete object_iterator;
		}
		if (effects & VISUALOBJECTMODEL_EFFECT_SLIME)
		{
			Iterator<IStorm3D_Model_Object *> *object_iterator;
			for (object_iterator = model->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
			{
				IStorm3D_Model_Object *object = object_iterator->GetCurrent();
				IStorm3D_Mesh *mesh = object->GetMesh();
				if(mesh == NULL)
					continue;

				IStorm3D_Material *mat = mesh->GetMaterial();

				if (strcmp(object->GetName(), "effect_layer") == 0)
				{
					float val = 0.0f;
					val = 1.0f - (this->effectDuration / 2000.0f);
					if (val < 0.0f) val = 0.0f;
					mat->SetTransparency(val);
				}
			}
			delete object_iterator;
		}
		if (effects & VISUALOBJECTMODEL_EFFECT_BURNING)
		{
			Iterator<IStorm3D_Model_Object *> *object_iterator;
			for (object_iterator = model->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
			{
				IStorm3D_Model_Object *object = object_iterator->GetCurrent();
				IStorm3D_Mesh *mesh = object->GetMesh();
				if(mesh == NULL)
					continue;

				IStorm3D_Material *mat = mesh->GetMaterial();

				if (strcmp(object->GetName(), "effect_layer") == 0)
				{
					float val = 0.0f;
					val = 1.0f - (this->effectDuration / 5000.0f);
					if (val < 0.0f) val = 0.0f;
					mat->SetTransparency(0.5f + val * 0.5f);
				}
			}
			delete object_iterator;
		}
		if (effects & VISUALOBJECTMODEL_EFFECT_BURNED_CRISPY)
		{
			Iterator<IStorm3D_Model_Object *> *object_iterator;
			for (object_iterator = model->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
			{
				IStorm3D_Model_Object *object = object_iterator->GetCurrent();
				IStorm3D_Mesh *mesh = object->GetMesh();
				if(mesh == NULL)
					continue;

				IStorm3D_Material *mat = mesh->GetMaterial();

				if (strcmp(object->GetName(), "effect_2nd_layer") == 0)
				{
					float val = 0.0f;
					//val = (this->effectDuration / 3000.0f);
					val = 1.0f - (this->burnedCrispyAmount / 100.0f);
					if (val < 0.0f) val = 0.0f;
					if (val > 1.0f) val = 1.0f;
					mat->SetTransparency(val);
				}
			}
			delete object_iterator;
		}
		if (effects & VISUALOBJECTMODEL_EFFECT_CLOAK)
		{
			Iterator<IStorm3D_Model_Object *> *object_iterator;
			for (object_iterator = model->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
			{
				IStorm3D_Model_Object *object = object_iterator->GetCurrent();
				IStorm3D_Mesh *mesh = object->GetMesh();
				if(mesh == NULL)
					continue;

				IStorm3D_Material *mat = mesh->GetMaterial();

				if (strcmp(object->GetName(), "effect_layer") == 0)
				{
					if (this->effectDuration > 19000)
					{
						float val = 0.0f;
						val = 1.0f - ((this->effectDuration - 19000) / 1000.0f);
						if (val < 0.0f) val = 0.0f;
						if (val > 1.0f) val = 1.0f;
						mat->SetTransparency(val);
						mat->SetGlow(val);
					} else {
						float val = 0.0f;
						val = 0.5f + 0.3f * sinf(Timer::getTime() / 500.0f);
						mat->SetTransparency(val);
						mat->SetGlow(0.0f);
					}
				}
			}
			delete object_iterator;
		}
		if (effects & VISUALOBJECTMODEL_EFFECT_CLOAKHIT || effects & VISUALOBJECTMODEL_EFFECT_CLOAKRED)
		{
			int rand1;
			rand1 = (SystemRandom::getInstance()->nextInt() & 15);

			Iterator<IStorm3D_Model_Object *> *object_iterator;
			for (object_iterator = model->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
			{
				IStorm3D_Model_Object *object = object_iterator->GetCurrent();
				IStorm3D_Mesh *mesh = object->GetMesh();
				if(mesh == NULL)
					continue;

				IStorm3D_Material *mat = mesh->GetMaterial();

				if (strcmp(object->GetName(), "effect_layer") == 0)
				{
					float val = 0.0f;
					val = 0.3f + 0.7f * ((float)rand1 / 15.0f);
					mat->SetTransparency(val);
				}
			}
			delete object_iterator;
		}
  }


  void VisualObject::setPosition(const VC3 &position)
  {
    if (model != NULL)
    {
			if (positionInterpolationAmount > 1)
			{
				positionHistory[atPositionHistory] = position;
			} else {
				if (position.x != renderPosition.x
					|| position.y != renderPosition.y
					|| position.z != renderPosition.z)
				{
					renderNeedPositionUpdate = true;
					renderPosition = position;
					if(interpolatedRenderPosition.x < -99999)
						interpolatedRenderPosition = renderPosition;
					if(lastRenderPosition.x < -99999)
						lastRenderPosition = renderPosition;
				}
      }
    }
  }


  void VisualObject::setScale(const VC3 &scale)
  {
    if (model != NULL)
    {
			model->SetScale(const_cast<VC3 &> (scale));
    }
  }


  void VisualObject::setRotationQuaternion( const QUAT& quat )
  {
	  if( model != NULL )
	  {
		  quatRotation = quat;
		  needQuatRotation = true;
		  renderNeedRotationUpdate = true;
	  }
  }

  void VisualObject::setRotation(float xAngle, float yAngle, float zAngle)
  {
    if (model != NULL)
    {
			if (rotationInterpolationAmount > 1)
			{
				if (yAngle < 0) yAngle += 360.0f;
				if (yAngle > 360.0f) yAngle -= 360.0f;
				rotationYHistory[atRotationHistory] = yAngle;
				if (xAngle != renderXAngle || zAngle != renderZAngle)
				{
					renderNeedRotationUpdate = true;
					renderXAngle = xAngle;
					renderZAngle = zAngle;
				}
			} else {
				if (xAngle != renderXAngle || yAngle != renderYAngle 
					|| zAngle != renderZAngle)
				{
					renderNeedRotationUpdate = true;
					renderXAngle = xAngle;
					renderYAngle = yAngle;
					renderZAngle = zAngle;
				}
      }
    }
  }

	void VisualObject::setSelfIllumination(const COL &color)
	{
		model->SetSelfIllumination(color);
	}

  void VisualObject::addPosition(const VC3 &position)
  {
		assert(!"VisualObject::addPosition - obsolete.");

    if (model != NULL)
    {
      VC3 newPos = model->GetPosition() + position;
      if (newPos.x != renderPosition.x
        || newPos.y != renderPosition.y
        || newPos.z != renderPosition.z)
      {
        renderNeedPositionUpdate = true;
        renderPosition = newPos;
      }
    }
  }


  void VisualObject::combine(VisualObject *sourceObject, const char *newObjectName, const char *attachTo)
  {
    if (model != NULL && sourceObject != NULL)
    {
      if (attachTo != NULL)
      {
        IStorm3D_Model *to = model;
        IStorm3D_Model *from = sourceObject->model;

				// check that object with that name does not already exist
//				#ifdef _DEBUG
					if (newObjectName != NULL && newObjectName[0] != '\0')
					{
						IStorm3D_Model_Object *oldObject = to->SearchObject(newObjectName);
						if (oldObject != NULL)
						{
							Logger::getInstance()->warning("VisualObject::combine - Object with given name already exists.");
							Logger::getInstance()->debug(newObjectName);
							return;
						}
					}
//				#endif

        // begin rip n' roll...

        IStorm3D_Helper *helper = to->SearchHelper(attachTo);
        if (helper == 0)
        {
					// HACK: weapon barrel may be in two different forms.. model helper or bone helper...
					// TODO: strncmp (when multiple weaponbarrels supported)
					if (attachTo != NULL && strcmp(attachTo, "HELPER_MODEL_WeaponBarrel") == 0)
					{
						helper = to->SearchHelper("HELPER_BONE_WeaponBarrel");
					} 
					if (attachTo != NULL && strcmp(attachTo, "HELPER_MODEL_WeaponBarrel2") == 0)
					{
						helper = to->SearchHelper("HELPER_BONE_WeaponBarrel2");
					} 
					if (attachTo != NULL && strcmp(attachTo, "HELPER_MODEL_WeaponBarrel3") == 0)
					{
						helper = to->SearchHelper("HELPER_BONE_WeaponBarrel3");
					} 
					if (attachTo != NULL && strcmp(attachTo, "HELPER_MODEL_WeaponBarrel4") == 0)
					{
						helper = to->SearchHelper("HELPER_BONE_WeaponBarrel4");
					} 

					if (helper == 0)
					{
						Logger::getInstance()->error("VisualObject::combine - Failed to find helper to attach to.");					
						return;
					}
          //Logger::getInstance()->error("VisualObject::combine - Failed to find helper to attach to.");					
          //return;
        }
                
        IStorm3D_Bone *b = helper->GetParentBone();
        if (b == 0)
        {
          Logger::getInstance()->error("VisualObject::combine - Failed to get parent bone.");
          return;
        }

        Iterator<IStorm3D_Model_Object *> *object_iterator = 0;
        for(object_iterator = from->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
        {
          IStorm3D_Model_Object *object = object_iterator->GetCurrent();

          // Create new object
          //IStorm3D_Model_Object *new_object = to->Object_New(object->GetName());
					// should change name too... -jpk
					IStorm3D_Model_Object *new_object = to->Object_New(newObjectName);
					new_object->CopyFrom(object, true);
					new_object->SetRenderPassMask(object->GetRenderPassMask());
          new_object->SetMesh(object->GetMesh());
					new_object->SetPosition(object->GetPosition());
					new_object->SetRotation(object->GetRotation());
  
          // Set to helper transform
          Matrix tm = helper->GetTM();
          Vector position = tm.GetTranslation();
          Rotation rotation = tm.GetRotation();
        
          new_object->SetPosition(position);
          new_object->SetRotation(rotation);
                
          // Link to bone
          b->AddChild(new_object);

					if (visible)
					{
						new_object->UpdateVisibility();
					}
        }
         
        delete object_iterator;

        // end rip n' roll

				// psd hax.. copy source helpers to point helpers
				// should really preserve type
        Iterator<IStorm3D_Helper *> *helper_iterator = 0;
        for(helper_iterator = from->ITHelper->Begin(); !helper_iterator->IsEnd(); helper_iterator->Next())
        {
          IStorm3D_Helper *source_helper = helper_iterator->GetCurrent();

					MAT helper_tm = helper->GetTM();
					MAT source_tm = source_helper->GetTM();
					Vector position = helper_tm.GetTransformedVector(source_tm.GetTranslation());

					//if(source_helper->GetHelperType() == IStorm3D_Helper::HTYPE_CAMERA)
					{
						VC3 up(source_tm.Get(4), source_tm.Get(5), source_tm.Get(6));
						VC3 dir(source_tm.Get(8), source_tm.Get(9), source_tm.Get(10));

						helper_tm.RotateVector(dir);
						helper_tm.RotateVector(up);

						IStorm3D_Helper_Camera *new_helper = to->Helper_Camera_New(source_helper->GetName(), position, dir, up);
						b->AddChild(new_helper);
					}
					//else
					{
					//IStorm3D_Helper *new_helper = to->Helper_Point_New(source_helper->GetName(), position);
					//b->AddChild(new_helper);
					}

        }
         
        delete helper_iterator;

        // end rip n' roll

      } else {

        IStorm3D_Model *srcModel = sourceObject->model;

        if (!collidable)
          srcModel->SetNoCollision(true);

        Iterator<IStorm3D_Model_Object *> *object_iterator;
        for (object_iterator = srcModel->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
        {
	        IStorm3D_Model_Object *object = object_iterator->GetCurrent();
	        IStorm3D_Mesh *mesh = object->GetMesh();
	        if(mesh == NULL)
		        continue;

					const char *name = newObjectName;
					// HACK: solve if collision model (if so, never set the visibility on)
					// FIXME: assumes that the name ends with "Collision", this may not always be the case for collision models
					bool isCollModel = false;
					if (object->GetName() != NULL)
					{
						int slen = strlen(object->GetName());
						if (slen >= 9)
						{
							if (strcmp(&object->GetName()[slen - 9], "Collision") == 0)
							{
								// NOTE: keep the original object name if collision object.
								name = object->GetName();
								isCollModel = true;
							}
						}
					}

          IStorm3D_Model_Object *objCopy = model->Object_New(name);
					objCopy->CopyFrom(object, true);
					objCopy->SetRenderPassMask(object->GetRenderPassMask());

          objCopy->SetMesh(object->GetMesh());

					objCopy->SetPosition(object->GetPosition());
					objCopy->SetRotation(object->GetRotation());

          //if (!collidable)
            //objCopy->SetNoCollision(true);
          if (sphereCollisionOnly)
            objCopy->SetSphereCollisionOnly(true);

          if (!visible || isCollModel)
            objCopy->SetNoRender(true);

					if (visible && !isCollModel)
					{
						objCopy->UpdateVisibility();
					}
        }      
        delete object_iterator;
      }

    }
  }


	void VisualObject::removeObject(const char *objectName)
	{
    if (model != NULL && objectName != NULL)
    {
			int foundObjects = 0;
			while (true)
			{
				IStorm3D_Model_Object *delObject = model->SearchObject(objectName);
				if (delObject != NULL)
				{
					foundObjects++;
					model->Object_Delete(delObject);	
				} else {
					if (foundObjects == 0)
					{
						//Logger::getInstance()->warning("VisualObject::removeObject - Object with given name not found.");
						Logger::getInstance()->debug("VisualObject::removeObject - Object with given name not found.");
						Logger::getInstance()->debug(objectName);
					}
					break;
				}
			}
		}
	}


  void VisualObject::clearObjects()
  {
    if (model != NULL)
    {
      //IStorm3D_Model_Object *obj = 
      //  sourceObject->model->SearchObject(sourceObject->objectName);
      
      // no such object in the source model?
      //if (obj == NULL) abort();

      // cannot do like this, iterator brokes if objects are deleted...
      /*
      Iterator<IStorm3D_Model_Object *> *object_iterator;
      for(object_iterator = model->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
      {
	      IStorm3D_Model_Object *object = object_iterator->GetCurrent();
        model->Object_Delete(object);
      }
      delete object_iterator;
      */
      LinkedList objlist;
      Iterator<IStorm3D_Model_Object *> *object_iterator;
      for(object_iterator = model->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
      {
	      IStorm3D_Model_Object *object = object_iterator->GetCurrent();
        objlist.append(object);
      }
      delete object_iterator;
      objlist.resetIterate();
      while (objlist.iterateAvailable())
      {
        IStorm3D_Model_Object *object = (IStorm3D_Model_Object *)objlist.iterateNext();
        //delete object->GetMesh();
        // can't delete mesh, it may be shared?
        model->Object_Delete(object);
      }

			LinkedList helpersList;
			Iterator<IStorm3D_Helper *> *helper_iterator = 0;
      for(helper_iterator = model->ITHelper->Begin(); !helper_iterator->IsEnd(); helper_iterator->Next())
      {
        IStorm3D_Helper *hlp = helper_iterator->GetCurrent();
				if (hlp->GetParentBone() == NULL
					|| (hlp->GetName() != NULL
					&& strncmp(hlp->GetName(), "HELPER_MODEL", 12) == 0))
				{
					helpersList.append(hlp);
				}
			}
			while (!helpersList.isEmpty())
			{
				IStorm3D_Helper *hlp = (IStorm3D_Helper *)helpersList.popLast();
				model->Helper_Delete(hlp);
			}

    }
  }


  void VisualObject::resetMaterialEffects()
  {
		// NOTE: this resets the scroll position for _shared_ materials!
		// (so, this affects all visual objects that use the same material)

		Iterator<IStorm3D_Model_Object *> *object_iterator;
		for (object_iterator = model->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
		{
			IStorm3D_Model_Object *object = object_iterator->GetCurrent();
			IStorm3D_Mesh *mesh = object->GetMesh();
			if(mesh == NULL)
				continue;

			IStorm3D_Material *mat = mesh->GetMaterial();
			if (mat != NULL)
			{
				mat->ResetScrollPosition();
			}
		}
		delete object_iterator;
	}

	
  void VisualObject::rotateBone(const char *boneName, float angle, float betaAngle)
  {
		if (boneName == NULL)
			return;

		IStorm3D_Bone *bone = model->SearchBone(boneName);

		if (bone == NULL)
		{
			Logger::getInstance()->warning("VisualObject::rotateBone - Requested bone not found.");
			Logger::getInstance()->debug(boneName);
			return;
		}

		// if rotation interpolation on, need to compensate the 
		// error caused by interpolation to bone rotation (torso twist aiming)
		if (this->rotationInterpolationAmount > 1)
		{
			if (this->interpolatedRotateBone != NULL
				&& this->interpolatedRotateBone != bone)
			{
				Logger::getInstance()->warning("VisualObject::rotateBone - Multiple manual bone rotatations not supported on a visualobject with rotation interpolation on.");
				assert(!"VisualObject::rotateBone - Multiple manual bone rotatations not supported on a visualobject with rotation interpolation on.");
				// TODO: support multiple different bone rotation even if rotation interpolation on
			}
			this->interpolatedRotateBone = bone;
			this->interpolatedBoneAngle = angle;
			this->interpolatedBoneBetaAngle = betaAngle;
		} else {
			QUAT rotationq;
			if (sideways)
			{
				if (angle < 180)
				{
					rotationq = QUAT(UNIT_ANGLE_TO_RAD(angle), UNIT_ANGLE_TO_RAD(betaAngle), 0);
				} else {
					rotationq = QUAT(-UNIT_ANGLE_TO_RAD(angle), UNIT_ANGLE_TO_RAD(betaAngle), 0);
				}
			} else {
				rotationq = QUAT(UNIT_ANGLE_TO_RAD(betaAngle), UNIT_ANGLE_TO_RAD(angle), 0);
			}
			model->BlendBoneIn(bone, rotationq, 200);
		}
	}



  void VisualObject::releaseRotatedBone(const char *boneName)
  {
		IStorm3D_Bone *bone = model->SearchBone(boneName);
		if (bone == NULL)
		{
			Logger::getInstance()->warning("VisualObject::releaseRotatedBone - Requested bone not found.");
			Logger::getInstance()->debug(boneName);
		} else {
			model->BlendBoneOut(bone, 500);
		}
	}


	VisualObjectModel *VisualObject::getVisualObjectModel()
	{
		return visualObjectModel;
	}


	// FIXME: THIS MAY ONLY BE CALLED ONCE, AFTER THE WHOLE 
	// VISUALMODELOBJECT HAS BEEN CONSTRUCTED (COMBINED/LOADED).
	// NOT BEFORE NOR AGAIN AFTER THIS CALL.

	void VisualObject::disableSharedObjects()
	{
		Iterator<IStorm3D_Model_Object *> *object_iterator;
		for (object_iterator = model->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
		{
			IStorm3D_Model_Object *object = object_iterator->GetCurrent();
			IStorm3D_Mesh *mesh = object->GetMesh();
			if(mesh == NULL)
				continue;

			IStorm3D_Mesh *meshClone = object->GetMesh()->CreateNewClone();
			object->SetMesh(meshClone);
		}
		delete object_iterator;
	}



	// FIXME: THIS MAY ONLY BE CALLED ONCE, AFTER THE WHOLE 
	// VISUALMODELOBJECT HAS BEEN CONSTRUCTED (COMBINED/LOADED).
	// NOT BEFORE NOR AGAIN AFTER THIS CALL.

	void VisualObject::createEffectLayer()
	{
		LinkedList copylist;

		Iterator<IStorm3D_Model_Object *> *object_iterator;
		for (object_iterator = model->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
		{
			IStorm3D_Model_Object *object = object_iterator->GetCurrent();

			bool isCollModel = false;
			bool isEffLayer = false;
			if (object->GetName() != NULL)
			{
				int slen = strlen(object->GetName());
				if (slen >= 9)
				{
					if (strcmp(&object->GetName()[slen - 9], "Collision") == 0)
					{
						isCollModel = true;
					}
				}
				if (strcmp(object->GetName(), "effect_2nd_layer") == 0)
				{
					isEffLayer = true;
				}
				if (strcmp(object->GetName(), "effect_layer") == 0)
				{
					fb_assert(!"VisualObject::createEffectLayer - Layer already exists!");
				}
			}

			if (!isCollModel && !isEffLayer)
			{
				copylist.append(object);
			}
		}
		delete object_iterator;

		while (!copylist.isEmpty())
		{
			IStorm3D_Model_Object *object = (IStorm3D_Model_Object *)copylist.popFirst();

			IStorm3D_Mesh *mesh = object->GetMesh();
			if(mesh == NULL)
				continue;

			IStorm3D_Model_Object *new_object = model->Object_New("effect_layer");

			IStorm3D_Mesh *meshClone = object->GetMesh()->CreateNewClone();
			new_object->CopyFrom(object, true);
			new_object->SetMesh(meshClone);

			IStorm3D_Bone *b = object->GetParentBone();
			if (b != NULL)
			{
				Vector position = object->GetPosition();
				Rotation rotation = object->GetRotation();
    
				new_object->SetPosition(position);
				new_object->SetRotation(rotation);

				b->AddChild(new_object);
			}

		}
	}


	void VisualObject::createEffectLayer2()
	{
		LinkedList copylist;

		Iterator<IStorm3D_Model_Object *> *object_iterator;
		for (object_iterator = model->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
		{
			IStorm3D_Model_Object *object = object_iterator->GetCurrent();

			bool isCollModel = false;
			bool isEffLayer = false;
			if (object->GetName() != NULL)
			{
				int slen = strlen(object->GetName());
				if (slen >= 9)
				{
					if (strcmp(&object->GetName()[slen - 9], "Collision") == 0)
					{
						isCollModel = true;
					}
				}
				if (strcmp(object->GetName(), "effect_layer") == 0)
				{
					isEffLayer = true;
				}
				if (strcmp(object->GetName(), "effect_2nd_layer") == 0)
				{
					fb_assert(!"VisualObject::createEffectLayer2 - Layer already exists!");
				}
			}

			if (!isCollModel && !isEffLayer)
			{
				copylist.append(object);
			}
		}
		delete object_iterator;

		while (!copylist.isEmpty())
		{
			IStorm3D_Model_Object *object = (IStorm3D_Model_Object *)copylist.popFirst();

			IStorm3D_Mesh *mesh = object->GetMesh();
			if(mesh == NULL)
				continue;

			IStorm3D_Model_Object *new_object = model->Object_New("effect_2nd_layer");

			IStorm3D_Mesh *meshClone = object->GetMesh()->CreateNewClone();
			new_object->CopyFrom(object, true);
			new_object->SetMesh(meshClone);

			IStorm3D_Bone *b = object->GetParentBone();
			if (b != NULL)
			{
				Vector position = object->GetPosition();
				Rotation rotation = object->GetRotation();
    
				new_object->SetPosition(position);
				new_object->SetRotation(rotation);

				b->AddChild(new_object);
			}

		}
	}



	void VisualObject::applyEffects()
	{
		assert(model != NULL);

		// no effects? just return.
		if (effects == 0)
			return;

		Iterator<IStorm3D_Model_Object *> *object_iterator;
		for (object_iterator = model->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
		{
			IStorm3D_Model_Object *object = object_iterator->GetCurrent();
			IStorm3D_Mesh *mesh = object->GetMesh();
			if(mesh == NULL)
				continue;

			IStorm3D_Material *mat = mesh->GetMaterial();
			// additive material effect
			if (effects & VISUALOBJECTMODEL_EFFECT_ADDITIVE)
			{
				mat->SetAlphaType(IStorm3D_Material::ATYPE_ADD);
			}
			if (effects & VISUALOBJECTMODEL_EFFECT_MULTIPLY)
			{
				mat->SetAlphaType(IStorm3D_Material::ATYPE_MUL);
			}
			// transparencies
			if (effects & VISUALOBJECTMODEL_EFFECT_TRANSPARENCY)
			{
				mat->SetAlphaType(IStorm3D_Material::ATYPE_USE_TRANSPARENCY);
			}
			if (effects & VISUALOBJECTMODEL_EFFECT_TRANSPARENT_80)
			{
				mat->SetTransparency(0.8f);
			}
			if (effects & VISUALOBJECTMODEL_EFFECT_TRANSPARENT_50)
			{
				mat->SetTransparency(0.5f);
			}
			if (effects & VISUALOBJECTMODEL_EFFECT_TRANSPARENT_30)
			{
				mat->SetTransparency(0.3f);
			}
			if (effects & VISUALOBJECTMODEL_EFFECT_ELECTRIC || effects & VISUALOBJECTMODEL_EFFECT_PROTECTIVESKIN)
			{
				if (strcmp(object->GetName(), "effect_layer") == 0)
				{
					if (visualobject_textureCache != NULL)
					{
						if (game::SimpleOptions::getBool(DH_OPT_B_RENDER_GLOW))
						{
							object->SetScale(VC3(1.05f, 1.05f, 1.05f));
							IStorm3D_Texture *tex = NULL;
							if(effects & VISUALOBJECTMODEL_EFFECT_PROTECTIVESKIN)
								tex = visualobject_textureCache->getTexture("protectiveskin.dds");
							else
								tex = visualobject_textureCache->getTexture("electricity.dds");
							
							if(tex != NULL)
								mat->SetBaseTexture(tex);
							mat->SetAlphaType(IStorm3D_Material::ATYPE_USE_TEXTRANSPARENCY);
							mat->SetScrollSpeed(VC2(0.35f, 0.29f));
						} else {
							object->SetScale(VC3(1.0f, 1.0f, 1.0f));
							IStorm3D_Texture *tex = NULL;
							if(effects & VISUALOBJECTMODEL_EFFECT_PROTECTIVESKIN)
								tex = visualobject_textureCache->getTexture("protectiveskin_noglow.dds");
							else
								tex = visualobject_textureCache->getTexture("electricity_noglow.dds");

							if(tex != NULL)
								mat->SetBaseTexture(tex);
							//mat->SetAlphaType(IStorm3D_Material::ATYPE_USE_TRANSPARENCY);
							mat->SetAlphaType(IStorm3D_Material::ATYPE_USE_TEXTRANSPARENCY);
							mat->SetTransparency(0.0f);
							//mat->SetScrollSpeed(VC2(0.35f, 0.29f), VC2(0.45f, 0.37f));
							mat->SetScrollSpeed(VC2(0.35f, 0.29f));
						}
						mat->EnableScroll(true);
						mat->SetSelfIllumination(COL(1.0f,1.0f,1.0f));
						mat->SetGlow(0.75f);
						mat->SetColor(COL(1.0f, 1.0f, 1.0f));
					} else {
						assert(!"VisualObject::applyEffects - No texture cache set.");
					}
				}
			}
			if (effects & VISUALOBJECTMODEL_EFFECT_BURNING)
			{
				if (strcmp(object->GetName(), "effect_layer") == 0)
				{
					if (visualobject_textureCache != NULL)
					{
						IStorm3D_Texture *tex = visualobject_textureCache->getTexture("burning.dds");
						if(tex != NULL)
							mat->SetBaseTexture(tex);
						mat->SetAlphaType(IStorm3D_Material::ATYPE_USE_TEXTRANSPARENCY);
						mat->SetTransparency(0.5f);
						mat->SetScrollSpeed(VC2(0.15f, 0.23f));
						mat->EnableScroll(true);
						mat->SetSelfIllumination(COL(0.9f,0.8f,0.7f));
						mat->SetGlow(0.0f);
						mat->SetColor(COL(1.0f, 0.6f, 0.1f));
						object->SetScale(VC3(1.0f, 1.1f, 1.0f));
					} else {
						assert(!"VisualObject::applyEffects - No texture cache set.");
					}
				}
			}
			if (effects & VISUALOBJECTMODEL_EFFECT_SLIME)
			{
				if (strcmp(object->GetName(), "effect_layer") == 0)
				{
					if (visualobject_textureCache != NULL)
					{
						IStorm3D_Texture *tex = visualobject_textureCache->getTexture("slimelayer.tga");
						if(tex != NULL)
							mat->SetBaseTexture(tex);
						mat->SetAlphaType(IStorm3D_Material::ATYPE_USE_TRANSPARENCY);
						mat->SetTransparency(0.0f);
						mat->SetColor(COL(1.0f, 1.0f, 1.0f));
						object->SetScale(VC3(1.0f, 1.0f, 1.0f));
					} else {
						assert(!"VisualObject::applyEffects - No texture cache set.");
					}
				}
			}
			if (effects & VISUALOBJECTMODEL_EFFECT_BURNED_CRISPY)
			{
				if (strcmp(object->GetName(), "effect_2nd_layer") == 0)
				{
					if (visualobject_textureCache != NULL)
					{
						IStorm3D_Texture *tex = visualobject_textureCache->getTexture("burned.dds");
						if(tex != NULL)
							mat->SetBaseTexture(tex);
						mat->SetAlphaType(IStorm3D_Material::ATYPE_USE_TRANSPARENCY);
						mat->SetTransparency(0.0f);
						mat->SetColor(COL(0.2f, 0.2f, 0.2f));

						mat->SetScrollSpeed(VC2(0.0f, 0.0f));
						mat->EnableScroll(false);

						object->SetScale(VC3(1.0f, 1.0f, 1.0f));
					} else {
						assert(!"VisualObject::applyEffects - No texture cache set.");
					}
				}
			}
			if ((effects & VISUALOBJECTMODEL_EFFECT_CLOAK) != 0
				&& (effects & VISUALOBJECTMODEL_EFFECT_CLOAKHIT) == 0)
			{
				if (strcmp(object->GetName(), "effect_layer") == 0)
				{
					if (visualobject_textureCache != NULL)
					{
						// TODO: proper texture!
						IStorm3D_Texture *tex = visualobject_textureCache->getTexture("cloak.tga");
						if(tex != NULL)
							mat->SetBaseTexture(tex);
						mat->SetAlphaType(IStorm3D_Material::ATYPE_USE_TRANSPARENCY);
						mat->SetTransparency(0.0f);
						//mat->SetScrollSpeed(VC2(0.013f, 0.03f));
						mat->SetScrollSpeed(VC2(3.13f, 7.35f));
						mat->EnableScroll(true);
						mat->SetSelfIllumination(COL(1.0f,1.0f,1.0f));
						mat->SetGlow(0.0f);
						mat->SetColor(COL(1.0f, 1.0f, 1.0f));
						object->SetScale(VC3(1.0f, 1.0f, 1.0f));
					} else {
						assert(!"VisualObject::applyEffects - No texture cache set.");
					}
				}
			}
			if (effects & VISUALOBJECTMODEL_EFFECT_CLOAKHIT || effects & VISUALOBJECTMODEL_EFFECT_CLOAKRED)
			{
				if (strcmp(object->GetName(), "effect_layer") == 0)
				{
					if (visualobject_textureCache != NULL)
					{
						// TODO: proper texture!
						IStorm3D_Texture *tex = visualobject_textureCache->getTexture("cloakhit.tga");
						if(tex != NULL)
							mat->SetBaseTexture(tex);
						mat->SetAlphaType(IStorm3D_Material::ATYPE_USE_TRANSPARENCY);
						mat->SetTransparency(0.0f);
						//mat->SetScrollSpeed(VC2(0.013f, 0.03f));
						mat->SetScrollSpeed(VC2(3.13f, 7.35f));
						mat->EnableScroll(true);
						mat->SetSelfIllumination(COL(1.0f,1.0f,1.0f));
						mat->SetGlow(1.0f);
						mat->SetColor(COL(1.0f, 1.0f, 1.0f));
						object->SetScale(VC3(1.0f, 1.0f, 1.0f));
					} else {
						assert(!"VisualObject::applyEffects - No texture cache set.");
					}
				}
			}
			// less diffuse
			if (effects & VISUALOBJECTMODEL_EFFECT_LESS_DIFFUSE)
			{
				mat->SetColor(COL(0.5f, 0.5f, 0.5f));
			}
		} 		 
		delete object_iterator;
	}


	void VisualObject::setEffect(int modelEffect)
	{
		effects |= modelEffect;
		if (model != NULL) applyEffects();
	}


	void VisualObject::clearEffects()
	{
		if (model != NULL)
		{
			/*
			Iterator<IStorm3D_Model_Object *> *object_iterator;
			for (object_iterator = model->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
			{
				IStorm3D_Model_Object *object = object_iterator->GetCurrent();
				IStorm3D_Mesh *mesh = object->GetMesh();
				if(mesh == NULL)
					continue;

				IStorm3D_Material *mat = mesh->GetMaterial();

				// FIXME: these assumptions may not always be true!!!
				//mat->SetAlphaType(IStorm3D_Material::ATYPE_NONE);
				//mat->SetColor(COL(1.0f, 1.0f, 1.0f));
			}
			delete object_iterator;
			*/
		}
		effects = 0;
	}

	void VisualObject::setStaticRotationYAngle(float staticRotationY)
	{
		this->staticRotationYAngle = staticRotationY;
	}


	void VisualObject::setPositionInterpolationAmount(int interpolationAmount)
	{
		if (interpolationAmount > VISUALOBJECT_MAX_POSITION_INTERPOLATION)
			interpolationAmount = VISUALOBJECT_MAX_POSITION_INTERPOLATION;
		this->positionInterpolationAmount = interpolationAmount;
	}

	
	void VisualObject::setPositionInterpolationTreshold(float interpolationTreshold)
	{
		if (interpolationTreshold < 0.0f)
			interpolationTreshold = 0.0f;
		this->positionInterpolationTreshold = interpolationTreshold;
	}

	
	void VisualObject::setRotationInterpolationAmount(int interpolationAmount)
	{
		if (interpolationAmount > VISUALOBJECT_MAX_ROTATION_INTERPOLATION)
			interpolationAmount = VISUALOBJECT_MAX_ROTATION_INTERPOLATION;
		this->rotationInterpolationAmount = interpolationAmount;
	}

	
	void VisualObject::setRotationInterpolationTreshold(float interpolationTreshold)
	{
		// NOTE: max. ~180 deg treshold (to keep the logic simple)
		if (interpolationTreshold > 180.0f)
			interpolationTreshold = 180.0f;
		if (interpolationTreshold < 0.0f)
			interpolationTreshold = 0.0f;
		this->rotationInterpolationTreshold = interpolationTreshold;
	}

  
	void VisualObject::advanceHistory()
	{
		lastRenderPosition = renderPosition;

		if (positionInterpolationAmount > 1)
		{
			VC3 position(0,0,0);

			int interpAmount = 1;
			position = positionHistory[atPositionHistory];
			for (int i = 1; i < positionInterpolationAmount; i++)
			{
				VC3 tmp = positionHistory[(atPositionHistory + (positionInterpolationAmount-i)) % positionInterpolationAmount];
				// TODO: should use position distance vector length, 
				// not individual components
				if (fabs(tmp.x - positionHistory[atPositionHistory].x) <= positionInterpolationTreshold
					&& fabs(tmp.y - positionHistory[atPositionHistory].y) <= positionInterpolationTreshold
					&& fabs(tmp.z - positionHistory[atPositionHistory].z) <= positionInterpolationTreshold)
				{
					position += tmp;
					interpAmount++;
				} else {
					break;
				}
			}

			position /= (float)interpAmount;

      if (position.x != renderPosition.x
        || position.y != renderPosition.y
        || position.z != renderPosition.z)
      {
				renderNeedPositionUpdate = true;
				renderPosition = position;
				if(interpolatePerFrame)
					interpolatedRenderPosition = lastRenderPosition;
				else
					interpolatedRenderPosition = renderPosition;
      }

			atPositionHistory++;
			if (atPositionHistory >= positionInterpolationAmount)
			{
				atPositionHistory = 0;
				positionHistory[atPositionHistory] = positionHistory[positionInterpolationAmount - 1];
			} else {
				positionHistory[atPositionHistory] = positionHistory[atPositionHistory-1];
			}
		}
		if (rotationInterpolationAmount > 1)
		{
			float yang;

			int interpAmount = 1;
			// Relative values used to latest direction...
			yang = 0;
			for (int i = 1; i < rotationInterpolationAmount; i++)
			{
				float tmp_abs = rotationYHistory[(atRotationHistory + (rotationInterpolationAmount-i)) % rotationInterpolationAmount];
				float tmp = util::AngleRotationCalculator::getFactoredRotationForAngles(rotationYHistory[atRotationHistory], tmp_abs, 0.1f);

				if (fabs(tmp) <= rotationInterpolationTreshold)
				{
					yang += tmp;
					interpAmount++;
				} else {
					break;
				}
			}

			yang /= interpAmount;
			float angleDifference = yang;
			yang += rotationYHistory[atRotationHistory];

      if (yang != renderYAngle)
      {
				renderNeedRotationUpdate = true;
				renderYAngle = yang;
      }

			atRotationHistory++;
			if (atRotationHistory >= rotationInterpolationAmount)
			{
				atRotationHistory = 0;
				rotationYHistory[atRotationHistory] = rotationYHistory[rotationInterpolationAmount - 1];
			} else {
				rotationYHistory[atRotationHistory] = rotationYHistory[atRotationHistory-1];
			}

			// WARNING: if bone structure (model) has changed on-the-fly,
			// this may crash, as the bone pointer may be invalidated.

			// and fix the bone rotation (torso twist)...
			if (this->interpolatedRotateBone != NULL)
			{
				// TODO: keep angle+difference inside 0-360
				QUAT rotationq;
				if (sideways)
				{
					if (renderYAngle < 180)
					{
						rotationq = QUAT(UNIT_ANGLE_TO_RAD(this->interpolatedBoneAngle - angleDifference), UNIT_ANGLE_TO_RAD(this->interpolatedBoneBetaAngle), 0);
					} else {
						rotationq = QUAT(-UNIT_ANGLE_TO_RAD(this->interpolatedBoneAngle - angleDifference), UNIT_ANGLE_TO_RAD(this->interpolatedBoneBetaAngle), 0);
					}
				} else {
					rotationq = QUAT(UNIT_ANGLE_TO_RAD(this->interpolatedBoneBetaAngle), UNIT_ANGLE_TO_RAD(this->interpolatedBoneAngle - angleDifference), 0);
				}
				model->BlendBoneIn(this->interpolatedRotateBone, rotationq, 200);

				if (fabs(angleDifference) < 0.01f)
				{
					this->interpolatedRotateBone = NULL;
					this->interpolatedBoneAngle = 0;
				}
			}
		}
		if (effectDuration > 0)
		{
			effectDuration -= GAME_TICK_MSEC;
			if (effectDuration < 0)
				effectDuration = 0;
		}
	}

	void VisualObject::setNoShadow(bool noShadow)
	{
		if (this->model != NULL)
		{
			this->model->CastShadows(!noShadow);
		}
	}

	void VisualObject::setTextureCache(frozenbyte::TextureCache *textureCache)
	{
		visualobject_textureCache = textureCache;
	}

	void VisualObject::setVisibilityFactor(float visFactor)
	{
		// snap to 0 and 1
		if (visFactor < 0.0001f)
			visFactor = 0.0f;
		if (visFactor > 0.9999f)
			visFactor = 1.0f;

		this->renderVisibility = visFactor;

		if (this->model != NULL)
		{
			Iterator<IStorm3D_Model_Object *> *object_iterator;
			for (object_iterator = model->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
			{
				IStorm3D_Model_Object *object = object_iterator->GetCurrent();
				// all other effect layers than cloak do not get the visibility factor
				if (object->GetName() == NULL
					|| strcmp(object->GetName(), "effect_layer") != 0
					|| (this->effects & VISUALOBJECTMODEL_EFFECT_CLOAK) != 0)
				{
					object->SetForceAlpha(1.0f - visFactor);
				}
			}
			delete object_iterator;
		}
	}

	float VisualObject::getVisibilityFactor() const
	{
		return this->renderVisibility;
	}


	void VisualObject::setLightVisibilityFactor(float visFactor, bool enable)
	{
		// snap to 0 and 1
		if (visFactor < 0.0001f)
			visFactor = 0.0f;
		if (visFactor > 0.9999f)
			visFactor = 1.0f;

		lightVisibility = visFactor;

		if(this->model != NULL)
		{
			Iterator<IStorm3D_Model_Object *> *object_iterator;
			for (object_iterator = model->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
			{
				IStorm3D_Model_Object *object = object_iterator->GetCurrent();
				// all other effect layers than cloak do not get the visibility factor
				if (object->GetName() == NULL
					|| strcmp(object->GetName(), "effect_layer") != 0
					|| (this->effects & VISUALOBJECTMODEL_EFFECT_CLOAK) != 0)
				{
					object->SetForceLightingAlpha(enable, 1.0f - visFactor);
				}
			}

			delete object_iterator;
		}
	}

	void VisualObject::setCAMotionBlur(bool motionBlur, float length)
	{
		assert(inScene);

		if (!this->motionBlurred && motionBlur)
		{
			// turn motion blur on...
			this->motionBlurred = true;
			this->motionBlurLength = length;

			if(this->model != NULL)
			{
				Iterator<IStorm3D_Model_Object *> *object_iterator;
				for (object_iterator = model->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
				{
					IStorm3D_Model_Object *object = object_iterator->GetCurrent();
					object->EnableRenderPass(RENDER_PASS_BIT_CAMOTIONBLUR1);
					object->EnableRenderPass(RENDER_PASS_BIT_CAMOTIONBLUR2);
					object->EnableRenderPass(RENDER_PASS_BIT_CAMOTIONBLUR3);
				}

				delete object_iterator;
			}
		}
		else if (this->motionBlurred && !motionBlur)
		{
			// turn motion blur off...
			this->motionBlurred = false;
			this->motionBlurLength = length;

			if(this->model != NULL)
			{
				Iterator<IStorm3D_Model_Object *> *object_iterator;
				for (object_iterator = model->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
				{
					IStorm3D_Model_Object *object = object_iterator->GetCurrent();
					object->DisableRenderPass(RENDER_PASS_BIT_CAMOTIONBLUR1);
					object->DisableRenderPass(RENDER_PASS_BIT_CAMOTIONBLUR2);
					object->DisableRenderPass(RENDER_PASS_BIT_CAMOTIONBLUR3);
				}

				delete object_iterator;
			}
		}
		if (this->motionBlurred)
		{
			if(this->model != NULL)
			{
				Iterator<IStorm3D_Model_Object *> *object_iterator;
				for (object_iterator = model->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
				{
					IStorm3D_Model_Object *object = object_iterator->GetCurrent();

					VC3 histPos;
					VC3 offset;

					if (this->positionInterpolationAmount > 1)
					{
						histPos = positionHistory[(this->positionInterpolationAmount + atPositionHistory - int((this->positionInterpolationAmount-1) * 0.3f * this->motionBlurLength)) % this->positionInterpolationAmount];
						offset = histPos - this->renderPosition;
						object->SetRenderPassParams(RENDER_PASS_BIT_CAMOTIONBLUR1, offset, VC3(1,1,1));

						histPos = positionHistory[(this->positionInterpolationAmount + atPositionHistory - int((this->positionInterpolationAmount-1) * 0.6f * this->motionBlurLength)) % this->positionInterpolationAmount];
						offset = histPos - this->renderPosition;
						object->SetRenderPassParams(RENDER_PASS_BIT_CAMOTIONBLUR2, offset, VC3(1,1,1));

						histPos = positionHistory[(this->positionInterpolationAmount + atPositionHistory - int((this->positionInterpolationAmount-1) * 1.0f * this->motionBlurLength)) % this->positionInterpolationAmount];
						offset = histPos - this->renderPosition;
						object->SetRenderPassParams(RENDER_PASS_BIT_CAMOTIONBLUR3, offset, VC3(1,1,1));
					}

				}
			}
		}
	}

	void VisualObject::setCAReflect(bool reflect, const VC3 &reflectionHeight, const VC3 &reflectionScale)
	{
		assert(inScene);

		if (!this->reflected && reflect)
		{
			// turn reflection on...
			this->reflected = true;

			if(this->model != NULL)
			{
				Iterator<IStorm3D_Model_Object *> *object_iterator;
				for (object_iterator = model->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
				{
					IStorm3D_Model_Object *object = object_iterator->GetCurrent();
					object->EnableRenderPass(RENDER_PASS_BIT_CAREFLECTION_REFLECTED);
				}

				delete object_iterator;
			}
		}
		else if (this->reflected && !reflect)
		{
			// turn reflection off...
			this->reflected = false;

			if(this->model != NULL)
			{
				Iterator<IStorm3D_Model_Object *> *object_iterator;
				for (object_iterator = model->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
				{
					IStorm3D_Model_Object *object = object_iterator->GetCurrent();
					object->DisableRenderPass(RENDER_PASS_BIT_CAREFLECTION_REFLECTED);
				}

				delete object_iterator;
			}
		}
		if (this->reflected)
		{
			if(this->model != NULL)
			{
				Iterator<IStorm3D_Model_Object *> *object_iterator;
				for (object_iterator = model->ITObject->Begin(); !object_iterator->IsEnd(); object_iterator->Next())
				{
					IStorm3D_Model_Object *object = object_iterator->GetCurrent();
					VC3 offset = (reflectionHeight - this->renderPosition) * 2;
					object->SetRenderPassParams(RENDER_PASS_BIT_CAREFLECTION_REFLECTED, offset, reflectionScale);
				}
			}
		}
	}

	void VisualObject::makeSkyModel()
	{
		this->skyModel = true;
		if (this->model != NULL)
		{
			this->model->MakeSkyModel();			
		}
		this->setCollidable(false);
	}

	float VisualObject::getLightVisibilityFactor() const
	{
		return lightVisibility;
	}

	void VisualObject::setLightingFactor(float lightFactor)
	{
		this->renderLighting = lightFactor;
	}

	float VisualObject::getLightingFactor() const
	{
		return this->renderLighting;
	}

	void VisualObject::setEffectDuration(int durationMsec)
	{
		this->effectDuration = durationMsec;
	}

	void VisualObject::forcePositionUpdate()
	{
		this->renderNeedPositionUpdate = true;
		this->renderNeedRotationUpdate = true;
	}

	void VisualObject::setBurnedCrispyAmount(int amount)
	{
		this->burnedCrispyAmount = amount;
	}

}

