#ifndef SPRITE_EMITTER_H
#define SPRITE_EMITTER_H


class IPositionGen {
public:
	virtual ~IPositionGen() {}
	virtual int  getType()=0;
	virtual void gen(Vector& v)=0;
};

#define POSITIONGEN_BOX 0

class BoxGen : public IPositionGen {
public:
	Vector bmin, bmax;
	virtual int getType();
	virtual void gen(Vector& v);
};

#define POSITIONGEN_SPHERE 1

class SphereGen : public IPositionGen {
public:
	float innerRadius, outerRadius;
	virtual int getType();
	virtual void gen(Vector& v);
};
/*
#define POSITIONGEN_MESH 2

class MeshGen : public IPositionGen {
public:
	void setModel(IStorm3D_ModelObject* obj);
	void setNumVertsUsed();
	void setVertexRange();

};
*/


struct FloatTrackData;
class FloatTrack {
	ScopedPtr<FloatTrackData> m; 
public:
	FloatTrack();
	~FloatTrack();
	void operator=(FloatTrack& other);
	void setNumKeys(int n);
	int getNumKeys();
	float getKeyValue(int i);
	float getKeyTime(int i);
	void setKey(int i, float t, float val);
	float eval(float t);
};

struct VectorTrackData;
class VectorTrack {
	ScopedPtr<VectorTrackData> m; 
public:
	VectorTrack();
	~VectorTrack();
	void operator=(VectorTrack& other);
	void setNumKeys(int n);
	int getNumKeys();
	const Vector& getKeyValue(int i);
	float getKeyTime(int i);
	void setKey(int i, float t, const Vector& v);
	Vector eval(float t);	
};


class ParticleDesc {
protected:

	TextureInfo texInfo;
	VectorTrack colorTrack;
	FloatTrack alphaTrack;
	FloatTrack sizeTrack;
	float minLife, maxLife;
	float minAngle, maxAngle;
	float minAngleChange, maxAngleChange;
	bool affectGravity;
	float frictionFactor;
	int frictionTypeByVelocity, frictionTypeBySize;
	IStorm3D_Material* mMaterial;
	int collisionType;
	int drawStyle;
	float bounce;
	std::string name;
	
public:

	const std::string& getName();
	void setName(const std::string& name);

	TextureInfo& getTextureInfo();
	VectorTrack& getColorTrack();
	VectorTrack& getAlphaTrack();
	FloatTrack& getSizeTrack();

	ParticleDesc();
	ParticleDesc(const ParticleDesc& other);
	
	virtual void loadTexture(const std::string& path, const std::string& name);	
	virtual void render(Particle* particles, IStorm3D_Scene* scene);
};

class EmitterDesc {
protected:

	VectorTrack emitDirectionTrack;
	FloatTrack emitRateTrack;
	FloatTrack sizeMulTrack;
	FloatTrack rotMulTrack;
	float velocityFactor;
	std::string name;
	float minEmitTime;
	float maxEmitTime;
	SharedPtr<IPositionGen> gen;

public:
	
	const std::string& getName();
	void setName(const std::string& name);
	
	VectorTrack& getEmitDirectionTrack();
	FloatTrack& getEmitRateTrack();
	FloatTrack& getSizeMulTrack();
	FloatTrack& getRotMulTrack();	
	float getVelocityFactor();
	void setVelocityFactor(float f);

	void setPositionGen(SharedPtr<IPositionGen> gen);

	SharedPtr<IPositionGen> getPositionGen();
	
};


struct ParticleSystemData {

	struct Entry {
		
		SharedPtr<EmitterDesc> ed;
		SharedPtr<ParticleDesc> pd;

		Particle* mParticles;
		Particle* mFreeParticles;

		int mTime;
		int mEndTime;

		int mNumParticles;
		float mParticleRemainder;
		
		int& totalParticles;
		int& maxParticles;

		Entry() {

			mEndTime = ed->ed->minEmitTime + (ed->maxEmitTime - ed->minEmitTime) * rnd;
		}
		
		bool tick() {
			
			mTime += 10;
			if(mTime > mEndTime) {
				if(ed->bRepeat()) {
					mTime -= mEndTime;
					mEndTime = ed->minEmitTime + (ed->maxEmitTime - ed->minEmitTime) * rnd;
				} else {
					if(mNumParticles == 0)
						return false;
				}
			}

			float t = mTime / mEndTime;

			Particle** pp = mParticles;
			while(*pp) {

			}

			mParticleRemainder += ed->getEmitRateTrack().eval(t);
			while((mParticleRemainder >= 1.0f) && (totalParticles < maxParticles)) {
				Particle* p = NULL;
				if(mFreeParticles) {

				} else {
					p = new Particle();
				}
				p->next = mParticles;
				mParticles = p;
				mNumParticles++;
				totalParticles++;
			}
			

			return true;

		}

		void render() {
			pd->render(mParticles);
		}
	};

	std::vector< SharedPtr<Entry> > mEntries;
	int mTimeCounter;

	ParticleSystemData() {

	}

	void copy(const ParticleSystemData& other) {
		for(int i = 0; i < other.mEntries.size(); i++) {
			Entry* e = new Entry(other.mEntries[i]->ed, other.mEntries[i].pd);
			mEntries.push_back(e);
		}
	}

	bool tick(int timeDif) {
		
		mTimeCounter += timeDif;
		while(mTimeCounter > 10) {

			mTimeCounter -= 10;

			for(int i = 0; i < mEntries.size(); i++) {
				mEntries.tick();
			}

		}


	}

	void render() {

		for(int i = 0; i < mEntries.size(); i++) {
			mEntries[i]->render();
		}

	}


};


class ParticleSystem {
	ScopedPtr<ParticleSystemData> m;
public:
	
	ParticleSystem();

	void copy(SharedPtr<ParticleSystem> ps);
	
	void setMaxParticles(int n);
	
	float setGravity(float f);

	void addEmitter(SharedPtr<EmitterDesc> emitter, 
		SharedPtr<ParticleDesc> particle);

	int getNumEmitters();

	SharedPtr<ParticleDesc> getParticleDesc(int i);

	SharedPtr<EmitterDesc> getEmitterDesc(int i);

	void removeEmitter(int i);

	void tick(int timeDif);

	void render();

};


class ParticleSystemManager {
public:

	bool loadParticleSystem(const std::string& filename);

	SharedPtr<ParticleSystem> spawnParticleSystem(const std::string& name);

	void tick(int timeDif);

	void render();
};



struct SpriteEmitterData;
class SpriteEmitter : public IParticleEmitter {
	ScopedPtr<SpriteEmitterData> m;
public:
	
	struct TextureInfo {
		
		void operator=(const TextureInfo& other);
		
		void setDefaults();

		std::string name;
		std::string path;
		int nFrames;
		int frameWidth;
		int frameHeight;
		int alphaType;
		int animType;
		float startFrame;
		float fps;
	};

	enum DRAW_STYLE {
		DS_POINT = 0,
		DS_QUAD,
		DS_LINE
	};

	enum COLLISION_TYPE {
		CT_NONE = 0,
		CT_DIE = 1,
		CT_BOUNCE = 2
	};
	

	TextureInfo texInfo;
	VectorTrack colorTrack;
	FloatTrack alphaTrack;
	FloatTrack sizeTrack;
	FloatTrack emitRateTrack;
	VectorTrack emitDirectionTrack;
	Vector minVelocity, maxVelocity;
	float minLife, maxLife;
	float velocityFactor;
	float minAngle, maxAngle;
	float minAngleChange, maxAngleChange;
	float minEmitTime, maxEmitTime;
	bool  loop;
	bool  affectForces;
	float frictionFactor;
	int   frictionTypeByVelocity, frictionTypeBySize;
	float gravity;
	int	  collisionType;
	int	  drawStyle;
	float bounceFactor;
	Vector position;
	int maxParticles;
	
	
	TextureInfo& getTextureInfo();
	VectorTrack& getColorTrack();
	VectorTrack& getAlphaTrack();
	FloatTrack& getSizeTrack();
	FloatTrack& getEmitRateTrack();
	VectorTrack& getEmitDirectionTrack();
	const Vector& getMinVelocity();
	void setMinVelocity(const Vector& vel);
	const Vector& getMaxVelocity();
	void setMaxVelocity(const Vector& vel);
	float getMinLife();
	void setMinLife(float life);
	float getMaxLife();
	void setMaxLife(float life);
	float setVelocityFactor();
	void setVelocityFactor(float f);
	float getMinAngle();
	void setMinAngle(float f);
	float getMaxAngle();
	void setMaxAngle(float f);
	float getMinAngleChange();
	void setMinAngleChange(float f);
	float getMaxAngleChange();
	void setMaxAngleChange(float f);
	float getMinEmitTime();
	void setMinEmitTime(float f);

	SpriteEmitter();
	
	void setInstance(SharedPtr<IParticleEmitter> emitter);
	
	SharedPtr<IParticleEmitter> clone();
	
	void setDefaults();

	const std::string& getName();

	void setName(const std::string& name);
	
	void setPositionGen(SharedPtr<IPositionGen> gen);

	SharedPtr<IPositionGen> getPositionGen();

	void loadTexture(const std::string& name, const std::string& path, IStorm3D* s3d);
	
	virtual bool tick(IStorm3D_Scene* scene, const Matrix& tm, 
		const Vector& velocity, int timeDif);	
	
	virtual void render(IStorm3D_Scene* scene);
};









class SpriteEmitterFactory : public IEmitterFactory {
public:
	IParticleEmitter* create();
	const char* getClassName();
	int getClassID();
};



#endif