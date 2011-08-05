
#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <c2_sptr.h>

#include "../container/LinkedList.h"
#include "GameObjectList.h"
#include "SaveData.h"


namespace game
{

  class GameObject
  {
  public:
    // sets the one object keeping a list of all gameobjects
    // (the one used for getting all objects for saving)
    static void setConstructorList(GameObjectList *objectList);

    // adds this object to object list defined by constructor list.
    GameObject();

    // destructor after private variables...

    // extending classes should overload this method, called at save
    virtual SaveData *getSaveData() const = 0;

    // extending classes should overload this method, used to get scene graph...
    virtual const char *getStatusInfo() const = 0;

  private:
    static GameObjectList *constructorList;

  protected:
    const ListNode *listSelfPointer;
    GameObjectList *gameObjectList;

  public:
    // removes this object from the object list it has been added to 
    virtual ~GameObject()
    {
      if (gameObjectList != NULL)
      {
        gameObjectList->removeByNode(listSelfPointer, this);
      }
    }

  };

}

#endif

