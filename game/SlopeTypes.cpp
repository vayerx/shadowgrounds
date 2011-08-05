
#include "precompiled.h"

#include "SlopeTypes.h"
#include "GameMap.h"
#include "GameScene.h"

namespace game
{

// the silly hard coded masks used to check what type of slope some position is.
// 0 means no obstacle must be there, 1 means obstacle must be there, ? means whichever is fine

// (5 chars + 1 for null + 2 for alignment)
char slope_type_mask[NUM_SLOPE_TYPES][4][5+1+2] =
{
	// none (default to this when no other matches???)
	{ "?????",
		"??0??", // <--
		"?????",
		"?????" },

	// left 45
	{ "?100?",
		"?110?",  // <--
		"?111?",
		"?????" },

	// right 45
	{ "?001?",
		"?011?",  // <--
		"?111?",
		"?????" },

	// left 22.5 lower
	{ "0000?",
		"1110?",  // <--
		"1111?",
		"?????" },

	// left 22.5 upper
	{ "?0000",
		"11110",  // <--
		"?1111",
		"?????" },

	// right 22.5 lower
	{ "?0000",
		"?0111",  // <--
		"?1111",
		"?????" },

	// right 22.5 upper
	{ "0000?",
		"01111",  // <--
		"1111?",
		"?????" },

	// left 45 temp
	{ "??00?",
		"?110?",  // <--
		"?111?",
		"?????" },

	// right 45 temp
	{ "?00??",
		"?011?",  // <--
		"?111?",
		"?????" },

	// flat
	{ "??0??",
		"?111?", // <--
		"?111?",
		"?????" },

	// inner
	{ "??1??",
		"??1??", // <--
		"??1??",
		"?????" },

};


void solveSlopeType(GameMap *gameMap, GameScene *gameScene, const VC3 &pos, bool &ongroundOut, bool &onslopeOut, SLOPE_TYPE &slopeTypeOut, VC3 &surfacePositionOut)
{
	slopeTypeOut = SLOPE_TYPE_NONE;
	if (gameScene->isBlockedAtScaled(pos.x, pos.z, gameMap->getScaledHeightAt(pos.x, pos.z)))
	{
		float scaleX = gameMap->getScaledSizeX() / gameMap->getObstacleSizeX();
		float scaleY = gameMap->getScaledSizeY() / gameMap->getObstacleSizeY();

		//bool bu = gameScene->isBlockedAtScaled(pos.x, pos.z + scaleY, gameMap->getScaledHeightAt(pos.x, pos.z + scaleY));
		//if (bu)
		//{
			for (int stype = 1; stype < NUM_SLOPE_TYPES; stype++)
			{
				bool mismatch = false;
				for (int z = -1; z < 3; z++)
				{
					for (int x = -2; x < 3; x++)
					{
						int maskX = x + 2;
						//int maskX = 2 - x;
						//int maskZ = 2 - z; // flipping z
						int maskZ = 1 + z; 

						// TODO: should optimize, use obstaclemap directly, using int coordinates
						float chkX = pos.x + scaleX*x;
						float chkZ = pos.z - scaleY*z; // flipping z 
						bool blocked = gameScene->isBlockedAtScaled(chkX, chkZ, gameMap->getScaledHeightAt(chkX, chkZ));

						if (slope_type_mask[stype][maskZ][maskX] != '?')
						{
							if ((slope_type_mask[stype][maskZ][maskX] == '1') == blocked)
							{
								// ok
							} else {
								// mismatch! test next slope type, please.
								x = 2; z = 3;
								mismatch = true;
								break;
							}
						}
					}
				}

				if (!mismatch)
				{
					// found it! set data accordingly...
					static const float mysteryThreshold = 0.15f;
					static const float extraLift = 0.02f;

					switch(stype)
					{
					case SLOPE_TYPE_NONE:
						// nop
						assert(!"This should never happen.");
						break;
					case SLOPE_TYPE_LEFT_45:
					case SLOPE_TYPE_LEFT_45_TEMP:
						{
							// NOTE: relies on +0.5 added by gamemap conversion routines to get center of the obstacle block...
							int obstX = gameMap->scaledToObstacleX(pos.x);
							int obstY = gameMap->scaledToObstacleY(pos.z);
							float offX = pos.x - gameMap->obstacleToScaledX(obstX) + scaleX * 0.5f;
							float offY = pos.z - gameMap->obstacleToScaledY(obstY) + scaleY * 0.5f;
							if (offY <= (scaleX - offX))
							{
								ongroundOut = true;
								onslopeOut = true;
								surfacePositionOut.z = (scaleX - offX) + gameMap->obstacleToScaledY(obstY) - scaleY * 0.5f + extraLift;
							} else {
								if (offY <= (scaleX - offX) + mysteryThreshold)
								{
									ongroundOut = true;
									onslopeOut = true;
									surfacePositionOut.z = (scaleX - offX) + gameMap->obstacleToScaledY(obstY) - scaleY * 0.5f + extraLift;
								}
							}
						}
						stype = SLOPE_TYPE_LEFT_45;
						break;
					case SLOPE_TYPE_RIGHT_45:
					case SLOPE_TYPE_RIGHT_45_TEMP:
						{
							// NOTE: relies on +0.5 added by gamemap conversion routines to get center of the obstacle block...
							int obstX = gameMap->scaledToObstacleX(pos.x);
							int obstY = gameMap->scaledToObstacleY(pos.z);
							float offX = pos.x - gameMap->obstacleToScaledX(obstX) + scaleX * 0.5f;
							float offY = pos.z - gameMap->obstacleToScaledY(obstY) + scaleY * 0.5f;
							if (offY <= offX)
							{
								ongroundOut = true;
								onslopeOut = true;
								surfacePositionOut.z = offX + gameMap->obstacleToScaledY(obstY) - scaleY * 0.5f + extraLift;
							} else {
								if (offY <= offX + mysteryThreshold)
								{
									ongroundOut = true;
									onslopeOut = true;
									surfacePositionOut.z = offX + gameMap->obstacleToScaledY(obstY) - scaleY * 0.5f + extraLift;
								}
							}
						}
						stype = SLOPE_TYPE_RIGHT_45;
						break;
					case SLOPE_TYPE_FLAT:
						{
							ongroundOut = true;
							int obstY = gameMap->scaledToObstacleY(pos.z);
							surfacePositionOut.z = gameMap->obstacleToScaledY(obstY) + scaleY * 0.5f + extraLift;
						}
						break;
					case SLOPE_TYPE_INNER:
						{
							bool bu2 = gameScene->isBlockedAtScaled(pos.x, pos.z + scaleY*2, gameMap->getScaledHeightAt(pos.x, pos.z + scaleY*2));
							if (!bu2)
							{
								ongroundOut = true;
								onslopeOut = true;
								int obstY = gameMap->scaledToObstacleY(pos.z);
								surfacePositionOut.z = gameMap->obstacleToScaledY(obstY) + scaleY * 0.5f + extraLift * 2;

								/*
								VC3 postmp = surfacePositionOut;
								int obstYCheck = gameMap->scaledToObstacleY(postmp.z);
								if (obstYCheck > obstY)
								{
									solveSlopeType(gameMap, gameScene, postmp, ongroundOut, onslopeOut, slopeTypeOut, surfacePositionOut);
									return;
								} else {
									assert(!"This should not happen?");
								}
								*/
							}
						}
						break;
					case SLOPE_TYPE_LEFT_22_5_LOWER:
						{
							// NOTE: relies on +0.5 added by gamemap conversion routines to get center of the obstacle block...
							int obstX = gameMap->scaledToObstacleX(pos.x);
							int obstY = gameMap->scaledToObstacleY(pos.z);
							float offX = pos.x - gameMap->obstacleToScaledX(obstX) + scaleX * 0.5f;
							float offY = pos.z - gameMap->obstacleToScaledY(obstY) + scaleY * 0.5f;
							if (offY <= (scaleX - offX) * 0.5f)
							{
								ongroundOut = true;
								onslopeOut = true;
								surfacePositionOut.z = (scaleX - offX) * 0.5f + gameMap->obstacleToScaledY(obstY) - scaleY * 0.5f + extraLift;
							} else {
								if (offY <= (scaleX - offX) * 0.5f + mysteryThreshold)
								{
									ongroundOut = true;
									onslopeOut = true;
									surfacePositionOut.z = (scaleX - offX) * 0.5f + gameMap->obstacleToScaledY(obstY) - scaleY * 0.5f + extraLift;
								}
							}
						}
						break;
					case SLOPE_TYPE_LEFT_22_5_UPPER:
						{
							// NOTE: relies on +0.5 added by gamemap conversion routines to get center of the obstacle block...
							int obstX = gameMap->scaledToObstacleX(pos.x);
							int obstY = gameMap->scaledToObstacleY(pos.z);
							float offX = pos.x - gameMap->obstacleToScaledX(obstX) + scaleX * 0.5f;
							float offY = pos.z - gameMap->obstacleToScaledY(obstY) + scaleY * 0.5f;
							if (offY <= scaleY * 0.5f + (scaleX - offX) * 0.5f)
							{
								ongroundOut = true;
								onslopeOut = true;
								surfacePositionOut.z = scaleY * 0.5f + (scaleX - offX) * 0.5f + gameMap->obstacleToScaledY(obstY) - scaleY * 0.5f + extraLift;
							} else {
								if (offY <= scaleY * 0.5f + (scaleX - offX) * 0.5f + mysteryThreshold)
								{
									ongroundOut = true;
									onslopeOut = true;
									surfacePositionOut.z = scaleY * 0.5f + (scaleX - offX) * 0.5f + gameMap->obstacleToScaledY(obstY) - scaleY * 0.5f + extraLift;
								}
							}
						}
						break;
					case SLOPE_TYPE_RIGHT_22_5_LOWER:
						{
							// NOTE: relies on +0.5 added by gamemap conversion routines to get center of the obstacle block...
							int obstX = gameMap->scaledToObstacleX(pos.x);
							int obstY = gameMap->scaledToObstacleY(pos.z);
							float offX = pos.x - gameMap->obstacleToScaledX(obstX) + scaleX * 0.5f;
							float offY = pos.z - gameMap->obstacleToScaledY(obstY) + scaleY * 0.5f;
							if (offY <= offX * 0.5f)
							{
								ongroundOut = true;
								onslopeOut = true;
								surfacePositionOut.z = offX * 0.5f + gameMap->obstacleToScaledY(obstY) - scaleY * 0.5f + extraLift;
							} else {
								if (offY <= offX * 0.5f + mysteryThreshold)
								{
									ongroundOut = true;
									onslopeOut = true;
									surfacePositionOut.z = offX * 0.5f + gameMap->obstacleToScaledY(obstY) - scaleY * 0.5f + extraLift;
								}
							}
						}
						break;
					case SLOPE_TYPE_RIGHT_22_5_UPPER:
						{
							// NOTE: relies on +0.5 added by gamemap conversion routines to get center of the obstacle block...
							int obstX = gameMap->scaledToObstacleX(pos.x);
							int obstY = gameMap->scaledToObstacleY(pos.z);
							float offX = pos.x - gameMap->obstacleToScaledX(obstX) + scaleX * 0.5f;
							float offY = pos.z - gameMap->obstacleToScaledY(obstY) + scaleY * 0.5f;
							if (offY <= scaleY * 0.5f + offX * 0.5f)
							{
								ongroundOut = true;
								onslopeOut = true;
								surfacePositionOut.z = scaleY * 0.5f + offX * 0.5f + gameMap->obstacleToScaledY(obstY) - scaleY * 0.5f + extraLift;
							} else {
								if (offY <= scaleY * 0.5f + offX * 0.5f + mysteryThreshold)
								{
									ongroundOut = true;
									onslopeOut = true;
									surfacePositionOut.z = scaleY * 0.5f + offX * 0.5f + gameMap->obstacleToScaledY(obstY) - scaleY * 0.5f + extraLift;
								}
							}
						}
						break;


					} // end of switch

					slopeTypeOut = (SLOPE_TYPE)stype;
					break;
				} // if (!mismatch)

			}
		//} if (!bu)

		//     |     |(pos)|     |
		// ----+-----+-----+-----+----
		//     | lu  |  u  |  ru |
		// ----+-----+-----+-----+----
		// l2u | lu2 |  u2 | ru2 |r2u
		// ----+-----+-----+-----+----
		// l2u2| lu2 |     | ru3 |r2u2
		// ----+-----+-----+-----+----
		//

		/*
		bool bl = game->getGameScene()->isBlockedAtScaled(pos.x - scaleX, pos.z, game->gameMap->getScaledHeightAt(pos.x, pos.z));
		bool br = game->getGameScene()->isBlockedAtScaled(pos.x + scaleX, pos.z, game->gameMap->getScaledHeightAt(pos.x, pos.z));
		bool bu = game->getGameScene()->isBlockedAtScaled(pos.x, pos.z + scaleY, game->gameMap->getScaledHeightAt(pos.x, pos.z));
		if (bu)
		{
			// non-functional stair-code - not needed, use helpers to make slopes instead.
			//if (unit->isGroundFriction())
			//{
				bool bu2 = game->getGameScene()->isBlockedAtScaled(pos.x, pos.z + scaleY*2, game->gameMap->getScaledHeightAt(pos.x, pos.z));
				if (!bu2)
				{
					ongroundOut = true;
					int obstY = game->gameMap->scaledToObstacleY(pos.z);
					surfacePositionOut.z = game->gameMap->obstacleToScaledY(obstY) + scaleY * 0.5f + 0.01f;
				}
			//}
		} else {
			if (bl && br)
			{
				ongroundOut = true;
			} 
			else if (bl || br) 
			{
				// NOTE: relies on +0.5 added by gamemap conversion routines to get center of the obstacle block...
				int obstX = game->gameMap->scaledToObstacleX(pos.x);
				int obstY = game->gameMap->scaledToObstacleY(pos.z);
				float offX = pos.x - game->gameMap->obstacleToScaledX(obstX) + scaleX * 0.5f;
				float offY = pos.z - game->gameMap->obstacleToScaledY(obstY) + scaleY * 0.5f;
				if (bl)
				{
					if (offY <= (scaleX - offX))
					{
						ongroundOut = true;
						onslopeOut = true;
						surfacePositionOut.z = (scaleX - offX) + game->gameMap->obstacleToScaledY(obstY) - scaleY * 0.5f;
					} else {
						if (offY <= (scaleX - offX) + 0.15f)
						{
							ongroundOut = true;
							onslopeOut = true;
							surfacePositionOut.z = (scaleX - offX) + game->gameMap->obstacleToScaledY(obstY) - scaleY * 0.5f;
						}
					}
				} else {
					if (offY <= offX)
					{
						ongroundOut = true;
						onslopeOut = true;
						surfacePositionOut.z = offX + game->gameMap->obstacleToScaledY(obstY) - scaleY * 0.5f;
					} else {
						if (offY <= offX + 0.15f)
						{
							ongroundOut = true;
							onslopeOut = true;
							surfacePositionOut.z = offX + game->gameMap->obstacleToScaledY(obstY) - scaleY * 0.5f;
						}
					}
				}
			} 
		} // if(bu)
		*/
	}
}

}

