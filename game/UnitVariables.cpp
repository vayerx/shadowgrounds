
#include "precompiled.h"

#include "UnitVariables.h"

#include <string.h>
#include <assert.h>
#include "../util/SimpleParser.h"
#include "../system/Logger.h"
#include "../convert/str2int.h"

#include "../util/Debug_MemoryManager.h"


namespace game
{
  char *UnitVariables::unitVariableNames[MAX_UNIT_VARIABLES] = { NULL };
  bool UnitVariables::inited = false;


  void UnitVariables::init()
  {
    if (inited)    
    {
      Logger::getInstance()->warning("UnitVariables::init - Already initialized.");
    }

    for (int i = 0; i < MAX_UNIT_VARIABLES; i++)
    {
      unitVariableNames[i] = NULL;
    }

    util::SimpleParser sp;
#ifdef LEGACY_FILES
    if (sp.loadFile("Data/Units/unitvariables.txt"))
#else
    if (sp.loadFile("data/unit/unitvariables.txt"))
#endif
    {
      while (sp.next())
      {
        char *keystr = sp.getKey();
        if (keystr != NULL)
        {
          int key = str2int(keystr);
					if (key >= MAX_UNIT_VARIABLES)
					{
			      Logger::getInstance()->error("UnitVariables::init - Unit variable number out of range.");
						assert(0);
					} else {
						char *value = sp.getValue();
						if (value != NULL)
						{
							unitVariableNames[key] = new char[strlen(value) + 1];
							strcpy(unitVariableNames[key], value);
						} else {
							unitVariableNames[key] = NULL;
						}
					}
        }
      }
      inited = true;
    } else {
      Logger::getInstance()->warning("UnitVariables::init - Could not read definitions file, unit variables may not work.");
    }
  }


  void UnitVariables::uninit()
  {
    if (!inited)
    {
      Logger::getInstance()->warning("UnitVariables::uninit - Not initialized.");
      return;
    }    
    for (int i = 0; i < MAX_UNIT_VARIABLES; i++)
    {
      if (unitVariableNames[i] != NULL)
      {
        delete [] unitVariableNames[i];
        unitVariableNames[i] = NULL;
      }
    }
    inited = false;
  }


  UnitVariables::UnitVariables()
  {
    allocedVariables = UNIT_VARIABLES_ALLOC;
    variable = new int[allocedVariables];
    for (int i = 0; i < allocedVariables; i++)
    {
      variable[i] = 0;
    }
  }


  UnitVariables::~UnitVariables()
  {
    delete [] variable;
  }

	void UnitVariables::clearVariables()
	{
		// reset variables
		if(variable)
		{
			for (int i = 0; i < allocedVariables; i++)
			{
				variable[i] = 0;
			}
		}
	}

  int UnitVariables::getVariableNumberByName(const char *varName)
  {
		// TODO: optimize this
		// (or optimize the script code using this)
		// (so that it will use the id numbers directly instead)
    for (int i = 0; i < MAX_UNIT_VARIABLES; i++)
    { 
			// psd
			if(!unitVariableNames[i])
				continue;

      if (strcmp(unitVariableNames[i], varName) == 0)
      {
        return i;
      }
    }
		Logger::getInstance()->warning("UnitVariabled::getVariableNumberByName - Requested unit variable name unknown.");
    assert(!"UnitVariabled::getVariableNumberByName - Requested unit variable name unknown.");
    return -1;
  }


  void UnitVariables::setVariable(int varNumber, int value)
  { 
    if (varNumber < 0 || varNumber >= allocedVariables)
    {
      Logger::getInstance()->warning("Unit::setVariable - Variable number out of range.");
      return;
    }
    variable[varNumber] = value;
  }


  void UnitVariables::setVariable(const char *varName, int value)
  { 
    int num = getVariableNumberByName(varName);
		if (num == -1)
		{
			Logger::getInstance()->error("Unit::setVariable - No unit variable with given name.");
			Logger::getInstance()->debug(varName);
		}
    assert(num != -1);
    setVariable(num, value);
  }


  int UnitVariables::getVariable(int varNumber) const
  {
    if (varNumber < 0 || varNumber >= allocedVariables)
    {
      Logger::getInstance()->warning("Unit::getVariable - Variable number out of range.");
      return 0;
    }
    return variable[varNumber];
  }


  int UnitVariables::getVariable(const char *varName) const
  {
    int num = getVariableNumberByName(varName);
		if (num == -1)
		{
			Logger::getInstance()->error("Unit::getVariable - No unit variable with given name.");
			Logger::getInstance()->debug(varName);
		}
    assert(num != -1);
    return getVariable(num);
  }


}

