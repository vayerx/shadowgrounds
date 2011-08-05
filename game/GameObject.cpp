
#include "precompiled.h"

#include "../container/LinkedList.h"
#include "GameObject.h"
#include "GameObjectList.h"

#include "../util/Debug_MemoryManager.h"

namespace game
{

  GameObjectList *GameObject::constructorList = NULL;
  
  void GameObject::setConstructorList(GameObjectList *objectList)
  {
    constructorList = objectList;
  }

  GameObject::GameObject()
  {
    if (constructorList != NULL)
    {
      gameObjectList = constructorList;
      listSelfPointer = gameObjectList->add(this);
    }
  }

  /*
  GameObject::~GameObject()
  {
    if (gameObjectList != NULL)
    {
      gameObjectList->removeByNode(listSelfPointer, this);
    }
  }
  */


}
