
#ifndef ITEMPACK_H
#define ITEMPACK_H

#include "PartType.h"

namespace game
{

  class ItemPack : public PartType
  {
  public:
    ItemPack();
    ItemPack(int id);
    virtual ~ItemPack();

    virtual bool setData(char *key, char *value);

    int getAmount();

    virtual void prepareNewForInherit(PartType *partType);
		virtual void saveOriginals();

  protected:
    int amount;

  };

}

#endif
