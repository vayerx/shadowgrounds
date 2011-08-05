
#ifndef ISCRIPTPROCESSOR_H
#define ISCRIPTPROCESSOR_H

typedef union {
	int i;
	float f;
} floatint;

namespace util
{
  class ScriptProcess;

  class IScriptProcessor
  {
  public:

	  virtual ~IScriptProcessor() {}
    virtual bool process(ScriptProcess *sp, int command, floatint intData,
      char *stringData, ScriptLastValueType *lastValue) = 0;
  };

}

#endif

