
#ifndef WORLDFOLD_H
#define WORLDFOLD_H

#include "DatatypeDef.h"

class WorldFold
{
public:
	static void initWorldFold(float foldStep);
	static void uninitWorldFold();

	static void resetWorldFold();

	static const MAT *getWorldFoldForPosition(const VC3 &position);

	// returns a "unique" key to identify when matrix changes (without having to compare the actual matrix data)
	static const int *getWorldFoldKeyForPosition(const VC3 &position);

	// NOTE: this should be called before adding any folds. (at least in current implementation)
	static void setWorldFoldCenter(const VC3 &position);

	static void addWorldFoldAtPosition(const VC3 &position, const MAT &fold);

	static void changeWorldFoldAtPosition(const VC3 &position, const MAT &fold);

	static void setCameraPosition(const VC3 &position);

};

#endif
