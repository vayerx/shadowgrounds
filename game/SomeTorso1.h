
#ifndef SOMETORSO1_H
#define SOMETORSO1_H

//
// An example of a source code based torso
// Part type files: SomeTorso1.h, SomeTorso1.cpp
// Part object files: SomeTorso1Object.h, SomeTorso1Object.cpp
//
// To see how to make a data file based torso, see CustomTorso.h
//

#include "Torso.h"

namespace game
{

  class SomeTorso1 : public Torso
  {
  public:
    SomeTorso1();
    ~SomeTorso1();

    virtual Part *getNewPartInstance();
  };

  //extern SomeTorso1 someTorso1;

}

#endif
