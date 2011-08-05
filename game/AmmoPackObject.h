
#ifndef AMMOPACKOBJECT_H
#define AMMOPACKOBJECT_H

#include "Part.h"

namespace game
{

  class AmmoPackObject : public Part
  {
  protected:
    int amount;
    int maxAmount;

  public:

    AmmoPackObject();

    virtual SaveData *getSaveData();

    int getReloadPrice();
    void reload();

    inline int getAmount() { return amount; }
    inline void setAmount(int amount) { this->amount = amount; }
    inline int getMaxAmount() { return maxAmount; }
    inline void setMaxAmount(int maxAmount) { this->maxAmount = maxAmount; }
  };

}

#endif

