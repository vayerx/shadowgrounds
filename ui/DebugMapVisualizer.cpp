
#include "precompiled.h"

#include "DebugMapVisualizer.h"
#include "DebugVisualizerTextUtil.h"
#include "../game/SlopeTypes.h"
#include "../game/GameMap.h"
#include "../game/GameScene.h"
#include "../game/options/options_debug.h"

#include <vector>
#include <Storm3D_UI.h>

#define DEBUGMAPVISUALIZER_MAX_DIST 10.0f
//#define DEBUGMAPVISUALIZER_EXTENDED_MAX_DIST 10.0f

using namespace game;

extern IStorm3D_Scene *disposable_scene;

namespace ui
{

	void DebugMapVisualizer::visualizeObstacleMap(game::GameMap *gameMap, game::GameScene *gameScene, const VC3 &cameraPosition)
	{
		assert(gameMap != NULL);

		//char textbuf[128];

		for (int oy = 0; oy < gameMap->getObstacleSizeY(); oy++)
		{
			for (int ox = 0; ox < gameMap->getObstacleSizeX(); ox++)
			{
				float sx = gameMap->obstacleToScaledX(ox);
				float sy = gameMap->obstacleToScaledY(oy);

				if (fabsf(sx - cameraPosition.x) < DEBUGMAPVISUALIZER_MAX_DIST
					&& fabsf(sy - cameraPosition.z) < DEBUGMAPVISUALIZER_MAX_DIST)
				{
					int oheight = gameMap->getObstacleHeight(ox, oy);
					float hmap_sheight = gameMap->getScaledHeightAt(sx, sy);

					float obst_sheight = oheight * gameMap->getScaleHeight();

					VC3 blockpos = VC3(sx, hmap_sheight, sy);
					VC3 blocksize = VC3(gameMap->getScaleX() * (0.5f / GAMEMAP_HEIGHTMAP_MULTIPLIER / GAMEMAP_PATHFIND_ACCURACY), obst_sheight, gameMap->getScaleY() * (0.5f / GAMEMAP_HEIGHTMAP_MULTIPLIER / GAMEMAP_PATHFIND_ACCURACY));

					VC3 pos = blockpos;
					VC3 sizes = VC3(blocksize.x, blocksize.y, blocksize.z);

					bool fullBox = false;
					COL colBg = COL(0,0,0.2f);
					COL colBlocked = COL(0,0,1.0f);

					if (oheight > 0)
					{
						fullBox = true;
					}

					//VC3 distToCamVec = pos - cameraPosition;
					//float distToCamSq = distToCamVec.GetSquareLength();

					//if (distToCamSq < (DEBUGMAPVISUALIZER_MAX_DIST*DEBUGMAPVISUALIZER_MAX_DIST)
					{

						VC3 c1 = VC3(pos.x - sizes.x, pos.y + sizes.y, pos.z + sizes.z);
						VC3 c2 = VC3(pos.x + sizes.x, pos.y + sizes.y, pos.z + sizes.z);
						VC3 c3 = VC3(pos.x + sizes.x, pos.y + sizes.y, pos.z - sizes.z);
						VC3 c4 = VC3(pos.x - sizes.x, pos.y + sizes.y, pos.z - sizes.z);

#ifdef GAME_SIDEWAYS
						if (oheight > 0)
						{
							VC3 dummy = pos;
							bool onground = false;
							bool onslope = false;
							SLOPE_TYPE slopeType = SLOPE_TYPE_NONE;
							solveSlopeType(gameMap, gameScene, pos, onground, onslope, slopeType, dummy);
							switch(slopeType)
							{
							case SLOPE_TYPE_NONE:
								// this should not happen?
								disposable_scene->AddLine(c1, c2, COL(1,0,0));
								disposable_scene->AddLine(c2, c3, COL(1,0,0));
								disposable_scene->AddLine(c3, c4, COL(1,0,0));
								disposable_scene->AddLine(c4, c1, COL(1,0,0));
								break;
							case SLOPE_TYPE_FLAT:
								disposable_scene->AddLine(c1, c2, colBlocked);
								disposable_scene->AddLine(c1, c1 + VC3(0,-2,0), colBlocked);
								disposable_scene->AddLine(c2, c3, colBg);
								break;
							case SLOPE_TYPE_INNER:
								{
									COL colInner = COL(0,0,0.6f);
									disposable_scene->AddLine(c1, c2, colInner);
									disposable_scene->AddLine(c2, c3, colInner);
									disposable_scene->AddLine(c3, c4, colInner);
									disposable_scene->AddLine(c4, c1, colInner);
								}
								break;
							case SLOPE_TYPE_LEFT_45:
								disposable_scene->AddLine(c1, c2, colBg);
								disposable_scene->AddLine(c2, c3, colBg);
								disposable_scene->AddLine(c1, c3, colBlocked);
								disposable_scene->AddLine(c1, c1 + VC3(0,-2,0), colBlocked);
								break;
							case SLOPE_TYPE_RIGHT_45:
								disposable_scene->AddLine(c1, c2, colBg);
								disposable_scene->AddLine(c2, c3, colBg);
								disposable_scene->AddLine(c2, c4, colBlocked);
								disposable_scene->AddLine(c2, c2 + VC3(0,-2,0), colBlocked);
								break;
							case SLOPE_TYPE_LEFT_22_5_LOWER:
								disposable_scene->AddLine(c1, c2, colBg);
								disposable_scene->AddLine(c2, c3, colBg);
								disposable_scene->AddLine((c1+c4)*0.5f, c3, colBlocked);
								disposable_scene->AddLine((c1+c4)*0.5f, (c1+c4)*0.5f + VC3(0,-2,0), colBlocked);
								break;
							case SLOPE_TYPE_LEFT_22_5_UPPER:
								disposable_scene->AddLine(c1, c2, colBg);
								disposable_scene->AddLine(c2, c3, colBg);
								disposable_scene->AddLine(c1, (c2+c3)*0.5f, colBlocked);
								disposable_scene->AddLine(c1, c1 + VC3(0,-2,0), colBlocked);
								break;
							case SLOPE_TYPE_RIGHT_22_5_LOWER:
								disposable_scene->AddLine(c1, c2, colBg);
								disposable_scene->AddLine(c2, c3, colBg);
								disposable_scene->AddLine((c2+c3)*0.5f, c4, colBlocked);
								disposable_scene->AddLine((c2+c3)*0.5f, (c2+c3)*0.5f + VC3(0,-2,0), colBlocked);
								break;
							case SLOPE_TYPE_RIGHT_22_5_UPPER:
								disposable_scene->AddLine(c1, c2, colBg);
								disposable_scene->AddLine(c2, c3, colBg);
								disposable_scene->AddLine(c2, (c1+c4)*0.5f, colBlocked);
								disposable_scene->AddLine(c2, c2 + VC3(0,-2,0), colBlocked);
								break;

							};
						} else {
							disposable_scene->AddLine(c1, c2, colBg);
							disposable_scene->AddLine(c2, c3, colBg);
						}
#else
						COL col = colBg;
						if (fullBox)
						{
							col = colBlocked;
						}
						disposable_scene->AddLine(c1, c2, col);
						disposable_scene->AddLine(c2, c3, col);
						if (fullBox)
						{
							disposable_scene->AddLine(c3, c4, col);
							disposable_scene->AddLine(c4, c1, col);
						}
#endif
					}

					/*
					int textoffy = 0;

					if (game::SimpleOptions::getBool(DH_OPT_B_DEBUG_VISUALIZE_OBSTACLEMAP_EXTENDED)
						&& distToCamSq < (DEBUGMAPVISUALIZER_EXTENDED_MAX_DIST*DEBUGMAPVISUALIZER_EXTENDED_MAX_DIST))
					{
						sprintf(textbuf, "h: %d", 123);
						DebugVisualizerTextUtil::renderText(pos, 0, textoffy, textbuf);
						textoffy += 16;
					}
					*/
				}
			}
		}
	}

}
