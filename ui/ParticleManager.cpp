/** @file ParticleManager.h
  */


#include "ParticleSystem.h"

#include "ParticleManager.h"


#include <algorithm>
#include <functional>

namespace ui {



ParticleManager::ParticleManager(IStorm3D *s3d, IStorm3D_ParticleSystem *stormParticleSystem)
	: stormParticleSystem_(stormParticleSystem), storm3d(s3d)
{
	ParticleSystem::setMaterials(s3d);
}



ParticleManager::~ParticleManager() {
	// nop
}



void ParticleManager::render() {

	std::for_each(particleSystems.begin(),
	              particleSystems.end(),
	              std::mem_fun<bool,ParticleSystem>(&ParticleSystem::render));

}


void ParticleManager::stepForwards(int tick) {
/*
	std::list<ParticleSystem*>::iterator i=particleSystems.begin();
	std::list<ParticleSystem*>::iterator end=particleSystems.end();
	for(;i!=end;++i) {
		bool isDone=(*i)->stepForwards(tick);
		if(isDone) {
			delete *i;
			i=particleSystems.erase(i);
		}
	}
*/
	// Uhh. Increment iterator IFF it wasn't removed. Erase returns next valid 
	// element anyway. Also, end is not fixed if container is modified 
	// (invalidates iterators, makes them undefined actually)
	//	-- psd
	for(std::list<ParticleSystem*>::iterator i=particleSystems.begin() ; i != particleSystems.end(); ) 
	{
		bool isDone=(*i)->stepForwards(tick);
		if(isDone) 
		{
			delete *i;
			i=particleSystems.erase(i);
		}
		else
			++i;
	}
}


void ParticleManager::spawnParticleSystem(ParticleSystem::ParticleSystemType type, Vector vector1, Vector vector2, float lifeTime) {


	ParticleSystem *particleSystem = new ParticleSystem(stormParticleSystem_,type,vector1,vector2,lifeTime);

	particleSystems.push_back(particleSystem);

}



} /* namespace ui */
