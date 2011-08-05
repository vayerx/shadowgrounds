
#include "precompiled.h"

#include "PartTypeAvailabilityList.h"
#include "gamedefs.h"

#include "../util/Debug_MemoryManager.h"

namespace game
{

  PartTypeAvailabilityList::PartTypeAvailabilityList()
  {
    ownedPartTypes = new LinkedList *[ABS_MAX_PLAYERS];
    for (int i = 0; i < ABS_MAX_PLAYERS; i++)
    {
      ownedPartTypes[i] = new LinkedList();
    }
  }


  PartTypeAvailabilityList::~PartTypeAvailabilityList()
  {
    for (int i = 0; i < ABS_MAX_PLAYERS; i++)
    {
      delete ownedPartTypes[i];
    }
    delete [] ownedPartTypes;
  }


  SaveData *PartTypeAvailabilityList::getSaveData() const
  {
    // TODO
    return NULL;
  }


  const char *PartTypeAvailabilityList::getStatusInfo() const
  {
    return "PartTypeAvailabilityList";
  }



  int PartTypeAvailabilityList::getAvailablePartTypesAmount(int player)
  {
    // TODO, optimize!!!
    int count = 0;
    ownedPartTypes[player]->resetIterate();
    while (ownedPartTypes[player]->iterateAvailable())
    {
      ownedPartTypes[player]->iterateNext();
      count++;
    }
    return count;
  }

  // returns a linked list containing PartType objects
  // (may want to change in future - to contain avail.amount for each type)
  LinkedList *PartTypeAvailabilityList::getAvailablePartTypes(int player)
  {
    return ownedPartTypes[player];
  }

  
  bool PartTypeAvailabilityList::isPartTypeAvailable(int player, PartType *partType)
  {
    ownedPartTypes[player]->resetIterate();
    while (ownedPartTypes[player]->iterateAvailable())
    {
      PartType *tmp = (PartType *)ownedPartTypes[player]->iterateNext();
      if (tmp == partType) return true;
    }
    return false;
  }


  void PartTypeAvailabilityList::addPartType(int player, PartType *partType)
  {
    ownedPartTypes[player]->append(partType);
  }

  void PartTypeAvailabilityList::removePartType(int player, PartType *partType)
  {
    ownedPartTypes[player]->remove(partType);
  }

}
