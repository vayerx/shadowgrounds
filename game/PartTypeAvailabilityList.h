
#ifndef PARTTYPEAVAILABILITYLIST_H
#define PARTTYPEAVAILABILITYLIST_H

//
// A list keeping all the part types that are available for each player
//

#include "../container/LinkedList.h"
#include "GameObject.h"
#include "PartType.h"


namespace game
{

  class PartTypeAvailabilityList : public GameObject
  {
  public:
    PartTypeAvailabilityList();
    ~PartTypeAvailabilityList();

    virtual SaveData *getSaveData() const;

    virtual const char *getStatusInfo() const;

    int getAvailablePartTypesAmount(int player);

    // returns a linked list containing PartType objects
    // (may want to change in future - to contain avail.amount for each type)
    LinkedList *getAvailablePartTypes(int player);

    bool isPartTypeAvailable(int player, PartType *partType);

    void addPartType(int player, PartType *partType);
    void removePartType(int player, PartType *partType);

  private:
    LinkedList **ownedPartTypes;
  };

}

#endif

