
#ifndef UNITVARIABLES_H
#define UNITVARIABLES_H

#define MAX_UNIT_VARIABLES 64

#define UNIT_VARIABLES_ALLOC MAX_UNIT_VARIABLES

namespace game
{

  class UnitVariables
  {
  public:

    // call this before using the unit variables in any way...
    static void init();

    // just in case you want proper cleanup...
    static void uninit();

    static int getVariableNumberByName(const char *varName);

    UnitVariables();
    ~UnitVariables();

    void setVariable(int varNumber, int value);
    void setVariable(const char *varName, int value);

    int getVariable(int varNumber) const;
    int getVariable(const char *varName) const;
		void clearVariables();

  private:
    int allocedVariables;
    int *variable;

    static char *unitVariableNames[MAX_UNIT_VARIABLES];
    static bool inited;

  };
}

#endif

