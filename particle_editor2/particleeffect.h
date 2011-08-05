// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef PARTICLE_EFFECT_H
#define PARTICLE_EFFECT_H

// just simple effect framework around particlesystems

#include "iparticlecollision.h"
#include "iparticlearea.h"
#include "../util/SoundMaterialParser.h"
#include <list>

namespace frozenbyte {
namespace physics {
	class PhysicsLib;
} // physics
namespace editor {
	class EditorParser;
}

namespace particle {

class IParticleEffect;
class IParticleSystem;
class ParticleEffectManager;
class ParticleSystemManager;
class IParticleCollision;
class ParticlePhysics;

class IParticleEffect {
public:
	virtual ~IParticleEffect() {}
	virtual void setPosition(const Vector& v)=0;
	virtual void setVelocity(const Vector& v)=0;
	virtual void setRotation(const Vector& v)=0;

	virtual void setEmitFactor(float factor) = 0;
	virtual void setLighting(const COL &ambient, const signed short int *lightIndices)=0;
	virtual void setCollision(boost::shared_ptr<IParticleCollision> &collision) = 0;
	virtual void setArea(boost::shared_ptr<IParticleArea> &area) = 0;
	virtual void setEmitterRotation(const QUAT &rotation) = 0;

	virtual void tick()=0;
	virtual void kill()=0;
	virtual bool isDead()=0;
	virtual int getNumSystems() const = 0;
	virtual IParticleSystem* getParticleSystem(int i)=0;
	virtual IParticleSystem* addParticleSystem(const std::string& className)=0;
	virtual void removeSystem(int i)=0;
	virtual void setSpawnModel(IStorm3D_Model *model) = 0;
};


class ParticleEffectManager {
public:
	struct Stats {
		int numParticles;
		int maxParticles;
		int numSystems;
		int maxSystems;
		int maxParticlesPerSystem;
	};
	
private:	
	Stats m_stats;
	std::list< boost::shared_ptr<IParticleSystem> > m_systems;
		
	std::vector< boost::shared_ptr<IParticleEffect> > m_protos;
	IStorm3D* m_s3d;
	IStorm3D_Scene* m_scene;
	static ParticleEffectManager* m_singleton;
	int m_index;

	bool physicsEnabled;
	bool particlePhysicsEnabled;
	bool fluidPhysicsEnabled;
	boost::shared_ptr<ParticlePhysics> physics;

	util::SoundMaterialParser materialParser;

public:
	static ParticleEffectManager& getSingleton();
	ParticleEffectManager(IStorm3D* s3d, IStorm3D_Scene* scene);
	~ParticleEffectManager();
	int loadParticleEffect(const editor::EditorParser& parser);
	int getEffectPrototypeAmount();
	boost::shared_ptr<IParticleEffect> getEffectPrototype(int id);
	boost::shared_ptr<IParticleEffect> addEffectToScene(int id);
	ParticleSystemManager& getParticleSystemManager();

	void release();
	void recreate(IStorm3D* s3d, IStorm3D_Scene* scene);

	void reset(bool statsOnly=true);
	const Stats& getStats();

	void updatePhysics();
	void enablePhysics(bool enable);
	void enableParticlePhysics(bool enable);
	void setPhysics(boost::shared_ptr<physics::PhysicsLib> &physics);
	void setModelParticleParameters(int maxAmount, int maxSpawnAmount);
	void addPhysicsExplosion(const VC3 &position, float forceFactor, float radius = 5.0f);
	void releasePhysicsResources();

	void tick();
	void render();
};


} // particle
} // frozenbyte


#endif
