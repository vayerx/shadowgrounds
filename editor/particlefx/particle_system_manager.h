#ifndef PARTICLE_SYSTEM_MANAGER_H
#define PARTICLE_SYSTEM_MANAGER_H


struct ParticleSystemManagerData;
class ParticleSystemManager {
	ScopedPtr<ParticleSystemManagerData> m;
	static float gGravity;
	static float gDragFactor;
public:
	
	ParticleSystemManager(IStorm3D* s3d, IStorm3D_Scene* scene);
	~ParticleSystemManager();
	
	// templates that are used to define systems

	SharedPtr<ParticleSystem> addTemplate(const std::string& name);
	void loadTemplate(const std::string& name, std::istream& is);
	void removeTemplate(const std::string& name);
	SharedPtr<ParticleSystem> getTemplate(const std::string& name);
	void renameTemplate(const std::string& name, const std::string& newName);

	// concrete systems made from templates

	SharedPtr<ParticleSystem> spawnParticleSystem(const std::string& name);
	SharedPtr<ParticleSystem> getParticleSystem(int i);
	std::string& getParticleSystemName(int i);
	int getNumParticleSystems();
	void removeParticleSystem(int i);
	void removeAllParticleSystems();

	// physics

	static void setDragFractor(float f);
	static float getDragFractor();
	static void setGravity(float g);
	static float getGravity();

	static void saveConfig(std::ostream& os);
	static void loadConfig(std::istream& is);
	
	// framestep and rendering	
	void tick(int timeDif);
	void render();
};


#endif
