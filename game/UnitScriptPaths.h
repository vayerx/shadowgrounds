
#ifndef UNITSCRIPTPATHS_H
#define UNITSCRIPTPATHS_H

#include <DatatypeDef.h>

#include "../util/AI_PathFind.h"

namespace game
{
  class Unit; // :(


  class UnitScriptPaths
  {
  public:
    /**
     * Default constructor.
     */
    UnitScriptPaths();

    /**
     * Destructor, needed to deallocate memory used for stored paths
     */
    ~UnitScriptPaths();

    /**
     * Set the currently selected stored path.
     * Used by scripts to select a current path for a unit.
     */    
    void setStoredPathNumber(int pathNumber);

    /**
     * Get the currently selected stored path.
     */    
    int getStoredPathNumber();

    /**
     * Just returns a stored path for usage (in the Unit class).
     * The returned path must not be deleted. (Unit class should mark
     * the path being a stored one) 
     * Much like getStoredPath, but this one makes some checks so that
     * the path really is valid for use.
     */
    frozenbyte::ai::Path *getStoredPathForUse(int pathNumber);

    /**
     * Stores a path.
     */
    void setStoredPath(int pathNumber, frozenbyte::ai::Path *path,
      const VC3 &startPosition, const VC3 &endPosition, Unit *unit);

    void setStoredPathStart(int pathNumber, const VC3 &position, Unit *unit,
			const char *pathName);

    void setStoredPathEnd(int pathNumber, const VC3 &position, Unit *unit);

    bool isStoredPathStart(int pathNumber);

    bool isStoredPathEnd(int pathNumber);

    bool isStoredPathUsed(int pathNumber);

    frozenbyte::ai::Path *getStoredPath(int pathNumber);

    const char *getStoredPathName(int pathNumber);

    VC3 getStoredPathStartPosition(int pathNumber);

    VC3 getStoredPathEndPosition(int pathNumber);

    void moveStoredPath(int fromPathNumber, int toPathNumber);

    int getAllocatedStoredPaths();

		// clears all data
		void clear();

  private:

		void create();
		void destroy();

		int storedPathNumber;
    int allocedPaths;
    bool *storedPathsUsed;
    bool *storedPathsIsStart;
    bool *storedPathsIsEnd;
    frozenbyte::ai::Path **storedPaths;
    char **storedPathNames;
    VC3 *pathStarts;
    VC3 *pathEnds;
  };

}

#endif


