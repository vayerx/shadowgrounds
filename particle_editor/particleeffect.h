#ifndef PARTICLE_EFFECT_H
#define PARTICLE_EFFECT_H

namespace frozenbyte {
namespace particle {

class ParticleEffect;
class ParticleEffectImp;
class ParticleEffectManager;
class ParticleSystemManager;

class ParticleEffect {
public:
	virtual ~ParticleEffect() {}
	virtual void setTM(const Matrix& tm)=0;
	virtual void setVelocity(const Vector& velocity)=0;
	virtual void kill()=0;
	virtual bool isDead()=0;
};

class ParticleEffectManager {
	std::map<int, boost::shared_ptr<ParticleEffectImp> > m_effectTemplates;
	IStorm3D* m_s3d;
	IStorm3D_Scene* m_scene;
	ParticleSystemManager* m_systemManager;
	static ParticleEffectManager* m_singleton;
public:
	static ParticleEffectManager& getSingleton();
	ParticleEffectManager(IStorm3D* s3d, IStorm3D_Scene* scene, ParticleSystemManager* mgr);
	bool loadParticleEffect(int id, editor::Parser& parser);
	boost::shared_ptr<ParticleEffect> addParticleEffect(int id);
};

} // particle
} // frozenbyte


#endif