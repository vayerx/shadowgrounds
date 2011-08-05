
#ifndef GAMEOBJECTFACTORYLIST_H
#define GAMEOBJECTFACTORYLIST_H

#include "../container/LinkedList.h"
#include "IGameObjectFactory.h"

namespace game
{

  class GameObjectFactoryList
  {
  public:
    GameObjectFactoryList();
    ~GameObjectFactoryList();

    void addFactory(int id, IGameObjectFactory *factory);
    IGameObjectFactory *getById(int id);

  private:
    LinkedList *factories;

  };

}

#endif
