
#ifndef REACTOR_H
#define REACTOR_H

#include "PartType.h"

namespace game
{

  class Reactor : public PartType
  {
  public:
    Reactor();
    Reactor(int id);
    virtual ~Reactor();

    virtual bool setData(char *key, char *value);

    int getEnergyAmount();

  protected:
    int energyAmount;

  };

}

#endif
