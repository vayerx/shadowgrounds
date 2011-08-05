
#ifndef POWERCELL_H
#define POWERCELL_H

#include "PartType.h"
#include "ItemPack.h"

namespace game
{

  class PowerCell : public ItemPack
  {
  public:
    PowerCell();
    PowerCell(int id);
    virtual ~PowerCell();

  };

}

#endif
