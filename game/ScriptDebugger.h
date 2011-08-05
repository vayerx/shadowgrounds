
#ifndef SCRIPTDEBUGGER_H
#define SCRIPTDEBUGGER_H

#include "scripting/GameScripting.h"
#include "../util/ScriptProcess.h"

namespace game
{

  /**
   * Class for debugging game scripts.
   */

  class ScriptDebugger
  {
  public:
    static void init();
    static void run(GameScripting *gs);
  private:
    static int historyNumber;
  };

}

#endif

