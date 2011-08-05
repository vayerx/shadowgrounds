#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

/*
class Gun {
public:
	ParticleSystem* ps;

	void fire() {
		if(ps) {
			ps->launch();
		}
	}
};


class ParticleSystem {
public:

	struct Entry {
		
		SharedPtr<EmitterDesc> ed;
		SharedPtr<ParticleDesc> pd;

		Particle* mParticles;
		Particle* mFreeParticles;

		float mTime;
		float mEndTime;

		int mNumParticles;
		float mRemainder;
		
		int& totalParticles;
		int& maxParticles;

		bool dead;

		std::vector<Storm3D_PointParticle> mPointParticles;
		std::vector<Storm3D_LineParticle> mLineParticles;

		inline float fRand() {
			return (float)(rand() % RAND_MAX) / (float)RAND_MAX;
		}
		
		Entry(SharedPtr<EmitterDesc> _ed, SharedPtr<ParticleDesc> _pd, int& totParts, int& maxParts);
				
		bool tick(const Matrix& tm, const Vector& velocity, IStorm3D_Scene* scene);
						
		int render(IStorm3D_Scene* scene, Storm3D_PointParticle* pointParticles, 
			Storm3D_LineParticle* lineParticles);			
		
	
	};

	ParticleSystem(ParticleSystemManager* mgr);
	EmitterDesc* addEmitter(const std::string& emitterType);
	int getNumEmitters();
	EmitterDesc* getEmitterDesc(int i);
	ParticleDesc* getParticleDesc(int i);		
	void launch();
	bool tick();
	void render();

};
*/


struct ParticleSystemData;
class ParticleSystem {

	friend class ParticleSystemManager;
	
	ScopedPtr<ParticleSystemData> m;

	ParticleSystem(IStorm3D* s3d, int nMaxParticls=100);

	void setName(const std::string& name);
	void copy(SharedPtr<ParticleSystem> ps);
	
	bool tick(IStorm3D_Scene* scene, int timeDif);
	void render(IStorm3D_Scene* scene);

public:
	
	~ParticleSystem();

	// update 100 time per sec by default
	static const float TIME_SCALE;
	static const int TIME_STEP;
	
	const std::string& getTemplateName();
	void setVelocity(const Vector& vel);
	const Vector& getVelocity();
	void setTM(const Matrix& tm);
	const Matrix& getTM();
	void setMaxParticles(int n);
	int getMaxParticles();
	void addEmitter(SharedPtr<EmitterDesc> emitter, 
		SharedPtr<ParticleDesc> particle);
	int getNumEmitters();
	SharedPtr<ParticleDesc> getParticleDesc(int i);
	SharedPtr<EmitterDesc> getEmitterDesc(int i);
	void removeEmitter(int i);
	std::ostream& writeStream(std::ostream& os) const;
	std::istream& readStream(std::istream& is);
	bool isDead();
	void kill();
};

inline std::ostream &operator << (std::ostream &stream, const ParticleSystem &ps) 
{ 
	return ps.writeStream(stream);
}

inline std::istream &operator >> (std::istream &stream, ParticleSystem &ps) 
{ 
	return ps.readStream(stream);
}


#endif
