#ifndef PARTICLE_SYSTEM_MANAGER_H
#define PARTICLE_SYSTEM_MANAGER_H

namespace frozenbyte
{
namespace particle
{

class ParticleSystem;
class ParticleSystemManager;
class ParticleForce;
class ParticleSystemClassDesc;
class ParticleForceClassDesc;
class PointArray;


class ParticleSystemManager {
public:
	struct Stats {
		int numParticles;
		int maxParticles;
		int numSystems;
		int maxSystems;
	};
private:
	static ParticleSystemManager* m_singleton;
	std::map< std::string, ParticleSystemClassDesc* > m_systemClasses;
	std::map< std::string, ParticleForceClassDesc* > m_forceClasses;
	std::map< std::string, IStorm3D_Material* > m_materials;
	std::list< boost::shared_ptr<ParticleSystem> > m_systems;
	IStorm3D* m_s3d;
	IStorm3D_Scene* m_scene;
	int m_timeCount;
	Stats m_stats;
public:
	static ParticleSystemManager& getSingleton();

	ParticleSystemManager(IStorm3D* s3d, IStorm3D_Scene* scene);
	
	void registerSystem(ParticleSystemClassDesc* cd);
	void registerForce(ParticleForceClassDesc* cd);
	
	ParticleSystem* createSystem(const std::string& className);
	ParticleForce* createForce(const std::string className);

	// makes materials with particle+textureName as id
	IStorm3D_Material* getMaterial(const std::string& textureName);
	PointArray* getMesh(const std::string& fileName);
	
	void addParticleSystem(boost::shared_ptr<ParticleSystem> ps);
	void tick();
	void render();

	const Stats& getStats();
};

} // particle

} // frozenbyte


#endif