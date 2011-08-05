
#include "precompiled.h"

// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning( disable : 4800 )
#endif

#include <vector>
#include <string>
#include <map>
#include <list>
#include <istream>
#include <ostream>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <Storm3D_UI.h>
#include "../editor/string_conversions.h"
#include "../editor/parser.h"
#include "track.h"
//#include "paramblock.h"
#include "parseutil.h"
#include "particlesystem.h"
#include "particleeffect.h"
#ifdef PHYSICS_PHYSX
#include "ParticlePhysics.h"
#include "../physics/physics_lib.h"
#endif

#include "sprayparticlesystem.h"
#include "cloudparticlesystem.h"
#include "modelparticlesystem.h"
#include "pointarrayparticlesystem.h"

#include "igios.h"

namespace frozenbyte {
namespace particle {

using namespace frozenbyte::editor;

class ParticleEffect : public IParticleEffect 
{
	friend class ParticleEffectManager;

	Vector m_velocity;
	Vector m_position;
	Vector m_rotation;
		
	std::vector< boost::shared_ptr<IParticleSystem> > m_systems;
	boost::shared_ptr<IParticleCollision> collision;
	boost::shared_ptr<IParticleArea> area;
	boost::shared_ptr<ParticlePhysics> physics;

	bool m_dieWithEffect;

	COL ambient;
	signed short int lightIndices[LIGHT_MAX_AMOUNT];
	//VC3 lightPos;
	//COL lightCol;
	//float lightRange;

	QUAT emitterRotation;

public:

	ParticleEffect()
	{
		for(int i = 0; i < LIGHT_MAX_AMOUNT; ++i)
			lightIndices[i] = -1;
	}

/*
	void calcFacingMatrix(Matrix& m, const Vector& d) {
		
		Vector r = d;
		r.Normalize();
		Vector v = r.GetCrossWith(Vector(0.0f, 1.0f, 0.0f));
		float angle = v.GetDotWith(v);
		if(angle<0.00001f) {
			v = r.GetCrossWith(Vector(1.0f, 0.0f, 0.0f));			
		}
		Vector s = r.GetCrossWith(v);
		Vector t = r.GetCrossWith(s);
		m.CreateBaseChangeMatrix(Vector(s.x, r.x, t.x), Vector(s.y, r.y, t.y), Vector(s.z, r.z, t.z));
		
	}
*/	
	
	void setPosition(const Vector& position) {
		
//		m_velocity = position - m_position; 
		m_position = position;
			
	}

	void setVelocity(const Vector& velocity) {
		
		m_velocity = velocity;
	
	}
	
	void setRotation(const Vector& rotation) {
		
		m_rotation = rotation;
	
	}

	void setEmitFactor(float factor)
	{
		for(int i = 0; i < (int)m_systems.size(); i++) 
			m_systems[i]->setEmitFactor(factor);
	}

	void setLighting(const COL &ambient_, const signed short int *lightIndices_)
	{
		ambient = ambient_;
		//lightPos = lightPos_;
		//lightCol = lightCol_;
		//lightRange = lightRange_;

		for(int i = 0; i < LIGHT_MAX_AMOUNT; ++i)
			lightIndices[i] = lightIndices_[i];
	}

	void setCollision(boost::shared_ptr<IParticleCollision> &collision_)
	{
		collision = collision_;
	}
	
	void setArea(boost::shared_ptr<IParticleArea> &area_)
	{
		area = area_;
	}

	void setEmitterRotation(const QUAT &rotation)
	{
		emitterRotation = rotation;
	}

	void setSpawnModel(IStorm3D_Model *model)
	{
		for(int i = 0; i < (int)m_systems.size(); ++i) 
		{
			m_systems[i]->setSpawnModel(model);
		}		
	}

	void tick() {

//		m_position += m_velocity;
		
		for(int i = 0; i < (int)m_systems.size(); ++i) 
		{
			m_systems[i]->setLighting(ambient, lightIndices);

			m_systems[i]->setCollision(collision);
			m_systems[i]->setArea(area);

			m_systems[i]->setPosition(m_position);
			m_systems[i]->setVelocity(m_velocity);
			m_systems[i]->setRotation(m_rotation);
			m_systems[i]->setEmitterRotation(emitterRotation);
//			m_systems[i]->setLenght(m_lenght);
		}
			
	}
	
	void kill() {
		// psd
		//if(m_dieWithEffect) 
		{
			for(int i = 0; i < (int)m_systems.size(); i++) {
				m_systems[i]->kill();
			}
		}
	}
		
	bool isDead() {
		bool alive = true;
		for(int i = 0; i < (int)m_systems.size(); i++) {
			if(m_systems[i]->isDead()) {
				alive = false;
			}
		}
		return !alive;
	}
	
	int getNumSystems() const {
		return m_systems.size();
	}
	
	IParticleSystem* getParticleSystem(int i) {
		assert(i >= 0 && i < (int)m_systems.size());
		return m_systems[i].get();
	}
	
	IParticleSystem* addParticleSystem(const std::string& className) {

		boost::shared_ptr<IParticleSystem> ps;
		if(className == "spray") {
			ps = SprayParticleSystem::createNew();
		}
		else if(className == "cloud") {
			ps = CloudParticleSystem::createNew();
		}
		else if(className == "parray") {
			ps = PointArrayParticleSystem::createNew();
		} 
		else if(className == "modelp") {
			ps = ModelParticleSystem::createNew();
		} 
		else
		{
			assert(!"undefined class name");
			igiosErrorMessage("Undefined class name.");
			return NULL;
		}

		m_systems.push_back(ps);
		return ps.get();
	}
	
	void removeSystem(int i) {
		assert(i >= 0 && i < (int)m_systems.size());
		m_systems.erase(m_systems.begin() + i);
	}
	
};


ParticleEffectManager* ParticleEffectManager::m_singleton = NULL;

ParticleEffectManager& ParticleEffectManager::getSingleton() {
	return *m_singleton;
}

ParticleEffectManager::ParticleEffectManager(IStorm3D* s3d, IStorm3D_Scene* scene) : 
	m_s3d(s3d), m_scene(scene), physicsEnabled(false), particlePhysicsEnabled(true)
{ 
	assert(m_singleton == NULL);
	m_singleton = this;
	assert(s3d != NULL);
	assert(scene != NULL);
	m_index = 0;
	memset(&m_stats, 0, sizeof(m_stats));
}

ParticleEffectManager::~ParticleEffectManager() {
  m_singleton = NULL;
}

/*

bool ParticleEffectManager::loadParticleEffect(int id, const Parser& parser) {
	const ParserGroup& g = parser.getGlobals();
	const ParserGroup& eg = g.getSubGroup("effect");
	int nSystems = convertFromString<int>(eg.getValue("num_systems", "0"), 0);
	if(nSystems <= 0)
		return false;
	
	ParticleEffect* proto = NULL;
	for(int i = 0; i < nSystems; i++) {
		
		std::string str = "system" + boost::lexical_cast<std::string>(i);
		const ParserGroup& sg = eg.getSubGroup(str);
		
		if(proto == NULL) {
			proto = new ParticleEffect;
			boost::shared_ptr<IParticleEffect> ptr(proto);
			m_protos[id] = ptr;
		}
		
		std::string className = sg.getValue("class", "");
		IParticleSystem* ps = proto->addParticleSystem(className);

		ps->parseFrom(sg);
		ps->init(m_s3d, m_scene);
	}

	return true;
}
*/

void ParticleEffectManager::recreate(IStorm3D* s3d, IStorm3D_Scene* scene) {
	m_s3d = s3d;
	m_scene = scene;
}


int ParticleEffectManager::loadParticleEffect(const EditorParser& parser) {
	const ParserGroup& g = parser.getGlobals();
	const ParserGroup& eg = g.getSubGroup("effect");
	int nSystems = convertFromString<int>(eg.getValue("num_systems", "0"), 0);
	if(nSystems <= 0)
		return -1;
	
	ParticleEffect* proto = NULL;
	for(int i = 0; i < nSystems; i++) {
		
		std::string str = "system" + boost::lexical_cast<std::string>(i);
		const ParserGroup& sg = eg.getSubGroup(str);
		
		if(proto == NULL) {
			proto = new ParticleEffect;
			boost::shared_ptr<IParticleEffect> ptr(proto);
			m_protos.push_back(ptr);
			int dieWithEffect = convertFromString<int>(eg.getValue("die_with_object", ""), 0);
			proto->m_dieWithEffect = (dieWithEffect == 1) ? true : false;
		}
		
		std::string className = sg.getValue("class", "");
		if (className.empty())
		{
			className = sg.getValue("@class", "");
		}
		IParticleSystem* ps = proto->addParticleSystem(className);

		if (ps != NULL)
		{
			ps->parseFrom(sg, materialParser);
			ps->setPhysics(physics);
			ps->init(m_s3d, m_scene);
		}
	}

	return m_protos.size()-1;
}

int ParticleEffectManager::getEffectPrototypeAmount()
{
	return m_protos.size();
}

boost::shared_ptr<IParticleEffect> ParticleEffectManager::getEffectPrototype(int id) 
{
	return m_protos[id];
}

boost::shared_ptr<IParticleEffect> ParticleEffectManager::addEffectToScene(int id) {
	
	assert(id >= 0 && id < (int)m_protos.size());
				
	ParticleEffect* effect = new ParticleEffect;
	boost::shared_ptr<IParticleEffect> ptr(effect);
	ParticleEffect* proto = static_cast<ParticleEffect*>(m_protos[id].get());
	
	for(int i = 0; i < (int)proto->m_systems.size(); i++) 
	{	
		boost::shared_ptr<IParticleSystem> ps = proto->getParticleSystem(i)->clone();
		
		effect->m_systems.push_back(ps);	
		effect->m_dieWithEffect = proto->m_dieWithEffect;
		
		if(particlePhysicsEnabled)
			ps->setPhysics(physics);

		ps->prepareForLaunch(m_s3d, m_scene);
		m_systems.push_back(ps);
	}
	
	return ptr;
	
	static boost::shared_ptr<IParticleEffect> empty;
	return empty;
}

void ParticleEffectManager::release()
{
	m_protos.clear();
}

void ParticleEffectManager::reset(bool statsOnly) {
	if(!statsOnly) {
		m_systems.clear();
	}
	memset(&m_stats, 0, sizeof(m_stats));
}

const ParticleEffectManager::Stats& ParticleEffectManager::getStats() {
	return m_stats;
}

void ParticleEffectManager::updatePhysics()
{
#ifdef PHYSICS_PHYSX
	if(physics)
		physics->update();
#endif
}

void ParticleEffectManager::enablePhysics(bool enable)
{
#ifdef PHYSICS_PHYSX
	if(physicsEnabled != enable)
	{
		if(!enable)
			physics.reset();
		else
			physics.reset(new ParticlePhysics());
	}

	physicsEnabled = enable;
#endif
}

void ParticleEffectManager::enableParticlePhysics(bool enable)
{
	particlePhysicsEnabled = enable;
}

void ParticleEffectManager::setPhysics(boost::shared_ptr<physics::PhysicsLib> &physics_)
{
#ifdef PHYSICS_PHYSX
	if(physics)
		physics->setPhysics(physics_);
#endif
}

void ParticleEffectManager::setModelParticleParameters(int maxAmount, int maxSpawnAmount)
{
#ifdef PHYSICS_PHYSX
	if(physics)
	{	
		physics->setMaxParticleAmount(maxAmount);
		physics->setMaxParticleSpawnAmount(maxSpawnAmount);
	}
#endif
}

void ParticleEffectManager::addPhysicsExplosion(const VC3 &position, float forceFactor, float radius)
{
#ifdef PHYSICS_PHYSX
	if(physics)
	{	
		physics->physicsExplosion(position, forceFactor, radius);
	}
#endif
}

void ParticleEffectManager::releasePhysicsResources()
{
	{
		std::list< boost::shared_ptr<IParticleSystem> >::iterator it = m_systems.begin();
		for(; it != m_systems.end(); ++it)
			(*it)->releasePhysicsResources();
	}

	{
		std::vector< boost::shared_ptr<IParticleEffect> >::iterator it = m_protos.begin();
		for(; it != m_protos.end(); ++it)
		{
			ParticleEffect *proto = static_cast<ParticleEffect *> (it->get());
			for(int i = 0; i < int(proto->m_systems.size()); i++) 
				proto->m_systems[i]->releasePhysicsResources();
		}
	}

#ifdef PHYSICS_PHYSX
	if(physics)
	{
		physics::PhysicsLib *ptr = 0;
		boost::shared_ptr<physics::PhysicsLib> nullLib(ptr);
		physics->setPhysics(nullLib);
	}
#endif
}

void ParticleEffectManager::tick() {
	m_stats.numParticles = 0;
	m_stats.numSystems = 0;
	std::list< boost::shared_ptr<IParticleSystem> >::iterator it = m_systems.begin();
	while(it != m_systems.end()) 
	{
		(*it)->tick(m_scene);
		if((*it)->isDead()) 
		{
			(*it)->releasePhysicsResources();
			it = m_systems.erase(it);
		} else {
			m_stats.numSystems++;
			if(m_stats.maxSystems < m_stats.numSystems) {
				m_stats.maxSystems = m_stats.numSystems;
			}
			m_stats.numParticles += (*it)->getNumParticles();
			if(m_stats.maxParticles < m_stats.numParticles) {
				m_stats.maxParticles = m_stats.numParticles;
			}
			if((*it)->getNumParticles() > m_stats.maxParticlesPerSystem) {
				m_stats.maxParticlesPerSystem = (*it)->getNumParticles();
			}
			it++;
		}
	}
}

void ParticleEffectManager::render() 
{
#ifdef PHYSICS_PHYSX
#ifndef NX_DISABLE_FLUIDS
	if(physics)
		physics->resetFluidRendering();
#endif
#endif

	std::list< boost::shared_ptr<IParticleSystem> >::iterator it = m_systems.begin();
	while(it != m_systems.end()) 
	{
		(*it)->render(m_scene);
		it++;
	}
}

} // frozenbyte
} // parser
