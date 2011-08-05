
#include "precompiled.h"

#include <Storm3D_UI.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <vector>
#include <string>
#include <map>
#include <list>
#include "../editor/parser.h"
#include "track.h"
//#include "paramblock.h"
#include "parseutil.h"
#include "particlesystem.h"
#include "particlesystemmanager.h"


namespace frozenbyte {
namespace particle {

ParticleSystemManager::ParticleSystemManager(IStorm3D* s3d, IStorm3D_Scene* scene) 
:	m_s3d(s3d), m_scene(scene) 
{
	memset(&m_stats, 0, sizeof(m_stats));
}

void ParticleSystemManager::addParticleSystem(boost::shared_ptr<IParticleSystem> ps) 
{
	ps->prepareForLaunch(m_s3d, m_scene);
	m_systems.push_back(ps);
}

void ParticleSystemManager::reset(bool statsOnly) 
{
	if(!statsOnly) 
	{
		m_systems.clear();
	}

	memset(&m_stats, 0, sizeof(m_stats));
}

const ParticleSystemManager::Stats& ParticleSystemManager::getStats() 
{
	return m_stats;
}

void ParticleSystemManager::tick() 
{
	m_stats.numParticles = 0;
	m_stats.numSystems = 0;
	
	std::list< boost::shared_ptr<IParticleSystem> >::iterator it = m_systems.begin();
	while(it != m_systems.end()) 
	{
		(*it)->tick(m_scene);
		if((*it)->isDead()) {
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

void ParticleSystemManager::render() 
{
	std::list< boost::shared_ptr<IParticleSystem> >::iterator it = m_systems.begin();
	while(it != m_systems.end()) 
	{
		(*it)->render(m_scene);
		it++;
	}
}


} // frozenbyte
} // particle
