
#include "precompiled.h"

#include "Muzzleflasher.h"

#include <string>
#include "VisualObject.h"
#include "IPointableObject.h"
#include "../game/Unit.h"
#include "../convert/str2int.h"
#include <IStorm3D_Bone.h>

namespace ui
{

	void Muzzleflasher::createMuzzleflash(IPointableObject *unit, 
		VisualObject *muzzleflash, const std::string &name, const std::string &helper)
	{
		// WARNING: unsafe cast!
		game::Unit *u = (game::Unit *)unit;

		VisualObject *vo = u->getVisualObject();
		vo->combine(muzzleflash, name.c_str(), helper.c_str());
	}

	void Muzzleflasher::createMuzzleflash(IPointableObject *unit, 
		VisualObject *muzzleflash, int muzzleFlashBarrelNumber)
	{
		// WARNING: unsafe cast!
		game::Unit *u = (game::Unit *)unit;

		std::string barrelName = "WeaponBarrel";
		if (muzzleFlashBarrelNumber >= 2)
			barrelName += int2str(muzzleFlashBarrelNumber);

		std::string modelHelperName = std::string("HELPER_MODEL_") + barrelName;
		
		const char *weaponHelper = modelHelperName.c_str();
		//char *weaponHelper = "HELPER_MODEL_WeaponBarrel";

		VisualObject *vo = u->getVisualObject();
		vo->combine(muzzleflash, "muzzleflash", weaponHelper);
	}

	void Muzzleflasher::deleteMuzzleflash(IPointableObject *unit, const std::string &name)
	{
		// WARNING: unsafe cast!
		game::Unit *u = (game::Unit *)unit;

		VisualObject *vo = u->getVisualObject();
		if (vo != NULL)
		{
			vo->removeObject(name.c_str());
		}
		else
		{
			Logger::getInstance()->warning(("Muzzleflasher::deleteMuzzleflash - no object found with name: " + name).c_str());
		}
	}

	void Muzzleflasher::deleteMuzzleflash(IPointableObject *unit)
	{
		deleteMuzzleflash(unit, "muzzleflash");

		game::Unit *u = (game::Unit *)unit;
		u->setMuzzleflashVisualEffect(NULL, 0);
	}

	bool Muzzleflasher::getMuzzleflash(IPointableObject *unit, VisualObject *muzzleflash, const std::string &name, const std::string &helper, VC3 &pos, VC3 &scale, QUAT &rot)
	{
		game::Unit *u = (game::Unit *)unit;

		VisualObject *vo = u->getVisualObject();
		if (vo == NULL || vo->getStormModel() == NULL)
		{
			return false;
		}

		IStorm3D_Model_Object *object = vo->getStormModel()->SearchObject(name.c_str());
		if(object == NULL)
		{
			return false;
		}
		
		pos = object->GetPosition();
		scale = object->GetScale();
		rot = object->GetRotation();
		return true;
	}


	void Muzzleflasher::updateMuzzleflash(IPointableObject *unit, VisualObject *muzzleflash, const std::string &name, const std::string &helper, const VC3 &pos, const VC3 &scale)
	{
		game::Unit *u = (game::Unit *)unit;

		VisualObject *vo = u->getVisualObject();
		if (vo == NULL || vo->getStormModel() == NULL)
		{
			return;
		}

		IStorm3D_Model_Object *object = vo->getStormModel()->SearchObject(name.c_str());
		if(object == NULL)
		{
			// didn't find it - add it
			createMuzzleflash(unit, muzzleflash, name, helper);
			object = vo->getStormModel()->SearchObject(name.c_str());
			if(object == NULL)
			{
				return;
			}
		}

		object->SetPosition(pos);
		object->SetScale(scale);
		//object->SetRotation(rot);
	}

}



