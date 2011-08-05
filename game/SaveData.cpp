
#include "precompiled.h"

#include "SaveData.h"
#include "../util/Debug_MemoryManager.h"

namespace game
{

  SaveData::SaveData(int id, int size, BYTE *data, int childAmount, 
    GameObject **children)
  {
    this->id = id;
    this->size = size;
    if (data != NULL)
    {
      this->data = new BYTE[size];
      memcpy(this->data, data, size);
    } else {
      this->data = NULL;
    }
    if (childAmount > 0)
    {
      if (children == NULL) abort();
      this->children = new GameObject *[childAmount];
      for (int i = 0; i < childAmount; i++) 
      {
        this->children[i] = children[i];
      }
    } else {
      if (children != NULL) abort();
    }
  }

  SaveData::~SaveData()
  {
    if (data != NULL) delete [] data;
    data = NULL;
    if (children != NULL) delete [] children;
    children = NULL;
  }

}


