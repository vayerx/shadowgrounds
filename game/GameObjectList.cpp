
#include "precompiled.h"

#include <assert.h>

#include "GameObjectList.h"
#include "../util/Debug_MemoryManager.h"

namespace game
{

  GameObjectList::GameObjectList()
  {
    objects = new LinkedList();
  }

  GameObjectList::~GameObjectList()
  {
    // NOTICE: does not delete the objects listed by this one, but only the
    // list of them. All listnode pointers of the objects listed by this one
    // are invalid now, though.

    // Check that all game objects were deleted.
    //assert(objects->isEmpty());

    delete objects;
  }
    
  const ListNode *GameObjectList::add(GameObject *obj)
  {
    objects->append(obj);
    return objects->getLastNode();
  }

  void GameObjectList::remove(GameObject *obj)
  {
    objects->remove(obj);
  }

  void GameObjectList::removeByNode(const ListNode *node, GameObject *obj)
  {
    // TODO, check that objects list really contain node, else bug
    // TODO, check that node really contains obj, else bug
    obj = obj;
    objects->removeNode(node);
  }

  // NOTE, iterator is not a seperate object -> not thread safe

  void GameObjectList::resetIterate()
  {
    objects->resetIterate();
  }

  bool GameObjectList::iterateAvailable()
  {
    return objects->iterateAvailable();
  }

  GameObject *GameObjectList::iterateNext()
  {
    return (GameObject *)objects->iterateNext();
  }

}

