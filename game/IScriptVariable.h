
#ifndef ISCRIPTVARIABLE_H
#define ISCRIPTVARIABLE_H



// TODO!!!
// Preliminary design



namespace game
{
	/**
	 * A Generic interface for game's scriptable variables.
	 * An object implementing this interface may be used within
	 * the game scripts (the object's value may be changed or read).
	 * 
   * @version 1.0, 13.3.2003
   * @author Jukka Kokkonen <jukka@frozenbyte.com>
	 * @see GameStateVariable
	 */
	class IScriptVariable
	{
		public:
			enum VARTYPE
			{
				VARTYPE_BOOLEAN = 1,
				VARTYPE_INT = 2,
				VARTYPE_FLOAT = 3,
				VARTYPE_STRING = 4
			};

			virtual IScriptVariable::VARTYPE getVariableType() = 0;

			virtual void setIntValue(int value) = 0;

			virtual void setBooleanValue(bool value) = 0;

			virtual void setFloatValue(float value) = 0;

			virtual void setStringValue(const char *value) = 0;

			virtual int getIntValue() = 0;

			virtual bool getBooleanValue() = 0;

			virtual float getFloatValue() = 0;

			virtual char *getStringValue() = 0;

			virtual bool isReadOnly() = 0;

			virtual ~IScriptVariable() {};
	};
}


#endif

