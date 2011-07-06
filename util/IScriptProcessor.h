
#ifndef ISCRIPTPROCESSOR_H
#define ISCRIPTPROCESSOR_H

namespace util
{
  class ScriptProcess;

  class IScriptProcessor
  {
  public:
	  virtual ~IScriptProcessor() {}
    virtual bool process(ScriptProcess *sp, int command, int intData, 
      char *stringData, ScriptLastValueType *lastValue) = 0;
  };

}

#endif

