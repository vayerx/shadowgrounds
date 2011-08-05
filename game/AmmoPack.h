
#ifndef AMMOPACK_H
#define AMMOPACK_H

#include "PartType.h"
#include "ItemPack.h"

namespace game
{

  class AmmoPack : public ItemPack
  {
  public:
    AmmoPack();
    AmmoPack(int id);
    virtual ~AmmoPack();

    virtual Part *getNewPartInstance();
  };

}

#endif
