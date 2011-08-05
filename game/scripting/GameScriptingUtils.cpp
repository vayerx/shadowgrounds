
#include "precompiled.h"

#include "GameScriptingUtils.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "../Game.h"
#include "../GameMap.h"
#include "../../convert/str2int.h"

#include "../../util/Debug_MemoryManager.h"


namespace game
{

	bool gs_coordinate_convert(const char *stringData, int *x, int *y, 
		bool *scaled, float *scaledX, float *scaledY)
	{
		const unsigned int buffer_size = 80; // original 40, but that caused some problems
		if (strlen(stringData) < buffer_size)
		{
			int numStart = 0;
			if (stringData[0] == 's' && stringData[1] == ',')
			{
				*scaled = true;
				numStart = 2;
			}

			char splitbuf[buffer_size];
			strcpy(splitbuf, stringData);
			int buflen = strlen(splitbuf);
			for (int splitp = numStart; splitp < buflen; splitp++)
			{
				if (splitbuf[splitp] == ',')
				{
					splitbuf[splitp] = '\0';

					if (*scaled)
					{
						*scaledX = (float)atof(&splitbuf[numStart]);
					} else {
						*x = str2int(&splitbuf[numStart]);
						if (str2int_errno() != 0)
							return false;
					}

					if (*scaled)
					{
						*scaledY = (float)atof(&splitbuf[splitp + 1]);
					} else {
						*y = str2int(&splitbuf[splitp + 1]);
						if (str2int_errno() != 0)
							return false;
					}

					break;
				}
			}
			return true;
		} else {
			assert(!"gs_coordinate_convert - string too long.");
			return false;
		}
	}



	bool gs_coordinate_param(GameMap *gameMap, const char *stringData, 
		VC3 *result)
	{
		if (stringData != NULL)
		{
			int px = 0;
			int py = 0;
			bool scaled = false;
			float sx = 0;
			float sy = 0;
			if (gs_coordinate_convert(stringData, &px, &py, &scaled, &sx, &sy))
			{
				if ((px <= 0 || py <= 0) && !scaled)
				{
					//sp->error("DecorScripting::process - Bad setDecorPosition coordinates.");
					return false;
				} else {
					float x;
					float y; 
					if (scaled)
					{
						x = sx;
						y = sy;
					} else {
						x = gameMap->configToScaledX(px);
						y = gameMap->configToScaledY(py);
					}
					if (!gameMap->isInScaledBoundaries(x, y))
					{
						return false;
					}
					result->x = x;
					result->y = 0;
					result->z = y;
					return true;
				}
			} else {
				//sp->error("DecorScripting::process - setDecorPosition parameter format bad.");
				return false;
			}
		} else {
			//sp->error("DecorScripting::process - Missing setDecorPosition coordinates.");
			return false;
		} 	
	}


	bool gs_tricoord_param(const char *stringData, 
		VC3 *result)
	{
		if (stringData == NULL)
		{
			assert(!"gs_tricoord_param - null string (caller should check for it!).");
			return false;
		}
		if (strlen(stringData) < 64)
		{
			float x = 0, y = 0, z = 0;
			int paramNum = 0;
			int prevSplit = 0;
			char splitbuf[64];
			strcpy(splitbuf, stringData);
			int buflen = strlen(splitbuf);
			for (int splitp = 0; splitp < buflen; splitp++)
			{
				if (splitbuf[splitp] == ',')
				{
					splitbuf[splitp] = '\0';

					if (paramNum == 0)
					{
						x = (float)atof(&splitbuf[0]);
						prevSplit = splitp;
					} else {
						y = (float)atof(&splitbuf[prevSplit + 1]);
						z = (float)atof(&splitbuf[splitp + 1]);
					}

					paramNum++;
					if (paramNum >= 2)
					{
						break;
					}
				}
			}
			*result = VC3(x,y,z);
			return true;
		} else {
			assert(!"gs_tricoord_param - string too long.");
			return false;
		} 	
	}



}

