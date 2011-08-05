
#include "Head.h"
#include "Leg.h"
#include "Arm.h"
#include "Torso.h"
#include "SomeTorso1.h"
#include "SomeTorso1Object.h"

//
// NOTICE!!! THIS CLASS IS NO-LONGER VALID.
// to fix should create an instance of this and add this to the part list.
// just like torso or any other base part type class
//

namespace game
{
  SomeTorso1::SomeTorso1()
  {
    // NOTICE: extending class may change this pointer -> memory leak
    // can't do this! no ogui yet available!
    //image = new ui::Visual2D("Data/Pictures/Parts/torso1.tga");
    imageFilename = "Data/Pictures/Parts/torso1.tga";
    parentType = getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Tors"));
    maxDamage = 100;
    maxHeat = 100;
    resistance[DAMAGE_TYPE_PROJECTILE] = 10;
    resistance[DAMAGE_TYPE_HEAT] = 10;
    resistance[DAMAGE_TYPE_ELECTRIC] = 10;
    damagePass[DAMAGE_TYPE_PROJECTILE] = 0;
    damagePass[DAMAGE_TYPE_HEAT] = 0;
    damagePass[DAMAGE_TYPE_ELECTRIC] = 0;
    damageAbsorb[DAMAGE_TYPE_PROJECTILE] = 50;
    damageAbsorb[DAMAGE_TYPE_HEAT] = 50;
    damageAbsorb[DAMAGE_TYPE_ELECTRIC] = 50;
  }

  SomeTorso1::~SomeTorso1()
  {
    // nop
  }

  Part *SomeTorso1::getNewPartInstance()
  {
    return new SomeTorso1Object();
  } 

  //SomeTorso1 someTorso1 = SomeTorso1();

}
