#include <storm3d_ui.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <vector>
#include <string>
#include <map>
#include <list>
#include "..\editor\parser.h"
#include "track.h"
#include "paramblock.h"
#include "parseutil.h"
#include "particlesystem.h"
#include "particlesystemmanager.h"


namespace frozenbyte {
namespace particle {


ParticleSystemManager* ParticleSystemManager::m_singleton = NULL;

ParticleSystemManager::ParticleSystemManager(IStorm3D* s3d, IStorm3D_Scene* scene) : 
m_s3d(s3d), m_scene(scene), m_timeCount(0) {
	m_singleton = this;
	m_stats.numParticles = 0;
	m_stats.numSystems = 0;
	m_stats.maxParticles = 0;
	m_stats.maxSystems = 0;
}

ParticleSystemManager& ParticleSystemManager::getSingleton() {
	return *m_singleton;
}

void ParticleSystemManager::registerSystem(ParticleSystemClassDesc* cd) {
	m_systemClasses[cd->getClassName()] = cd;
}

void ParticleSystemManager::registerForce(ParticleForceClassDesc* cd) {
	m_forceClasses[cd->getClassName()] = cd;
}
	
ParticleSystem* ParticleSystemManager::createSystem(const std::string& className) {
	std::map< std::string, ParticleSystemClassDesc* >::iterator it = m_systemClasses.find(className);
	if(it != m_systemClasses.end()) {
		ParticleSystem *ps =  (*it).second->create();
		ps->create();
		return ps;
	}
	assert("Unkown Particle System Class");
	return NULL;
}

ParticleForce* ParticleSystemManager::createForce(const std::string className) {
	std::map< std::string, ParticleForceClassDesc* >::iterator it = m_forceClasses.find(className);
	if(it != m_forceClasses.end()) {
		ParticleForce* f =  it->second->create();
		f->create();
		return f;
	}
	assert("Unkown Particle Force Class");
	return NULL;
}


IStorm3D_Material* ParticleSystemManager::getMaterial(const std::string& textureName) {

	static int uniqueID = 0;
	uniqueID++;
	
	std::string mtlName = "particle_mtl" + boost::lexical_cast<std::string>(uniqueID);	
	IStorm3D_Material* mtl = m_s3d->CreateNewMaterial(mtlName.c_str());
	IStorm3D_Texture* tex = m_s3d->CreateNewTexture(textureName.c_str());
	mtl->SetBaseTexture(tex);
	
	return mtl; //NULL;
}

void ParticleSystemManager::addParticleSystem(boost::shared_ptr<ParticleSystem> ps) {
	m_systems.push_back(ps);
	ps->prepareForLaunch(m_s3d, m_scene);
}

void ParticleSystemManager::tick() {
/*
	m_timeCount += timeDif;
	while(m_timeCount > 10) {
		m_timeCount -= 10;
*/
		m_stats.numParticles = 0;
		m_stats.numSystems = 0;
		std::list< boost::shared_ptr<ParticleSystem> >::iterator it = m_systems.begin();
		while(it != m_systems.end()) {
			m_stats.numSystems++;
			if(m_stats.numSystems > m_stats.maxSystems)
				m_stats.maxSystems = m_stats.numSystems;
			(*it)->tick(m_scene);
			if((*it)->isDead()) {
				it = m_systems.erase(it);			
			} else {
				m_stats.numParticles += (*it)->getNumParticles();
				if(m_stats.numParticles > m_stats.maxParticles)
					m_stats.maxParticles = m_stats.numParticles;
				it++;
			}
		}
//	}
}

void ParticleSystemManager::render() {
	std::list< boost::shared_ptr<ParticleSystem> >::iterator it = m_systems.begin();
	while(it != m_systems.end()) {
		(*it)->render(m_scene);
		it++;
	}
}

const ParticleSystemManager::Stats& ParticleSystemManager::getStats() {
	return m_stats;
}

} // frozenbyte
} // particle
