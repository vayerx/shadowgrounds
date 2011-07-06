
#ifndef ISCRIPTPARSERINTERFACE_H
#define ISCRIPTPARSERINTERFACE_H

#include <stdlib.h>
#include "DynamicTypeVariable.h"

namespace util
{
namespace scriptparser
{
	class IDynamicTypeVariableIterator
	{
	public:
		virtual ~IDynamicTypeVariableIterator() {};
		virtual bool isNextVariableAvailable() = 0;
		virtual const DynamicTypeVariable &nextVariable() = 0;
	};


	class IScriptParserInterface
	{
		public:
			virtual ~IScriptParserInterface() { };

			virtual bool requestNextLine() = 0;
			virtual char readCharacter() = 0;
			virtual void unreadCharacter() = 0;

			virtual void scriptParseError(const char *msg) = 0;

			virtual DynamicTypeVariable getDynamicTypeVariableValue(const char *varName) = 0;
			virtual void setDynamicTypeVariableValue(const char *varName, const DynamicTypeVariable &value) = 0;

			virtual IDynamicTypeVariableIterator *getVariables() = 0;
	};
}
}

#endif

