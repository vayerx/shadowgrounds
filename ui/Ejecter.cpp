
#include "precompiled.h"

#include "Ejecter.h"

#include <string>
#include "VisualObject.h"
#include "IPointableObject.h"
#include "../game/Unit.h"
#include "../convert/str2int.h"

namespace ui
{

	void Ejecter::createEject(IPointableObject *unit)
	{
		// WARNING: unsafe cast!
		//game::Unit *u = (game::Unit *)unit;

		//std::string barrelName = "WeaponEject";

		//std::string modelHelperName = std::string("HELPER_MODEL_") + barrelName;
		
		//const char *weaponHelper = modelHelperName.c_str();
		//char *weaponHelper = "HELPER_MODEL_WeaponBarrel";

		//VisualObject *vo = u->getVisualObject();
		//vo->combine(muzzleflash, "eject", weaponHelper);
	}


	void Ejecter::deleteEject(IPointableObject *unit)
	{
		// WARNING: unsafe cast!
		game::Unit *u = (game::Unit *)unit;

		//VisualObject *vo = u->getVisualObject();
		//if (vo != NULL)
		//{
		//	vo->removeObject("eject");
		//}

		u->setEjectVisualEffect(NULL, 0);
	}

}



