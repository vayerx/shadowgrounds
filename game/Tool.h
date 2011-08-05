
#ifndef TOOL_H
#define TOOL_H

#include "PartType.h"

namespace game
{

  class Tool : public PartType
  {
  public:
    Tool();
    Tool(int id);
    virtual ~Tool();
  };

  //extern Tool tool;

}

#endif
