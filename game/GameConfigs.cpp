// Copyright 2002-2004 Frozenbyte Ltd.

#include "precompiled.h"

#ifdef _MSC_VER
#pragma warning(disable:4103)
#pragma warning(disable:4786)
#endif

#include "GameConfigs.h"

#include <string.h>
#include <assert.h>
#include "../convert/str2int.h"
#include "../container/LinkedList.h"
#include "../system/Logger.h"

#include "../util/Debug_MemoryManager.h"


#define GAMECONFNODE_INVALID 0
#define GAMECONFNODE_BOOL 1
#define GAMECONFNODE_INT 2
#define GAMECONFNODE_FLOAT 3
#define GAMECONFNODE_STRING 4

namespace {

	struct ConfTracker
	{
		ConfTracker()
		{
		}

		~ConfTracker()
		{
			game::GameConfigs::cleanInstance();
		}
	};

	ConfTracker confTracker;
}

namespace game
{
	class GameConfNodeImpl
	{
		public:
			char *confName;
			int idNumber;
			int nodeType;
			int intValue;
			bool boolValue;
			float floatValue;
			char *stringValue;

		GameConfNodeImpl()
		{
			confName = NULL;
			idNumber = 0;
			nodeType = GAMECONFNODE_INVALID;
			intValue = 0;
			boolValue = 0;
			floatValue = 0;
			stringValue = NULL;
		}
	};

	GameConfigs *GameConfigs::instance = NULL;


	GameConfigs::GameConfigs()
	{
		confList = new LinkedList();
		for (int i = 0; i < GAMECONFIGS_MAX_IDS; i++)
		{
			idTable[i] = NULL;
		}
	}


	GameConfigs::~GameConfigs()
	{
    while (!confList->isEmpty())
		{
			GameConfNodeImpl *n = (GameConfNodeImpl *)confList->popLast();
			delete [] (n->confName);
			if (n->stringValue != NULL)
				delete [] (n->stringValue);
			delete n;
		}
		delete confList;
	}


	GameConfigs *GameConfigs::getInstance()
	{
		if (instance == NULL)
		{
			instance = new GameConfigs();
		}
		return instance;
	}


	void GameConfigs::cleanInstance()
	{
		if (instance != NULL)
		{
			delete instance;
			instance = NULL;
		}
	}


	bool GameConfigs::getBoolean(const char *confname)
	{
		GameConfNodeImpl *n = getNode(confname);
		if (n == NULL)
		{
			Logger::getInstance()->warning("GameConfigs::getBoolean - Requested conf not found.");
			Logger::getInstance()->debug(confname);
			return false;
		} else {
			assert(n->nodeType == GAMECONFNODE_BOOL);
			return n->boolValue;
		}
	}


	int GameConfigs::getInt(const char *confname)
	{
		GameConfNodeImpl *n = getNode(confname);
		if (n == NULL) 
		{
			Logger::getInstance()->warning("GameConfigs::getInt - Requested conf not found.");
			Logger::getInstance()->debug(confname);
			return 0;
		} else {
			assert(n->nodeType == GAMECONFNODE_INT);
		  return n->intValue;
		}
	}


	float GameConfigs::getFloat(const char *confname)
	{
		GameConfNodeImpl *n = getNode(confname);
		if (n == NULL) 
		{
			Logger::getInstance()->warning("GameConfigs::getFloat - Requested conf not found.");
			Logger::getInstance()->debug(confname);
			return 0;
		} else {
			assert(n->nodeType == GAMECONFNODE_FLOAT);
		  return n->floatValue;
		}
	}


	char *GameConfigs::getString(const char *confname)
	{
		GameConfNodeImpl *n = getNode(confname);
		if (n == NULL) 
		{
			Logger::getInstance()->warning("GameConfigs::getString - Requested conf not found.");
			Logger::getInstance()->debug(confname);
			return NULL;
		} else {
			assert(n->nodeType == GAMECONFNODE_STRING);
		  return n->stringValue;
		}
	}


	bool GameConfigs::getBoolean(int id)
	{
		assert(id >= 0 && id < GAMECONFIGS_MAX_IDS);
	  GameConfNodeImpl *n = idTable[id];
		if (n == NULL)
		{
			Logger::getInstance()->warning("GameConfigs::getBoolean - Requested conf id not found.");
			Logger::getInstance()->debug(int2str(id));
			return false;
		} else {
			assert(n->nodeType == GAMECONFNODE_BOOL);
			return n->boolValue;
		}
	}


	int GameConfigs::getInt(int id)
	{
		assert(id >= 0 && id < GAMECONFIGS_MAX_IDS);
	  GameConfNodeImpl *n = idTable[id];
		if (n == NULL)
		{
			Logger::getInstance()->warning("GameConfigs::getInt - Requested conf id not found.");
			Logger::getInstance()->debug(int2str(id));
			return 0;
		} else {
			assert(n->nodeType == GAMECONFNODE_INT);
			return n->intValue;
		}
	}


	float GameConfigs::getFloat(int id)
	{
		assert(id >= 0 && id < GAMECONFIGS_MAX_IDS);
	  GameConfNodeImpl *n = idTable[id];
		if (n == NULL)
		{
			Logger::getInstance()->warning("GameConfigs::getFloat - Requested conf id not found.");
			Logger::getInstance()->debug(int2str(id));
			return 0;
		} else {
			assert(n->nodeType == GAMECONFNODE_FLOAT);
			return n->floatValue;
		}
	}


	char *GameConfigs::getString(int id)
	{
		assert(id >= 0 && id < GAMECONFIGS_MAX_IDS);
	  GameConfNodeImpl *n = idTable[id];
		if (n == NULL)
		{
			Logger::getInstance()->warning("GameConfigs::getString - Requested conf id not found.");
			Logger::getInstance()->debug(int2str(id));
			return 0;
		} else {
			assert(n->nodeType == GAMECONFNODE_STRING);
			return n->stringValue;
		}
	}


	const char *GameConfigs::getNameForId(int id)
	{
		assert(id >= 0 && id < GAMECONFIGS_MAX_IDS);
	  GameConfNodeImpl *n = idTable[id];
		if (n == NULL)
		{
			Logger::getInstance()->warning("GameConfigs::getNameForId - Requested conf id not found.");
			Logger::getInstance()->debug(int2str(id));
			return NULL;
		} else {
			return n->confName;
		}		
	}


	void GameConfigs::addBoolean(const char *confname, bool value,
		int id)
	{
		GameConfNodeImpl *n = getNode(confname);
		if (n == NULL) 
		{
			n = new GameConfNodeImpl();
			n->confName = new char[strlen(confname) + 1];
			strcpy(n->confName, confname);
			confList->append(n);
		} else {
			Logger::getInstance()->warning("GameConfigs::addBoolean - Requested conf already set.");
			Logger::getInstance()->debug(confname);
			assert(n->nodeType == GAMECONFNODE_BOOL);
		}
		// TODO: assert that a node with that id does not already exist
		n->idNumber = id;
		n->nodeType = GAMECONFNODE_BOOL;
	  n->boolValue = value;
		n->intValue = 0;
		n->floatValue = 0.0f;
		n->stringValue = NULL;
		if (id != -1)
		{
			assert(id >= 0 && id < GAMECONFIGS_MAX_IDS);
			idTable[id] = n;
		}
	}


	void GameConfigs::addInt(const char *confname, int value,
		int id)
	{
		GameConfNodeImpl *n = getNode(confname);
		if (n == NULL) 
		{
			n = new GameConfNodeImpl();
			n->confName = new char[strlen(confname) + 1];
			strcpy(n->confName, confname);
			confList->append(n);
		} else {
			Logger::getInstance()->warning("GameConfigs::addInt - Requested conf already set.");
			Logger::getInstance()->debug(confname);
			assert(n->nodeType == GAMECONFNODE_INT);
		}
		// TODO: assert that a node with that id does not already exist
		n->idNumber = id;
		n->nodeType = GAMECONFNODE_INT;
		n->boolValue = false;
	  n->intValue = value;
		n->floatValue = 0.0f;
		n->stringValue = NULL;
		if (id != -1)
		{
			assert(id >= 0 && id < GAMECONFIGS_MAX_IDS);
			idTable[id] = n;
		}
	}


	void GameConfigs::addFloat(const char *confname, float value,
		int id)
	{
		GameConfNodeImpl *n = getNode(confname);
		if (n == NULL) 
		{
			n = new GameConfNodeImpl();
			n->confName = new char[strlen(confname) + 1];
			strcpy(n->confName, confname);
			confList->append(n);
		} else {
			Logger::getInstance()->warning("GameConfigs::addFloat - Requested conf already set.");
			Logger::getInstance()->debug(confname);
			assert(n->nodeType == GAMECONFNODE_FLOAT);
		}
		// TODO: assert that a node with that id does not already exist
		n->idNumber = id;
		n->nodeType = GAMECONFNODE_FLOAT;
		n->boolValue = false;
	  n->intValue = 0;
		n->floatValue = value;
		n->stringValue = NULL;
		if (id != -1)
		{
			assert(id >= 0 && id < GAMECONFIGS_MAX_IDS);
			idTable[id] = n;
		}
	}


	void GameConfigs::addString(const char *confname, const char *value,
		int id)
	{
		GameConfNodeImpl *n = getNode(confname);
		if (n == NULL) 
		{
			n = new GameConfNodeImpl();
			n->confName = new char[strlen(confname) + 1];
			strcpy(n->confName, confname);
			confList->append(n);
		} else {
			Logger::getInstance()->warning("GameConfigs::addString - Requested conf already set.");
			Logger::getInstance()->debug(confname);
			assert(n->nodeType == GAMECONFNODE_STRING);
		}
		// TODO: assert that a node with that id does not already exist
		n->idNumber = id;
		n->nodeType = GAMECONFNODE_STRING;
		n->boolValue = false;
	  n->intValue = 0;
		n->floatValue = 0.0f;
		n->stringValue = new char[strlen(value) + 1];
		strcpy(n->stringValue, value);
		if (id != -1)
		{
			assert(id >= 0 && id < GAMECONFIGS_MAX_IDS);
			idTable[id] = n;
		}
	}


	void GameConfigs::setBoolean(const char *confname, bool value)
	{
	  GameConfNodeImpl *n = getNode(confname);
		if (n == NULL)
		{
			Logger::getInstance()->warning("GameConfigs::setBoolean - Requested conf not found.");
			Logger::getInstance()->debug(confname);
		} else {
			assert(n->nodeType == GAMECONFNODE_BOOL);
			n->boolValue = value;
		}
	}


	void GameConfigs::setInt(const char *confname, int value)
	{
	  GameConfNodeImpl *n = getNode(confname);
		if (n == NULL)
		{
			Logger::getInstance()->warning("GameConfigs::setInt - Requested conf not found.");
			Logger::getInstance()->debug(confname);
		} else {
			assert(n->nodeType == GAMECONFNODE_INT);
			n->intValue = value;
		}
	}


	void GameConfigs::setFloat(const char *confname, float value)
	{
	  GameConfNodeImpl *n = getNode(confname);
		if (n == NULL)
		{
			Logger::getInstance()->warning("GameConfigs::setFloat - Requested conf not found.");
			Logger::getInstance()->debug(confname);
		} else {
			assert(n->nodeType == GAMECONFNODE_FLOAT);
			n->floatValue = value;
		}
	}



	void GameConfigs::setString(const char *confname, const char *value)
	{
	  GameConfNodeImpl *n = getNode(confname);
		if (n == NULL)
		{
			Logger::getInstance()->warning("GameConfigs::setString - Requested conf not found.");
			Logger::getInstance()->debug(confname);
		} else {
			assert(n->nodeType == GAMECONFNODE_STRING);
			if (n->stringValue != NULL)
			{
				delete [] n->stringValue;
			}
			n->stringValue = new char[strlen(value) + 1];
			strcpy(n->stringValue, value);
		}
	}


	void GameConfigs::setBoolean(int id, bool value)
	{
		assert(id >= 0 && id < GAMECONFIGS_MAX_IDS);
	  GameConfNodeImpl *n = idTable[id];
		if (n == NULL)
		{
			Logger::getInstance()->warning("GameConfigs::setBoolean - Requested conf id not found.");
			Logger::getInstance()->debug(int2str(id));
		} else {
			assert(n->nodeType == GAMECONFNODE_BOOL);
			n->boolValue = value;
		}
	}


	void GameConfigs::setInt(int id, int value)
	{
		assert(id >= 0 && id < GAMECONFIGS_MAX_IDS);
	  GameConfNodeImpl *n = idTable[id];
		if (n == NULL)
		{
			Logger::getInstance()->warning("GameConfigs::setInt - Requested conf id not found.");
			Logger::getInstance()->debug(int2str(id));
		} else {
			assert(n->nodeType == GAMECONFNODE_INT);
			n->intValue = value;
		}
	}


	void GameConfigs::setFloat(int id, float value)
	{
		assert(id >= 0 && id < GAMECONFIGS_MAX_IDS);
	  GameConfNodeImpl *n = idTable[id];
		if (n == NULL)
		{
			Logger::getInstance()->warning("GameConfigs::setFloat - Requested conf id not found.");
			Logger::getInstance()->debug(int2str(id));
		} else {
			assert(n->nodeType == GAMECONFNODE_FLOAT);
			n->floatValue = value;
		}
	}


	void GameConfigs::setString(int id, const char *value)
	{
		assert(id >= 0 && id < GAMECONFIGS_MAX_IDS);
	  GameConfNodeImpl *n = idTable[id];
		if (n == NULL)
		{
			Logger::getInstance()->warning("GameConfigs::setString - Requested conf id not found.");
			Logger::getInstance()->debug(int2str(id));
		} else {
			assert(n->nodeType == GAMECONFNODE_STRING);
			n->stringValue = new char[strlen(value) + 1];
			strcpy(n->stringValue, value);
		}
	}


	GameConfNodeImpl *GameConfigs::getNode(const char *confname)
	{
		// not thread safe.
		confList->resetIterate();
    while (confList->iterateAvailable())
		{
			GameConfNodeImpl *n = (GameConfNodeImpl *)confList->iterateNext();
			if (strcmp(n->confName, confname) == 0)
			{
				return n;
			}
		}
		return NULL;
	}
}

