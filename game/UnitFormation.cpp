
#include "precompiled.h"

#include "UnitFormation.h"

#include "UnitActor.h"
#include "Unit.h"
#include "UnitType.h"
#include "GameMap.h"
#include "GameScene.h"
#include "SimpleOptions.h"
#include "options/options_tactical.h"
#include "CoverFinder.h"
//#include "HideMap.h"
#include "unittypes.h"
#include "ConnectionChecker.h"

// for debug
#include "../system/Logger.h"
#include "../convert/str2int.h"

#include <vector>

#include "../util/Debug_MemoryManager.h"


namespace game 
{

struct UnitFormationData
{
	UnitFormation::FormationType currentFormation;

	UnitFormationData()
	:	currentFormation (UnitFormation::FormationFoo),
		gameMap(0),
		gameScene(0)
	{
	}

	GameMap *gameMap;
	GameScene *gameScene;
};

UnitFormation::UnitFormation()
{
	data = new UnitFormationData();
}

void UnitFormation::setGameMap(GameMap *gameMap)
{
	data->gameMap = gameMap;
}

void UnitFormation::setGameScene(GameScene *gameScene)
{
	data->gameScene = gameScene;
}

UnitFormation::~UnitFormation()
{
	delete data;
}

// Mutators

void UnitFormation::addMovePoint(std::vector<Unit *> *units, 
	const VC3 &scaledMapPos, Unit::MoveType moveType)
{
	// If some units have waypoints set, they are overriden

	// If all units have same amount of waypoints, just add
	//	-- Check if too far away from each other??
	// Just override all move points (and target?)

	int i;
	VC3 avgPos = VC3(0,0,0);
	for (i = 0; i < (int)units->size(); i++)
	{
		Unit *u = (*units)[i];
		avgPos += u->getPosition();
	}
	avgPos /= (float)units->size();

	// positions already taken by a unit
	// (so that others won't try to go there too)
	std::vector<std::pair<int, int> > pointsTaken;

	bool clickedBlocked = data->gameScene->isBlockedAtScaled(
		scaledMapPos.x, scaledMapPos.z, 0);

	IStorm3D_Model *buildingModel = NULL;
	ConnectionChecker *connectionChecker = NULL;
	if (units->size() > 1 || clickedBlocked)
	{
		int px = data->gameMap->scaledToPathfindX(scaledMapPos.x);
		int py = data->gameMap->scaledToPathfindY(scaledMapPos.z);		
		buildingModel = data->gameScene->getBuildingModelAtPathfind(px, py);

		// HACK:
		// ok, did we click blocked and not inside building (building wall maybe)?
		// now check if there is a building nearby..
		if (clickedBlocked && buildingModel == NULL)
		{
			px+=2;
			buildingModel = data->gameScene->getBuildingModelAtPathfind(px, py);

			px-=4;
			if (buildingModel == NULL && data->gameMap->inPathfindBoundaries(px, py))
			{
				buildingModel = data->gameScene->getBuildingModelAtPathfind(px, py);
			}

			px+=2;
			py+=2;
			if (buildingModel == NULL && data->gameMap->inPathfindBoundaries(px, py))
			{
				buildingModel = data->gameScene->getBuildingModelAtPathfind(px, py);
			}

			py-=4;
			if (buildingModel == NULL && data->gameMap->inPathfindBoundaries(px, py))
			{
				buildingModel = data->gameScene->getBuildingModelAtPathfind(px, py);
			}
		}

		// smaller "rooms" inside buildings...
		int connDist = 14;
		if (buildingModel != NULL) connDist = 10;
		connectionChecker = new ConnectionChecker(data->gameScene, px, py, connDist);
	}

	for (i = 0; i < (int)units->size(); i++)
	{
		Unit *u = (*units)[i];

		UnitActor *ua = getUnitActorForUnit(u);
		
    VC3 offset = u->getPosition() - avgPos;
		offset.y = 0;

		float offsetLen = offset.GetLength();
		if (offsetLen > 0.01f)
		{
			offset.Normalize();
		}

		float destX, okDestX;
		float destZ, okDestZ;

		//okDestX = scaledMapPos.x + offset.x;
		//okDestZ = scaledMapPos.z + offset.z;
		okDestX = scaledMapPos.x;
		okDestZ = scaledMapPos.z;

		float minDist = 0.5f;
		float maxDist = 3.0f;

		if (units->size() == 1) 
		{
			minDist = 0;
			maxDist = 1.0f;
		} else {
			minDist += (float)units->size() * 0.2f;
			maxDist += (float)units->size() * 0.2f;
		}

		bool nonBlockedPassed = false;

		// first solve a position that would kinda match current position
		// in formation... (from formation center to some direction)
		for (float distfactor = minDist; distfactor <= maxDist; distfactor += 0.1f)
		{
			if (distfactor > offsetLen)
				break;

			destX = scaledMapPos.x + (offset.x * distfactor);
			destZ = scaledMapPos.z + (offset.z * distfactor);

			if (data->gameMap->isWellInScaledBoundaries(destX, destZ))
			{
				int ox = data->gameMap->scaledToObstacleX(destX);
				int oy = data->gameMap->scaledToObstacleY(destZ);
				//if (data->gameMap->getObstacleHeight(ox, oy) == 0)
				if (!data->gameScene->isBlocked(ox, oy, 0))
				{
					okDestX = destX;
					okDestZ = destZ;
					nonBlockedPassed = true;
				} else {
					if (nonBlockedPassed)
					{
						break;
					}
				}
			}
		}

		// then try to adjust position so that it would be nicely
		// covered from enemy...
		if (units->size() > 1 || clickedBlocked)
		{			
			int ox = data->gameMap->scaledToObstacleX(okDestX);
			int oy = data->gameMap->scaledToObstacleY(okDestZ);

			/*
			if (connectionChecker != NULL)
			{
				delete connectionChecker;
			}
			connectionChecker = new ConnectionChecker(data->gameScene, ox, oy, 8);
			*/

			VC3 pos = u->getPosition();
			int moveDirX = ox - data->gameMap->scaledToObstacleX(pos.x);
			int moveDirY = oy - data->gameMap->scaledToObstacleY(pos.z);
			int coverFromX = ox + moveDirX;
			int coverFromY = oy + moveDirY;

			if (u->targeting.hasTarget() && u->targeting.getTargetUnit() != NULL)
			{
				VC3 targPos = u->targeting.getTargetUnit()->getPosition();
				coverFromX = data->gameMap->scaledToObstacleX(targPos.x);
				coverFromY = data->gameMap->scaledToObstacleY(targPos.z);
			} else {
				if (u->getSeeUnit() != NULL)
				{
					VC3 seePos = u->getSeeUnit()->getPosition();
					coverFromX = data->gameMap->scaledToObstacleX(seePos.x);
					coverFromY = data->gameMap->scaledToObstacleY(seePos.z);
				}
			}

			int tx1 = ox - 8;
			int ty1 = oy - 8;
			int tx2 = ox + 8;
			int ty2 = oy + 8;
			if (tx1 < 0) tx1 = 0;
			if (ty1 < 0) ty1 = 0;
			if (tx2 >= data->gameMap->getObstacleSizeX()) tx2 = data->gameMap->getObstacleSizeX() - 1;
			if (ty2 >= data->gameMap->getObstacleSizeX()) ty2 = data->gameMap->getObstacleSizeY() - 1;
			int preferX = ox;
			int preferY = oy;
			int preferDistFactorSq = 9999*9999;
			for (int ty = ty1; ty <= ty2; ty++)
			{
				for (int tx = tx1; tx <= tx2; tx++)
				{
					int factSq;
					if (CoverFinder::isCoveredFrom(data->gameMap->getCoverMap(), tx, ty, coverFromX, coverFromY))
					{
						factSq = (tx - ox)*(tx - ox) + (ty - oy)*(ty - oy); 
/*
					}
					else if (data->gameMap->getHideMap()->getHiddenessAt(tx, ty) == HideMap::maxHiddeness)
					{
						// hiddeness not quite as important as covered.
						factSq = 2*(tx - ox)*(tx - ox) + 2*(ty - oy)*(ty - oy);
						factSq += 3*3;
*/
					} else {
					  // no hiddeness, no cover, not preferred very much...
						factSq = 4*(tx - ox)*(tx - ox) + 4*(ty - oy)*(ty - oy);
						factSq += 6*6;
					}

					if (factSq < preferDistFactorSq)
					{
						// penalty for points are already taken by another unit
						int pointAmount = pointsTaken.size();
						for (int i = 0; i < pointAmount; i++)
						{
							if (abs(pointsTaken[i].first - tx) <= 2
								&& abs(pointsTaken[i].second - ty) <= 2)
							{
								if (abs(pointsTaken[i].first - tx) <= 1
									&& abs(pointsTaken[i].second - ty) <= 1)
								{
									if (pointsTaken[i].first == tx
										&& pointsTaken[i].second == ty)
										factSq += 6*6;
									else
										factSq += 4*4;
								} else {
									factSq += 2*2;
								}
							}
						}
					}

					// penalty if not in same building area
					if (factSq < preferDistFactorSq)
					{
						if (data->gameScene->getBuildingModelAtPathfind(tx, ty) != buildingModel)
						{
							factSq += 40*40;
						}
					}

					// penalty if not in the same "room"
					if (factSq < preferDistFactorSq)
					{
						if (!connectionChecker->isCenterConnectedTo(tx - ox, ty - oy))
						{
							factSq += 30*30;
						}
					}

					// blocked really gets penalty :)
					if (data->gameScene->isBlockedAtScaled(
						data->gameMap->obstacleToScaledX(tx), 
						data->gameMap->obstacleToScaledY(ty), 0))
					{
						factSq += 100*100;
					}

					if (factSq < preferDistFactorSq)
					{
						preferDistFactorSq = factSq;
						preferX = tx;
						preferY = ty;
					}
				}
			}
			okDestX = data->gameMap->obstacleToScaledX(preferX);
			okDestZ = data->gameMap->obstacleToScaledY(preferY);

			pointsTaken.push_back(std::pair<int, int>(preferX, preferY));
		}

		// now move right next to any obstacle...
		for (int i = 1; i < 10; i++)
		{
			// right?
			{
				float adjustedX = okDestX + ((float)i / 10.0f);
				float adjustedZ = okDestZ;
				int ox = data->gameMap->scaledToObstacleX(adjustedX);
				int oy = data->gameMap->scaledToObstacleY(adjustedZ);
				//if (data->gameMap->getObstacleHeight(ox, oy) != 0)
				if (data->gameScene->isBlocked(ox, oy, 0))
				{
					okDestX = adjustedX - 0.10f;
					break;
				}
			}
			// left?
			{
				float adjustedX = okDestX - ((float)i / 10.0f);
				float adjustedZ = okDestZ;
				int ox = data->gameMap->scaledToObstacleX(adjustedX);
				int oy = data->gameMap->scaledToObstacleY(adjustedZ);
				//if (data->gameMap->getObstacleHeight(ox, oy) != 0)
				if (data->gameScene->isBlocked(ox, oy, 0))
				{
					okDestX = adjustedX + 0.10f;
					break;
				}
			}
			// up?
			{
				float adjustedX = okDestX;
				float adjustedZ = okDestZ - ((float)i / 10.0f);
				int ox = data->gameMap->scaledToObstacleX(adjustedX);
				int oy = data->gameMap->scaledToObstacleY(adjustedZ);
				//if (data->gameMap->getObstacleHeight(ox, oy) != 0)
				if (data->gameScene->isBlocked(ox, oy, 0))
				{
					okDestZ = adjustedZ + 0.10f;
					break;
				}
			}
			// down?
			{
				float adjustedX = okDestX;
				float adjustedZ = okDestZ + ((float)i / 10.0f);
				int ox = data->gameMap->scaledToObstacleX(adjustedX);
				int oy = data->gameMap->scaledToObstacleY(adjustedZ);
				//if (data->gameMap->getObstacleHeight(ox, oy) != 0)
				if (data->gameScene->isBlocked(ox, oy, 0))
				{
					okDestZ = adjustedZ - 0.10f;
					break;
				}
			}
		}

		// now go to destination...
		ua->setPathTo(u, VC3(okDestX, 0, okDestZ));

		
		// and set unit behaviour modes...
		if (moveType == Unit::MoveTypeNormal)
		{
			u->setStealthing(false);
			u->setSpeed(Unit::UNIT_SPEED_FAST);
			u->setMode(Unit::UNIT_MODE_DEFENSIVE);
		}
		if (moveType == Unit::MoveTypeFast)
		{
			u->setStealthing(false);
			if (u->getRunningValue() > 0)
			{
				u->setSpeed(Unit::UNIT_SPEED_SPRINT);
				u->setMode(Unit::UNIT_MODE_HOLD_FIRE);
			} else {
				// unit incapable of sprinting, just move at normal running speed.
				u->setSpeed(Unit::UNIT_SPEED_FAST);
				u->setMode(Unit::UNIT_MODE_HOLD_FIRE);
			}
		}
		if (moveType == Unit::MoveTypeStealth)
		{
			// stealth armor goes to stealth mode...
			if (u->getStealthValue() > 0)
			{
				u->setSpeed(Unit::UNIT_SPEED_FAST);
				u->setStealthing(true);
				u->setMode(Unit::UNIT_MODE_HOLD_FIRE);
			} else {
				// other just normal movement (no more sneak)
				//u->setSpeed(Unit::UNIT_SPEED_SLOW);
				u->setSpeed(Unit::UNIT_SPEED_FAST);
				u->setStealthing(false);
				u->setMode(Unit::UNIT_MODE_DEFENSIVE);
			}
		}
	}

	if (connectionChecker != NULL)
	{
		delete connectionChecker;
	}
}

void UnitFormation::setMovePoint(std::vector<Unit *> *units, const VC3 &scaledMapPos)
{
	// TODO
}

void UnitFormation::setTarget(std::vector<Unit *> *units, const VC3 &targetPosition, Unit::FireType fireType)
{
	bool stopMove = SimpleOptions::getBool(DH_OPT_B_TARGETING_STOPS);
	bool autoWeaps = SimpleOptions::getBool(DH_OPT_B_TARGET_BASED_WEAPON_CHOOSE);

	for (int i = 0; i < (int)units->size(); i++)
	{
    Unit *u = (*units)[i];

		// stop if the option says so
		if (stopMove)
		{
			UnitActor *ua = getUnitActorForUnit(u);
			ua->stopUnit(u);
		}

		// then set target position...
		if (u->hasWeaponsForFiretype(fireType))
		{
			VC3 tmp = targetPosition;
			u->setWeaponsActiveByFiretype(fireType);
			if (autoWeaps
				&& fireType != Unit::FireTypePrimary && fireType != Unit::FireTypeSecondary)
				u->inactivateAntiVehicleWeapons();
			u->targeting.setTarget(tmp);
		}
	}
}

void UnitFormation::setTarget(std::vector<Unit *> *units, Unit *targetUnit, Unit::FireType fireType)
{
	bool stopMove = SimpleOptions::getBool(DH_OPT_B_TARGETING_STOPS);
	bool autoWeaps = SimpleOptions::getBool(DH_OPT_B_TARGET_BASED_WEAPON_CHOOSE);

	for (int i = 0; i < (int)units->size(); i++)
	{
    Unit *u = (*units)[i];

		// stop if the option says so
		if (stopMove)
		{
			UnitActor *ua = getUnitActorForUnit(u);
			ua->stopUnit(u);
		}

		// then set target unit...
		if (u->hasWeaponsForFiretype(fireType))
		{
			if (targetUnit == u)
			{
				u->targeting.clearTarget();
			} else {
				u->setWeaponsActiveByFiretype(fireType);
				if (autoWeaps && !targetUnit->getUnitType()->isVehicle()
					&& fireType != Unit::FireTypePrimary && fireType != Unit::FireTypeSecondary)
					u->inactivateAntiVehicleWeapons();
				u->targeting.setTarget(targetUnit);
			}
		}
	}
}

void UnitFormation::setFormation(FormationType formation)
{
	data->currentFormation = formation;
}

void UnitFormation::clearTarget(std::vector<Unit *> *units)
{
	for (int i = 0; i < (int)units->size(); i++)
	{
    Unit *u = (*units)[i];
		u->targeting.clearTarget();
	}
}

void UnitFormation::clearMovePoint(std::vector<Unit *> *units)
{
	for (int i = 0; i < (int)units->size(); i++)
	{
    Unit *u = (*units)[i];
		UnitActor *ua = getUnitActorForUnit(u);
		ua->stopUnit(u);
	}
}

// Accessors

UnitFormation::FormationType UnitFormation::getFormation() const
{
	return data->currentFormation;
}

} // end of namespace game
