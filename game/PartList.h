
#ifndef PARTLIST_H
#define PARTLIST_H

#include "../container/LinkedList.h"
#include "GameObject.h"
#include "Part.h"


namespace game
{

  class PartList : public GameObject
  {
  public:
    PartList();
    ~PartList();

    virtual SaveData *getSaveData() const;

    virtual const char *getStatusInfo() const;

    int getAllPartAmount();
    int getOwnedPartAmount(int player);

    LinkedList *getAllParts();
    LinkedList *getOwnedParts(int player);

    void addPart(Part *part);
    void removePart(Part *part);

  private:
    LinkedList *allParts;
    LinkedList **ownedParts;
  };

}

#endif

