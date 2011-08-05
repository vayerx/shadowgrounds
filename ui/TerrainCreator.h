#ifndef TERRAINCREATOR_H
#define TERRAINCREATOR_H

//
// This class generates storm terrains based on game maps
//

//#include <Storm3D_UI.h>
//#include "../game/GameMap.h"
//#include "../ui/Terrain.h"

class IStorm3D;
class IStorm3D_Scene;
class Terrain;

namespace game {
	class GameMap;
}

namespace ui {

class LightManager;
class AmbientSoundManager;

class TerrainCreator
{
    IStorm3D *storm3d;
    IStorm3D_Scene *scene;
    float cameraRange;

public:
	TerrainCreator(IStorm3D *s3d, IStorm3D_Scene *scene);
	~TerrainCreator();

	Terrain *createTerrain(game::GameMap *gameMap, ui::LightManager *lightManager, ui::AmbientSoundManager *ambientSoundManager, const char *configFile);
	void setCameraRange(float range);
};

} // ui

#endif
