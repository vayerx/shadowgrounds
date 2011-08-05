
#ifndef TORSO_H
#define TORSO_H

#include "PartType.h"

namespace game
{

  class Torso : public PartType
  {
  public:
    Torso();
    Torso(int id);
    virtual ~Torso();
  };

  //extern Torso torso;

}

#endif
