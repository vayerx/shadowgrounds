
#ifndef IGAMEOBJECTFACTORY_H
#define IGAMEOBJECTFACTORY_H

#include "GameObject.h"

namespace game
{

  class IGameObjectFactory
  {
  public:
    // called if save data contained a chunk with this factory id
    // parent is the object saved as parent for this data or NULL if none set
    virtual GameObject *create(int id, int size, BYTE *data, 
      GameObject *parent) = 0;

    // set pointers?
    //virtual setPointers(GameObject *obj, int amount, void **ptrs) = 0;

	virtual ~IGameObjectFactory() {};
  };

}

#endif

