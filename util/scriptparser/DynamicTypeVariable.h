
#ifndef DYNAMICTYPEVARIABLE_H
#define DYNAMICTYPEVARIABLE_H

#include <assert.h>
#include "../CheckedIntValue.h"

namespace util
{
namespace scriptparser
{
	class IDynamicTypeValueUserValueHandler 
	{
	public:
		virtual ~IDynamicTypeValueUserValueHandler() {};

		// TODO: ...
		virtual void setUserTypeValue(void *toVar, void *from) = 0;
		virtual void addUserTypeValue(void *toVar, void *from) = 0;
		virtual void subtractUserTypeValue(void *toVar, void *from) = 0;
		virtual void multiplyUserTypeValue(void *toVar, void *from) = 0;
		virtual void divideUserTypeValue(void *toVar, void *from) = 0;

		// bool
		virtual void setUserTypeValue(void *toVar, bool from) = 0;
		virtual void addUserTypeValue(void *toVar, bool from) = 0;
		virtual void subtractUserTypeValue(void *toVar, bool from) = 0;
		virtual void multiplyUserTypeValue(void *toVar, bool from) = 0;
		virtual void divideUserTypeValue(void *toVar, bool from) = 0;

		// int
		virtual void setUserTypeValue(void *toVar, int from) = 0;
		virtual void addUserTypeValue(void *toVar, int from) = 0;
		virtual void subtractUserTypeValue(void *toVar, int from) = 0;
		virtual void multiplyUserTypeValue(void *toVar, int from) = 0;
		virtual void divideUserTypeValue(void *toVar, int from) = 0;

		// float
		virtual void setUserTypeValue(void *toVar, float from) = 0;
		virtual void addUserTypeValue(void *toVar, float from) = 0;
		virtual void subtractUserTypeValue(void *toVar, float from) = 0;
		virtual void multiplyUserTypeValue(void *toVar, float from) = 0;
		virtual void divideUserTypeValue(void *toVar, float from) = 0;

		// std::string
		virtual void setUserTypeValue(void *toVar, const std::string &from) = 0;
		virtual void addUserTypeValue(void *toVar, const std::string &from) = 0;
		virtual void subtractUserTypeValue(void *toVar, const std::string &from) = 0;
		virtual void multiplyUserTypeValue(void *toVar, const std::string &from) = 0;
		virtual void divideUserTypeValue(void *toVar, const std::string &from) = 0;
	};


	class IDynamicTypeValueErrorListener
	{
	public:
		virtual ~IDynamicTypeValueErrorListener() {};

		virtual void error(const char *msg) = 0;
		virtual void warning(const char *msg) = 0;
	};


	class DynamicTypeVariable 
	{
	public:

		// note, if multiple user types are required, then the user handler should take care of those.
		enum DYNAMICTYPEVARIABLE_TYPE
		{
			DYNAMICTYPEVARIABLE_TYPE_UNINITED = 0,
			DYNAMICTYPEVARIABLE_TYPE_BOOL = 1,
			DYNAMICTYPEVARIABLE_TYPE_INT = 2,
			DYNAMICTYPEVARIABLE_TYPE_FLOAT = 3,
			DYNAMICTYPEVARIABLE_TYPE_STRING = 4,
			DYNAMICTYPEVARIABLE_TYPE_USER = 5
		};

		DynamicTypeVariable()
			: valueType(DYNAMICTYPEVARIABLE_TYPE_UNINITED), 
				boolValue(false), intValue(0), floatValue(0), stringValue(""),
				userValue(NULL)
		{ 
			// nop.
		}

		static void setDynamicTypeVariableUserHandler(IDynamicTypeValueUserValueHandler *handler)
		{
			// should always be previously uninited or being uninited now
//			assert(userHandler == NULL || handler == NULL);

			userHandler = handler;
		}

		static void setDynamicTypeVariableErrorListener(IDynamicTypeValueErrorListener *listener)
		{
			// should always be previously uninited or being uninited now
//			assert(errorListener == NULL || listener == NULL);

			errorListener = listener;
		}

		void setStringValue(const std::string &value);
		void setBoolValue(bool value);
		void setIntValue(int value);
		void setFloatValue(float value);
		void setUserValue(void *userValue);

		DYNAMICTYPEVARIABLE_TYPE getValueType() const;

		std::string getStringValue() const;
		bool getBoolValue() const;
		int getIntValue() const;
		float getFloatValue() const;
		void *getUserValue() const;

		std::string getCastStringValue() const;
		bool getCastBoolValue() const;
		int getCastIntValue() const;
		float getCastFloatValue() const;

    DynamicTypeVariable& operator= (const DynamicTypeVariable &v);
    DynamicTypeVariable& operator= (bool value);
    DynamicTypeVariable& operator= (int value);
    DynamicTypeVariable& operator= (float value);
		DynamicTypeVariable& operator= (const std::string &value);

    DynamicTypeVariable& operator+= (const DynamicTypeVariable &v);
    //DynamicTypeVariable& operator+= (bool value); // only allowed for string + bool case
    //DynamicTypeVariable& operator+= (int value);
    //DynamicTypeVariable& operator+= (float value);
		//DynamicTypeVariable& operator+= (const std::string &value);

    DynamicTypeVariable& operator-= (const DynamicTypeVariable &v);
    //DynamicTypeVariable& operator-= (bool value); // not allowed
    //DynamicTypeVariable& operator-= (int value);
    //DynamicTypeVariable& operator-= (float value);

    DynamicTypeVariable& operator*= (const DynamicTypeVariable &v);
    //DynamicTypeVariable& operator*= (bool value); // not allowed
    //DynamicTypeVariable& operator*= (int value);
    //DynamicTypeVariable& operator*= (float value);

    DynamicTypeVariable& operator/= (const DynamicTypeVariable &v);
    //DynamicTypeVariable& operator/= (bool value); // not allowed
    //DynamicTypeVariable& operator/= (int value);
    //DynamicTypeVariable& operator/= (float value);

    DynamicTypeVariable& operator%= (const DynamicTypeVariable &v);
    //DynamicTypeVariable& operator%= (bool value); // not allowed
    //DynamicTypeVariable& operator%= (int value);
    //DynamicTypeVariable& operator%= (float value);

	private:

		DYNAMICTYPEVARIABLE_TYPE valueType;

		//int intValue;
		bool boolValue;
		CheckedIntValue intValue;
		float floatValue;
		std::string stringValue;
		void *userValue;
		
		static IDynamicTypeValueUserValueHandler *userHandler;
		static IDynamicTypeValueErrorListener *errorListener;
	};


	DynamicTypeVariable operator+ (const DynamicTypeVariable &v1, const DynamicTypeVariable &v2);
	DynamicTypeVariable operator- (const DynamicTypeVariable &v1, const DynamicTypeVariable &v2);
	DynamicTypeVariable operator* (const DynamicTypeVariable &v1, const DynamicTypeVariable &v2);
	DynamicTypeVariable operator/ (const DynamicTypeVariable &v1, const DynamicTypeVariable &v2);

	DynamicTypeVariable operator+ (const DynamicTypeVariable &v1, const DynamicTypeVariable &v2);
	DynamicTypeVariable operator- (const DynamicTypeVariable &v1, const DynamicTypeVariable &v2);
	DynamicTypeVariable operator* (const DynamicTypeVariable &v1, const DynamicTypeVariable &v2);
	DynamicTypeVariable operator/ (const DynamicTypeVariable &v1, const DynamicTypeVariable &v2);

	bool operator== (const DynamicTypeVariable &v1, const DynamicTypeVariable &v2);
	bool operator!= (const DynamicTypeVariable &v1, const DynamicTypeVariable &v2);
	bool operator>= (const DynamicTypeVariable &v1, const DynamicTypeVariable &v2);
	bool operator<= (const DynamicTypeVariable &v1, const DynamicTypeVariable &v2);



}
}

#endif

