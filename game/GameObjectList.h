
#ifndef GAMEOBJECTLIST_H
#define GAMEOBJECTLIST_H

#include "../container/LinkedList.h"

namespace game
{

  // incomplete class def...
  class GameObject;


  class GameObjectList
  {
  public:
    GameObjectList();
    ~GameObjectList();
    
    const ListNode *add(GameObject *obj);
    void remove(GameObject *obj);
    void removeByNode(const ListNode *node, GameObject *obj);

    void resetIterate();
    bool iterateAvailable();
    GameObject *iterateNext();

  private:
    LinkedList *objects;

  };

}

#endif
