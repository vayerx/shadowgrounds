
#ifndef ARM_H
#define ARM_H

#include "PartType.h"

namespace game
{

  class Arm : public PartType
  {
  public:
    Arm();
    Arm(int id);
    virtual ~Arm();
  };

  //extern Arm arm;

}

#endif
