/** @file ParticleManager.h


	*/

#ifndef PARTICLEMANAGER_H
#define PARTICLEMANAGER_H

#include "ParticleSystem.h"

#include <list>

/* Forward declarations */
class IStorm3D_Scene;

namespace ui {



/** A class that handles all particle systems.
  */

class ParticleManager {
public:
	ParticleManager(IStorm3D *s3d, IStorm3D_ParticleSystem *particleSystem);
	~ParticleManager();

	void stepForwards(int tick);
	void render();

	void spawnParticleSystem(ParticleSystem::ParticleSystemType type, Vector vector1, Vector vector2, float lifeTime);

private:
	
	/* This list should be the sole owner of the particle systems.
	   (otherwise we would need shared (smart) pointers)          */
	std::list<ParticleSystem*> particleSystems;

	IStorm3D_ParticleSystem *stormParticleSystem_;
	IStorm3D *storm3d;
};



} // namespace ui

#endif /* PARTICLEMANAGER_H */
/* EOF */
