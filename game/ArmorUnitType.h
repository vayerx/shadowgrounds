
#ifndef ARMORUNITTYPE_H
#define ARMORUNITTYPE_H

#include "UnitType.h"

namespace game
{

  class ArmorUnitType : public UnitType
  {
  public:
    ArmorUnitType();

    virtual Unit *getNewUnitInstance(int player);

    virtual UnitActor *getActor();

    // parser calls this function to tell that we're in a sub conf
    // called with NULL key when exiting a sub conf
    // should return true if valid sub key
    virtual bool setSub(char *key);

    // parser calls this function to configure the part type based on file
    // should return true if key and value pair was identified and valid
    virtual bool setData(char *key, char *value);
  };

}

#endif

