#include <storm3d_ui.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>
#include <string>
#include <map>
#include <list>
#include <istream>
#include <ostream>
#include "..\editor\string_conversions.h"
#include "..\editor\parser.h"
#include "track.h"
#include "paramblock.h"
#include "parseutil.h"
#include "particlesystem.h"
#include "particlesystemmanager.h"
#include "particleeffect.h"

namespace frozenbyte {
namespace particle {

using namespace frozenbyte::editor;

class ParticleEffectImp : public ParticleEffect {
public:
	std::vector< boost::shared_ptr<ParticleSystem> > m_systems;

	~ParticleEffectImp() {
		
	}

	ParticleEffectImp* clone() {
		ParticleEffectImp* imp = new ParticleEffectImp();
		for(int i = 0; i < m_systems.size(); i++) {
			boost::shared_ptr<ParticleSystem> ps(m_systems[i]->launch());
			imp->m_systems.push_back(ps);
		}
		return imp;
	}
	
	void setTM(const Matrix& tm) {
		std::vector< boost::shared_ptr<ParticleSystem> >::iterator it = m_systems.begin();
		for(it; it != m_systems.end(); it++)
			(*it)->setTM(tm);
	}
	
	void setVelocity(const Vector& velocity) {
		std::vector< boost::shared_ptr<ParticleSystem> >::iterator it = m_systems.begin();
		for(it; it != m_systems.end(); it++)
			(*it)->setVelocity(velocity);
	}
	
	void kill() {
		std::vector< boost::shared_ptr<ParticleSystem> >::iterator it = m_systems.begin();
		for(it; it != m_systems.end(); it++)
			(*it)->kill();
	}
	bool isDead() {
		bool alive = false;
		std::vector< boost::shared_ptr<ParticleSystem> >::iterator it = m_systems.begin();
		for(it; it != m_systems.end(); it++) {
			if(!(*it)->isDead())
				alive = true;
		}
		return !alive;
	}
};

ParticleEffectManager* ParticleEffectManager::m_singleton = NULL;

ParticleEffectManager& ParticleEffectManager::getSingleton() {
	return *m_singleton;
}

ParticleEffectManager::ParticleEffectManager(IStorm3D* s3d, IStorm3D_Scene* scene,
										ParticleSystemManager* particleSystemManager) : 
m_s3d(s3d), m_scene(scene), m_systemManager(particleSystemManager) {
	assert(m_singleton == NULL);
	m_singleton = this;
	assert(s3d != NULL);
	assert(scene != NULL);
	assert(m_systemManager != NULL);
}

bool ParticleEffectManager::loadParticleEffect(int id, Parser& parser) {
//	Parser parser;
//	is >> parser;
	int nSystems;
	ParserGroup& g = parser.getGlobals();
	const ParserGroup& eg = g.getSubGroup("effect");
	nSystems = convertFromString<int>(eg.getValue("num_systems", "0"), 0);
	ParticleEffectImp* effect = NULL;
	for(int i = 0; i < nSystems; i++) {
		std::string str = "system" + boost::lexical_cast<std::string>(i);
		const ParserGroup& sg = eg.getSubGroup(str);
		std::string className = sg.getValue("class", "");
		if(className.empty())
			continue;
		boost::shared_ptr<ParticleSystem> ps(ParticleSystemManager::getSingleton().createSystem(className));
		if(effect == NULL) {
			effect = new ParticleEffectImp;
			boost::shared_ptr<ParticleEffectImp> imp(effect);
			m_effectTemplates[id] = imp;
		}
		ps->parseFrom(sg);
		ps->init(m_s3d, m_scene);
		effect->m_systems.push_back(ps);
	}
	return true;
}


boost::shared_ptr<ParticleEffect> ParticleEffectManager::addParticleEffect(int id) {
	
	std::map<int, boost::shared_ptr<ParticleEffectImp> >::iterator it = m_effectTemplates.find(id);
	
	if(it != m_effectTemplates.end()) {
		
		ParticleEffectImp* temp = it->second->clone();
		
		for(int i = 0; i < temp->m_systems.size(); i++) {
			ParticleSystemManager::getSingleton().addParticleSystem(temp->m_systems[i]);
		}
		
		boost::shared_ptr<ParticleEffect> ptr(temp);
		return ptr;
	}
	assert("apuapuapuapua");
	static boost::shared_ptr<ParticleEffect> empty;
	return empty;
}


} // frozenbyte
} // parser
