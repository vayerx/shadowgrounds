
#ifndef HEAD_H
#define HEAD_H

#include "PartType.h"

namespace game
{

  class Head : public PartType
  {
  public:
    Head();
    Head(int id);
    virtual ~Head();
  };

  //extern Head head;

}

#endif
