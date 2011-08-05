#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <list>
#include <vector>
#include <map>
#include <string>
#include <istream>
#include <ostream>
#include <Storm3D_Ui.h>
#include "particle_typedef.h"
#include "particle.h"
#include "float_track.h"
#include "vector_track.h"
#include "particle_desc.h"
#include "emitter_desc.h"
#include "particle_system.h"
#include "particle_system_manager.h"


struct ParticleSystemManagerData {

	ParticleSystemManagerData(IStorm3D* _s3d, IStorm3D_Scene* _scene) : s3d(_s3d), scene(_scene),
		numParticleSystems(0) {}
	
	IStorm3D* s3d;
	IStorm3D_Scene* scene;

	std::map< std::string, SharedPtr<ParticleSystem> > templateMap;

	std::list< SharedPtr<ParticleSystem> > systems;
	int numParticleSystems;
/*	
	void tick(int timeDif) {

		std::list< SharedPtr<ParticleSystem> >::iterator it = mSystems.begin();
		/*
		while(it != mSystems.end()) {
			if(!(*it)->tick(mScene, timeDif)) {
				it = mSystems.erase(it);
				mNumParticleSystems--;
			}
			it++;
		}
		*/
/*		while(it != mSystems.end())
		{
			if(!(*it)->tick(mScene, timeDif)) 
			{
				it = mSystems.erase(it);
				mNumParticleSystems--;
			}
			else
				++it;
		}
	}
	
	void render() {

		std::list< SharedPtr<ParticleSystem> >::iterator it = mSystems.begin();
		while(it != mSystems.end()) {
			(*it)->render(mScene);
			++it;
		}
	
	}

>>>>>>> 1.5
*/
};


ParticleSystemManager::ParticleSystemManager(IStorm3D* s3d, IStorm3D_Scene* scene) {
	ScopedPtr<ParticleSystemManagerData> temp(new ParticleSystemManagerData(s3d, scene));
	m.swap(temp);
}
	
ParticleSystemManager::~ParticleSystemManager() {

}

SharedPtr<ParticleSystem> ParticleSystemManager::addTemplate(const std::string& name) {
	SharedPtr<ParticleSystem> ps(new ParticleSystem(m->s3d));
	ps->setName(name);
	m->templateMap[name] = ps;
	return ps;
}
	
void ParticleSystemManager::renameTemplate(const std::string& oldName, const std::string& newName) {
	SharedPtr<ParticleSystem> ps;
	std::map< std::string, SharedPtr<ParticleSystem> >::iterator it = m->templateMap.find(oldName);
	if(it != m->templateMap.end()) {
		ps = it->second;			
		m->templateMap.erase(it);
	}
	m->templateMap[newName] = ps;
	ps->setName(newName);
}

void ParticleSystemManager::removeTemplate(const std::string& name) {
	std::map< std::string, SharedPtr<ParticleSystem> >::iterator it = m->templateMap.find(name);
	if(it != m->templateMap.end()) {
		m->templateMap.erase(it);
	}
}
	
SharedPtr<ParticleSystem> ParticleSystemManager::getTemplate(const std::string& name) {
	SharedPtr<ParticleSystem> ps;
	std::map< std::string, SharedPtr<ParticleSystem> >::iterator it = m->templateMap.find(name);
	if(it != m->templateMap.end()) {
		ps = it->second;
	}
	return ps;
}

SharedPtr<ParticleSystem> ParticleSystemManager::spawnParticleSystem(const std::string& name) {

	SharedPtr<ParticleSystem> ps;
		
	std::map< std::string, SharedPtr<ParticleSystem> >::iterator it = m->templateMap.find(name);
	if(it != m->templateMap.end()) {
		
		SharedPtr<ParticleSystem> temp(new ParticleSystem(m->s3d));
		ps.swap(temp);
		ps->copy(it->second);

		m->numParticleSystems++;
		m->systems.push_back(ps);

	} else {
		assert("Failed to find particlesystem named...");
	}

	return ps;
}

SharedPtr<ParticleSystem> ParticleSystemManager::getParticleSystem(int i) {
	assert(i < m->systems.size());
	std::list< SharedPtr<ParticleSystem> >::iterator it = m->systems.begin();
	for(int j = 0; j < i; j++) it++;
	return (*it);
}

int ParticleSystemManager::getNumParticleSystems() {
	return m->numParticleSystems;
}
	
void ParticleSystemManager::removeParticleSystem(int i) {
	assert(i < m->systems.size());
	std::list< SharedPtr<ParticleSystem> >::iterator it = m->systems.begin();
	for(int j = 0; j < i; j++) it++;
		m->systems.erase(it);	
}

void ParticleSystemManager::removeAllParticleSystems() {
	m->systems.clear();
}

void ParticleSystemManager::tick(int timeDif) {

	std::list< SharedPtr<ParticleSystem> >::iterator it = m->systems.begin();
	while(it != m->systems.end()) {
		if(!(*it)->tick(m->scene, timeDif)) {
			it = m->systems.erase(it);
			m->numParticleSystems--;
		} else 
			it++;
	}
	
}

void ParticleSystemManager::render() {

	std::list< SharedPtr<ParticleSystem> >::iterator it = m->systems.begin();
	while(it != m->systems.end()) {
		(*it)->render(m->scene);
		it++;
	}
	
}


float ParticleSystemManager::gGravity = 0.1f * ParticleSystem::TIME_SCALE;
float ParticleSystemManager::gDragFactor = 1.0f * ParticleSystem::TIME_SCALE;

void ParticleSystemManager::setDragFractor(float f) {
	gDragFactor = f;
}

float ParticleSystemManager::getDragFractor() {
	return gDragFactor;
}

void ParticleSystemManager::setGravity(float g) {
	gGravity = g;
}

float ParticleSystemManager::getGravity() {
	return gGravity;
}

void ParticleSystemManager::saveConfig(std::ostream& os) {

	Parser parser;
	ParserGroup& g = parser.getGlobals();
	::parseOut(g, "gravity", gGravity);
	::parseOut(g, "drag_factor", gDragFactor);
	os << parser;
		
}

void ParticleSystemManager::loadConfig(std::istream& is) {

	Parser parser;
	is >> parser;
	ParserGroup& g = parser.getGlobals();
	::parseIn(g, "gravity", gGravity);
	::parseIn(g, "drag_factor", gDragFactor);

}


