#ifndef PARTICLE_SYSTEM_MANAGER_H
#define PARTICLE_SYSTEM_MANAGER_H

namespace frozenbyte
{
namespace particle
{

class IParticleSystem;
class ParticleSystemManager;


class ParticleSystemManager {
public:
	struct Stats {
		int numParticles;
		int maxParticles;
		int numSystems;
		int maxSystems;
		int maxParticlesPerSystem;
	};
private:
	IStorm3D* m_s3d;
	IStorm3D_Scene* m_scene;
	Stats m_stats;
	std::list< boost::shared_ptr<IParticleSystem> > m_systems;
public:
	ParticleSystemManager(IStorm3D* s3d, IStorm3D_Scene* scene);
							
	void addParticleSystem(boost::shared_ptr<IParticleSystem> ps);
	
	void reset(bool statsOnly=true);
	void tick();
	const Stats& getStats();
	void render();
};



} // particle

} // frozenbyte


#endif
