#ifndef INCLUDED_PARTICLE_COLLISION_H
#define INCLUDED_PARTICLE_COLLISION_H

#include <DatatypeDef.h>
#include "../particle_editor2/iparticlecollision.h"

class IStorm3D_Scene;

namespace game {
	class GameMap;
	class GameScene;
} // game

namespace ui {

class ParticleCollision: public frozenbyte::particle::IParticleCollision
{
	game::GameMap *gameMap;
	game::GameScene *scene;
	float scale;
	int meters_1;
	int meters_1_5;
	int meters_3_5;
	int meters_0_2;
	int meters_0_3;

public:
	ParticleCollision(game::GameMap *gameMap, game::GameScene *scene);
	~ParticleCollision();

	bool spawnPosition(const VC3 &emitter, const VC3 &dir, VC3 &position) const;
	bool getCollision(const VC3 &oldPosition, VC3 &position, VC3 &velocity, float slowFactor, float groundSlowFactor, float wallSlowFactor) const;
};

class FluidParticleCollision: public frozenbyte::particle::IParticleCollision
{
	game::GameMap *gameMap;
	game::GameScene *scene;
	float scale;
	int meters_1;
	int meters_1_5;
	int meters_3_5;
	int meters_0_2;
	int meters_0_3;

public:
	FluidParticleCollision(game::GameMap *gameMap, game::GameScene *scene);
	~FluidParticleCollision();

	bool spawnPosition(const VC3 &emitter, const VC3 &dir, VC3 &position) const;
	bool getCollision(const VC3 &oldPosition, VC3 &position, VC3 &velocity, float slowFactor, float groundSlowFactor, float wallSlowFactor) const;
};

} // ui

#endif

