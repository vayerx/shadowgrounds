
#include "precompiled.h"

#include "UnitScriptPaths.h"

// damn, this dependency needed because we need to clear the unit's
// path if it is a stored path and we're modifying that.
#include "Unit.h"

#include "../system/Logger.h"

#include "../util/Debug_MemoryManager.h"


#define UNIT_STORED_PATHS_ALLOC 64


namespace game
{

	UnitScriptPaths::UnitScriptPaths()
	{
		allocedPaths = 0;
		storedPaths = NULL;
		storedPathNames = NULL;
		storedPathsUsed = NULL;
		storedPathsIsStart = NULL;
		storedPathsIsEnd = NULL;
		pathStarts = NULL;
		pathEnds = NULL;
		storedPathNumber = 0;

		create();
	}


	UnitScriptPaths::~UnitScriptPaths()
	{
		destroy();
	}


	void UnitScriptPaths::destroy()
	{
		// destroy
		for (int i = 0; i < allocedPaths; i++)
		{
			if (storedPaths[i] != NULL) 
				delete storedPaths[i];
			if (storedPathNames[i] != NULL) 
				delete [] storedPathNames[i];
		}
		allocedPaths = 0;
		delete [] storedPaths;
		delete [] storedPathNames;
		delete [] storedPathsUsed;
		delete [] storedPathsIsStart;
		delete [] storedPathsIsEnd;
		delete [] pathStarts;
		delete [] pathEnds;
	}


	void UnitScriptPaths::create()
	{
		// create
		allocedPaths = UNIT_STORED_PATHS_ALLOC;

		storedPaths = new frozenbyte::ai::Path *[allocedPaths];
		storedPathNames = new char *[allocedPaths];
		storedPathsUsed = new bool[allocedPaths];
		storedPathsIsStart = new bool[allocedPaths];
		storedPathsIsEnd = new bool[allocedPaths];
		pathStarts = new VC3[allocedPaths];
		pathEnds = new VC3[allocedPaths];
		for (int i = 0; i < allocedPaths; i++)
		{
			storedPathsUsed[i] = false;
			storedPaths[i] = NULL;
			storedPathNames[i] = NULL;
			storedPathsIsStart[i] = false;
			storedPathsIsEnd[i] = false;
			pathStarts[i] = VC3(0,0,0);
			pathEnds[i] = VC3(0,0,0);
		}
		storedPathNumber = 0;
	}

	void UnitScriptPaths::clear()
	{
		destroy();
		create();
	}

	void UnitScriptPaths::setStoredPathNumber(int pathNumber)
	{
		this->storedPathNumber = pathNumber;
	}


	int UnitScriptPaths::getStoredPathNumber()
	{ 
		return storedPathNumber;
	}


	frozenbyte::ai::Path *UnitScriptPaths::getStoredPathForUse(int pathNumber)
	{
		if (pathNumber < 0 || pathNumber >= allocedPaths)
		{
			Logger::getInstance()->warning("Unit::getStoredPathForUse - Path number out of range.");
			return NULL;
		}
		if (!storedPathsUsed[pathNumber])
		{
			Logger::getInstance()->warning("Unit::getStoredPathForUse - Requested stored path does not exist.");
			return NULL;
		}
		if (storedPaths[pathNumber] == NULL)
		{
			//if (!storedPathsIsStart[pathNumber] && !storedPathsIsEnd[pathNumber])
			Logger::getInstance()->debug("Unit::getStoredPathForUse - Null path encountered.");
			return NULL;
		}
		return storedPaths[pathNumber];
	}


	void UnitScriptPaths::moveStoredPath(int fromPathNumber, int toPathNumber)
	{
		if (toPathNumber < 0 || toPathNumber >= allocedPaths)
		{
			Logger::getInstance()->warning("Unit::moveStoredPath - Target path number out of range.");
			return;
		}
		if (fromPathNumber < 0 || fromPathNumber >= allocedPaths)
		{
			Logger::getInstance()->warning("Unit::moveStoredPath - Source path number out of range.");
			return;
		}
		if (fromPathNumber == toPathNumber)
			return;
		if (!storedPathsUsed[fromPathNumber])
		{
			Logger::getInstance()->warning("Unit::moveStoredPath - Requested stored path does not exist.");
			return;
		}
		if (storedPathsUsed[toPathNumber])
		{
			if (storedPaths[toPathNumber] != NULL)
				delete storedPaths[toPathNumber];
			if (storedPathNames[toPathNumber] != NULL)
				delete [] storedPathNames[toPathNumber];
			Logger::getInstance()->debug("Unit::moveStoredPath - Replacing old stored path.");
			//return;
		}
		storedPathsUsed[toPathNumber] = storedPathsUsed[fromPathNumber];
		storedPaths[toPathNumber] = storedPaths[fromPathNumber];
		storedPathNames[toPathNumber] = storedPathNames[fromPathNumber];
		storedPathsIsStart[toPathNumber] = storedPathsIsStart[fromPathNumber];
		storedPathsIsEnd[toPathNumber] = storedPathsIsEnd[fromPathNumber];
		pathStarts[toPathNumber] = pathStarts[fromPathNumber];
		pathEnds[toPathNumber] = pathEnds[fromPathNumber];
		
		storedPathsUsed[fromPathNumber] = false;
		storedPaths[fromPathNumber] = NULL;
		storedPathNames[fromPathNumber] = NULL;
	}


	void UnitScriptPaths::setStoredPath(int pathNumber, frozenbyte::ai::Path *path,
		const VC3 &startPosition, const VC3 &endPosition, Unit *unit)
	{
		if (pathNumber < 0 || pathNumber >= allocedPaths)
		{
			Logger::getInstance()->warning("Unit::setStoredPath - Path number out of range.");
			return;
		}
		if (storedPathsUsed[pathNumber])
		{
			if (pathNumber != 0)
			{
				Logger::getInstance()->debug("Unit::setStoredPath - Replacing old stored path.");
			}
			if (storedPaths[pathNumber] != NULL)
			{
				// NOTE: because of this, we depend on Unit class...
				// should solve this somehow nicely.
				if (unit->getPath() == storedPaths[pathNumber]) 
					unit->setPath(NULL);
				delete storedPaths[pathNumber];
			}
			if (storedPathNames[pathNumber] != NULL)
			{
				delete [] storedPathNames[pathNumber];
				storedPathNames[pathNumber] = NULL;
			}
		}
		storedPathsUsed[pathNumber] = true;
		storedPaths[pathNumber] = path;
		storedPathsIsStart[pathNumber] = false;
		storedPathsIsEnd[pathNumber] = false;
		pathStarts[pathNumber] = startPosition;
		pathEnds[pathNumber] = endPosition;
	}


	void UnitScriptPaths::setStoredPathStart(int pathNumber, const VC3 &position,
		Unit *unit, const char *pathName)
	{
		if (pathNumber < 0 || pathNumber >= allocedPaths)
		{
			Logger::getInstance()->warning("Unit::setStoredPathStart - Path number out of range.");
			return;
		}
		if (storedPathsUsed[pathNumber])
		{
			if (pathNumber != 0)
			{
				Logger::getInstance()->debug("Unit::setStoredPathStart - Replacing old stored path.");
			}
			if (storedPaths[pathNumber] != NULL)
			{
				// NOTE: because of this, we depend on Unit class...
				// should solve this somehow nicely.
				if (unit->getPath() == storedPaths[pathNumber]) 
					unit->setPath(NULL);
				delete storedPaths[pathNumber];
			}
			if (storedPathNames[pathNumber] != NULL)
			{
				delete [] storedPathNames[pathNumber];
				storedPathNames[pathNumber] = NULL;
			}
		}
		storedPathsUsed[pathNumber] = true;
		storedPaths[pathNumber] = NULL;
		storedPathsIsStart[pathNumber] = true;
		storedPathsIsEnd[pathNumber] = false;
		pathStarts[pathNumber] = position;
		pathEnds[pathNumber] = position;
		if (pathName != NULL)
		{
			storedPathNames[pathNumber] = new char[strlen(pathName) + 1];
			strcpy(storedPathNames[pathNumber], pathName);
		}
	}


	void UnitScriptPaths::setStoredPathEnd(int pathNumber, const VC3 &position,
		Unit *unit)
	{
		if (pathNumber < 0 || pathNumber >= allocedPaths)
		{
			Logger::getInstance()->warning("Unit::setStoredPathEnd - Path number out of range.");
			return;
		}
		if (storedPathsUsed[pathNumber])
		{
			if (pathNumber != 0)
			{
				Logger::getInstance()->debug("Unit::setStoredPathEnd - Replacing old stored path.");
			}
			if (storedPaths[pathNumber] != NULL)
			{
				// NOTE: because of this, we depend on Unit class...
				// should solve this somehow nicely.
				if (unit->getPath() == storedPaths[pathNumber]) 
					unit->setPath(NULL);
				delete storedPaths[pathNumber];
			}
			if (storedPathNames[pathNumber] != NULL)
			{
				delete [] storedPathNames[pathNumber];
				storedPathNames[pathNumber] = NULL;
			}
		}
		storedPathsUsed[pathNumber] = true;
		storedPaths[pathNumber] = NULL;
		storedPathsIsStart[pathNumber] = false;
		storedPathsIsEnd[pathNumber] = true;
		pathStarts[pathNumber] = position;
		pathEnds[pathNumber] = position;
	}


	bool UnitScriptPaths::isStoredPathUsed(int pathNumber)
	{
		if (pathNumber < 0 || pathNumber >= allocedPaths)
		{
			Logger::getInstance()->warning("Unit::isStoredPathUsed - Path number out of range.");
			return false;
		}
		return storedPathsUsed[pathNumber];
	}


	bool UnitScriptPaths::isStoredPathStart(int pathNumber)
	{
		if (pathNumber < 0 || pathNumber >= allocedPaths)
		{
			Logger::getInstance()->warning("Unit::isStoredPathStart - Path number out of range.");
			return false;
		}
		if (!storedPathsUsed[pathNumber])
		{
			Logger::getInstance()->warning("Unit::isStoredPathStart - Requested stored path does not exist.");
			return false;
		}
		return storedPathsIsStart[pathNumber];
	}


	bool UnitScriptPaths::isStoredPathEnd(int pathNumber)
	{
		if (pathNumber < 0 || pathNumber >= allocedPaths)
		{
			Logger::getInstance()->warning("Unit::isStoredPathEnd - Path number out of range.");
			return true; // false;
		}
		if (!storedPathsUsed[pathNumber])
		{
			Logger::getInstance()->warning("Unit::isStoredPathEnd - Requested stored path does not exist.");
			return true; // false;
		}
		return storedPathsIsEnd[pathNumber];
	}


	frozenbyte::ai::Path *UnitScriptPaths::getStoredPath(int pathNumber)
	{
		if (pathNumber < 0 || pathNumber >= allocedPaths)
		{
			Logger::getInstance()->warning("Unit::getStoredPath - Path number out of range.");
			return NULL;
		}
		if (!storedPathsUsed[pathNumber])
		{
			Logger::getInstance()->warning("Unit::getStoredPath - Requested stored path does not exist.");
			return NULL;
		}
		return storedPaths[pathNumber];
	}


	const char *UnitScriptPaths::getStoredPathName(int pathNumber)
	{
		if (pathNumber < 0 || pathNumber >= allocedPaths)
		{
			Logger::getInstance()->warning("Unit::getStoredPathName - Path number out of range.");
			return NULL;
		}
		if (!storedPathsUsed[pathNumber])
		{
			Logger::getInstance()->warning("Unit::getStoredPathName - Requested stored path does not exist.");
			return NULL;
		}
		return storedPathNames[pathNumber];
	}


	VC3 UnitScriptPaths::getStoredPathStartPosition(int pathNumber)
	{
		if (pathNumber < 0 || pathNumber >= allocedPaths)
		{
			Logger::getInstance()->warning("Unit::getStoredPathStartPosition - Path number out of range.");
			return VC3(0,0,0);
		}
		if (!storedPathsUsed[pathNumber])
		{
			Logger::getInstance()->warning("Unit::getStoredPathStartPosition - Requested stored path does not exist.");
			return VC3(0,0,0);
		}
		return pathStarts[pathNumber];
	}


	VC3 UnitScriptPaths::getStoredPathEndPosition(int pathNumber)
	{
		if (pathNumber < 0 || pathNumber >= allocedPaths)
		{
			Logger::getInstance()->warning("Unit::getStoredPathEndPosition - Path number out of range.");
			return VC3(0,0,0);
		}
		if (!storedPathsUsed[pathNumber])
		{
			Logger::getInstance()->warning("Unit::getStoredPathEndPosition - Requested stored path does not exist.");
			return VC3(0,0,0);
		}
		return pathEnds[pathNumber];
	}


	int UnitScriptPaths::getAllocatedStoredPaths()
	{
		return allocedPaths;
	}

}


