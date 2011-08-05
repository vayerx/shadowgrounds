
#include "precompiled.h"

#include "Building.h"
#include "scaledefs.h"
#include "../ui/VisualObject.h"
#include "physics/StaticPhysicsObject.h"
#include "../util/SoundMaterialParser.h"

#include "../util/Debug_MemoryManager.h"


using namespace ui;

namespace game
{

  const int buildingDataId = 0;


  //Building::Building(char *filename, int mapX, int mapY)
  Building::Building(const char *filename)
  {
    this->modelFilename = new char[strlen(filename) + 1];
    strcpy(modelFilename, filename);
    //mapPosition = VC2I(mapX, mapY);
		this->physicsObject = NULL;
  }

  Building::~Building()
  {
    if (modelFilename != NULL)
    {
      delete [] modelFilename;
    }
    if (visualObject != NULL)
    {
      delete visualObject;
    }
		if (this->physicsObject != NULL)
		{
			delete this->physicsObject;
		}
  }

  char *Building::getModelFilename()
  {
    return modelFilename;
  }

  SaveData *Building::getSaveData() const
  {
    // TODO
    return NULL;
  }

  const char *Building::getStatusInfo() const
  {
    return "Building";
  }

  void *Building::getVisualObjectDataId() const
  { 
    return (void *)&buildingDataId; 
  }

	/*
	// deprecated
  VC2I Building::getMapPosition()
  {
    return mapPosition;
  }
	*/

  void Building::setPosition(VC3 &position)
  {
    this->position = position;
    if (visualObject != NULL)
    {
      visualObject->setPosition(position);
    }
  }


  void Building::addPhysics(GamePhysics *gamePhysics)
  {
		if (visualObject != NULL)
		{
			assert(physicsObject == NULL);
			assert(visualObject->getStormModel() != NULL);

			QUAT rot = QUAT();
			physicsObject = new StaticPhysicsObject(gamePhysics, modelFilename, visualObject->getStormModel(), position, rot);

			util::SoundMaterialParser soundmp;
			int soundindex = soundmp.getMaterialIndexByName("Building");
			if (soundindex == SOUNDMATERIALPARSER_NO_SOUND_INDEX)
			{
				Logger::getInstance()->warning("Building::addPhysics - Failed to solve sound material index for \"Building\" sound material name (material definition missing).");
			}
			physicsObject->setSoundMaterial(soundindex);
		}
	}

  void Building::deletePhysics(GamePhysics *gamePhysics)
  {
		if (physicsObject != NULL)
		{
			delete physicsObject;
			physicsObject = NULL;
		}
	}

  VC3 Building::getPosition()
  {
    return position;
  }

  ui::VisualObject *Building::getVisualObject()
  {
    return visualObject;
  }

  void Building::setVisualObject(ui::VisualObject *visualObject)
  {
    this->visualObject = visualObject;
  }


}

