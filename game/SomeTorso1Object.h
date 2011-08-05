
#ifndef SOMETORSO1OBJECT_H
#define SOMETORSO1OBJECT_H

#include "Part.h"

namespace game
{

  class SomeTorso1Object : public Part
  {
  public:
    //SomeTorso1Object();
    //~SomeTorso1Object();

    virtual SaveData *getSaveData();
    
    virtual PartType *getType();
  };

  extern SomeTorso1Object someTorso1Object;

}

#endif
