#ifndef INCLUDED_PARTICLE_AREA_H
#define INCLUDED_PARTICLE_AREA_H

#include <DatatypeDef.h>
#include "../particle_editor2/iparticlearea.h"

class IStorm3D_Scene;

namespace game {
	class GameMap;
	class GameScene;
} // game

namespace ui {

class ParticleArea: public frozenbyte::particle::IParticleArea
{
	game::GameMap *gameMap;
	game::GameScene *scene;
	bool insideCheck;

public:
	ParticleArea(game::GameMap *gameMap, game::GameScene *scene);
	~ParticleArea();

	void biasValues(const VC3 &position, VC3 &velocity) const;
	float getObstacleHeight(const VC3 &position) const;
	float getBaseHeight(const VC3 &position) const;
	bool isInside(const VC3 &position) const;

	void enableInsideCheck(bool enable);
};

} // ui

#endif

