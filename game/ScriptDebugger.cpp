
#include "precompiled.h"

#include "ScriptDebugger.h"

#include <assert.h>
#include <stdio.h>

#include "../convert/str2int.h"
#include "../system/Logger.h"

#include "../filesystem/input_stream_wrapper.h"
#include "../util/Debug_MemoryManager.h"

using namespace frozenbyte;


namespace game
{

  void ScriptDebugger::init()
  {
    bool deleteMore = true;
    int i = 0;
    while (deleteMore)
    {
      char namebuf[128];
      char *tmp = int2str(i);
      strcpy(namebuf, "ScriptDebug/console_cmd_");
      strcat(namebuf, tmp);
      strcat(namebuf, ".txt");
      if (remove(namebuf) != 0)
        deleteMore = false;
      i++;
    }    
  }

  void ScriptDebugger::run(GameScripting *gs)
  {
    char namebuf[128];
    char *tmp = int2str(historyNumber);
    strcpy(namebuf, "ScriptDebug/console_cmd_");
    strcat(namebuf, tmp);
    strcat(namebuf, ".txt");

    filesystem::FB_FILE *f = filesystem::fb_fopen(namebuf, "rb");
    if (f != NULL)
    {
      //fseek(f, 0, SEEK_END);
      //int size = ftell(f);
      //fseek(f, 0, SEEK_SET);
      //fclose(f);
		int size = filesystem::fb_fsize(f);
		filesystem::fb_fclose(f);

      if (size > 0)
      {
        Logger::getInstance()->debug("ScriptDebugger::run - Executing console command...");

        char scriptName[128];
        strcpy(scriptName, "console_cmd_");
        strcat(scriptName, tmp);

        gs->loadScripts(namebuf, NULL);
        util::ScriptProcess *sp = gs->startNonUnitScript(scriptName, "_exec");
        gs->runScriptProcess(sp, false);
        historyNumber++;
        return;
      }
      /*
      fseek(f, 0, SEEK_END);
      int size = ftell(f);
      fseek(f, 0, SEEK_SET);

      char *buf = new char[size + 1];

      int got = fread(buf, 1, size, f);
      buf[size] = '\0';

      assert(got == 1);
      */
    }
  }

  int ScriptDebugger::historyNumber = 0;

}

