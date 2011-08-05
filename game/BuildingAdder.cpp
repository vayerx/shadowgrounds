
#include "precompiled.h"

#include "BuildingAdder.h"

#include "Game.h"
#include "GameScene.h"
#include "Building.h"
#include "BuildingList.h"
#include "GameUI.h"
#include "VisualObjectModelStorage.h"
#include "../ui/VisualObject.h"

#include "../ui/LightManager.h"
#include "../util/SelfIlluminationChanger.h"

#include "../util/Debug_MemoryManager.h"

using namespace ui;

namespace game
{

	bool buildingadder_enableterraincut = false;

	void BuildingAdder::setTerrainCut(bool enableTerrainCut)
	{
		buildingadder_enableterraincut = enableTerrainCut;
	}


	void BuildingAdder::addBuilding(Game *game, const VC3 &position, 
		const char *filename, GamePhysics *gamePhysics)
	{
		Building *b = new Building(filename);
		game->buildings->addBuilding(b);

    createVisualForBuilding(game, b);
    //VC2I mapPos = b->getMapPosition();
    //float x = gameMap->configToScaledX(mapPos.x);
    //float y = gameMap->configToScaledY(mapPos.y);
    //b->setPosition(VC3(x, gameMap->getScaledHeightAt(x, y), y));
		VC3 pos = position;
		b->setPosition(pos);

    game->getGameScene()->addBuildingObstacle(b, buildingadder_enableterraincut);
    
    VisualObject *vo = b->getVisualObject();
    if (vo != NULL)
    {
			// Damn, should change the buidlinghandler to use the
			// visual object instead of storm models...

      // Add to building handler too
      game->gameUI->buildingHandler.addBuilding(vo->model);

			b->addPhysics(gamePhysics);

// NOTE: these resources should be freed. (but cannot be, because we might be in physics simulation/because 
// the actual mesh cooking is done at next sync, not right now... (applies to PhysX)
// NOTE: also, ODE physics will pose same kind of problems (possibly even more due to multithreaded cooking)
// MOVED THIS LATER ON...

#ifdef PHYSICS_NONE
			if(vo->getStormModel())
			{
				vo->getStormModel()->FreeMemoryResources();
			}
#endif
    }

		// TODO: not an optimal solution doing this after every single
		// building!
    game->gameMap->applyObstacleHeightChanges();
	}


  void BuildingAdder::createVisualForBuilding(Game *game, Building *building)
  {
    // another quick haxor

    // maybe should check that the visual does not already exist

    //VisualObjectModel *vom = new VisualObjectModel(building->getModelFilename());
		VisualObjectModel *vom = game->visualObjectModelStorage->getVisualObjectModel(building->getModelFilename());

    VisualObject *vo = vom->getNewObjectInstance();
    building->setVisualObject(vo);
    vo->setInScene(true);
    vo->setDataObject(building);

		// disable collision to "firethrough" collision layer...
		IStorm3D_Model *model = vo->getStormModel();
		if(model)
		{
			const VC3 &sunDir = game->getGameUI()->getTerrain()->getSunDirection();
			model->SetDirectional(sunDir, 1.f);
		}

		Iterator<IStorm3D_Model_Object *> *objectIterator;
		for(objectIterator = model->ITObject->Begin(); !objectIterator->IsEnd(); objectIterator->Next())
		{
			IStorm3D_Model_Object *object = objectIterator->GetCurrent();

			const char *name = object->GetName();

			if (strstr(name, "FireThrough") != NULL)
			{
				object->SetNoCollision(true);
			}
		}

		delete objectIterator;

		// add building for self-illum (lightmap brightness) changes...
		game->gameUI->getLightManager()->getSelfIlluminationChanger()->addModel(vo->getStormModel());
  }

}

