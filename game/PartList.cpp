
#include "precompiled.h"

#include "gamedefs.h"
#include "PartList.h"

#include "../util/Debug_MemoryManager.h"

namespace game
{

  PartList::PartList()
  {
    allParts = new LinkedList();
    ownedParts = new LinkedList *[ABS_MAX_PLAYERS];
    for (int i = 0; i < ABS_MAX_PLAYERS; i++)
    {
      ownedParts[i] = new LinkedList();
    }
  }

  // NOTE, does not delete the parts inside this but just the list of them
  PartList::~PartList()
  {
    while (!allParts->isEmpty())
    {
      allParts->popLast();
    }
    delete allParts;
    for (int i = 0; i < ABS_MAX_PLAYERS; i++)
    {
      while (!ownedParts[i]->isEmpty())
      {
        ownedParts[i]->popLast();
      }
      delete ownedParts[i];
    }
    delete [] ownedParts;
  }

  SaveData *PartList::getSaveData() const
  {
    // TODO
    return NULL;
  }

  const char *PartList::getStatusInfo() const
  {
    return "PartList";
  }

  int PartList::getAllPartAmount()
  {
    // TODO, optimize, not thread safe! (caller may not be using iterator)
    int count = 0;
    allParts->resetIterate();
    while (allParts->iterateAvailable())
    {
      allParts->iterateNext();
      count++;
    }
    return count;
  }

  int PartList::getOwnedPartAmount(int player)
  {
    // TODO, optimize, not thread safe! (caller may not be using iterator)
    int count = 0;
    ownedParts[player]->resetIterate();
    while (ownedParts[player]->iterateAvailable())
    {
      ownedParts[player]->iterateNext();
      count++;
    }
    return count;
  }

  LinkedList *PartList::getAllParts()
  {
    return allParts;
  }

  LinkedList *PartList::getOwnedParts(int player)
  {
    return ownedParts[player];
  }

  void PartList::addPart(Part *part)
  {
    allParts->append(part);
    int own = part->getOwner();
    if (own != NO_PART_OWNER)
    {
      if (own < 0 || own >= ABS_MAX_PLAYERS) abort();
      ownedParts[own]->append(part);
    }
  }

  // does not delete the part, just removes it from the list
  void PartList::removePart(Part *part)
  {
    allParts->remove(part);
    int own = part->getOwner();
    if (own != NO_PART_OWNER)
    {
      if (own < 0 || own >= ABS_MAX_PLAYERS) abort();
      ownedParts[own]->remove(part);
    }    
  }

}

