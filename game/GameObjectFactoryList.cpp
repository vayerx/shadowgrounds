
#include "precompiled.h"

#include "IGameObjectFactory.h"
#include "GameObjectFactoryList.h"

#include "../util/Debug_MemoryManager.h"

namespace game
{

  // internal class used to wrap each factory before putting to linked list
  class GameObjectFactoryNode
  {
  public:
    int id;
    IGameObjectFactory *factory;

    GameObjectFactoryNode(int id, IGameObjectFactory *factory)
    {
      this->id = id;
      this->factory = factory;
    }
  };
 

  GameObjectFactoryList::GameObjectFactoryList()
  {
    factories = new LinkedList();
  }

  GameObjectFactoryList::~GameObjectFactoryList()
  {
    // NOTICE: does not delete the objects listed by this one, but only the
    // list of them. 
    delete factories;
  }
    
  void GameObjectFactoryList::addFactory(int id, IGameObjectFactory *factory)
  {
    factories->append(new GameObjectFactoryNode(id, factory));
  }

  /*
  void GameObjectFactoryList::removeFactory(int id)
  {
    // TODO, ... remember to delete factory node and remove it from list
  }
  */

  IGameObjectFactory *GameObjectFactoryList::getById(int id)
  {
    // TODO: Would need a data structure with faster seek time
    factories->resetIterate();
    while (factories->iterateAvailable())
    {
      GameObjectFactoryNode *tmp = 
        (GameObjectFactoryNode *)factories->iterateNext();
      if (tmp->id == id)
      {
        return tmp->factory;
      }
    }
    return NULL;
  }

}

