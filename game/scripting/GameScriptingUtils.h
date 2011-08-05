
#ifndef GAMESCRIPTINGUTILS_H
#define GAMESCRIPTINGUTILS_H

#include <DatatypeDef.h>

namespace game
{
	class GameMap;

	// you are likely to want to use this.
  bool gs_coordinate_param(GameMap *gameMap, const char *stringData, VC3 *result);

	// unlikely to want to use this (old stuff).
  //bool gs_coordinate_convert(const char *stringData, int *x, int *y, 
	//  bool *scaled, float *scaledX, float *scaledY);

  bool gs_tricoord_param(const char *stringData, VC3 *result);
}

#endif

