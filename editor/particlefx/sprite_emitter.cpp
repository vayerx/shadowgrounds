#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <stdio.h>
#include <list>
#include <vector>
#include <map>
#include <string>
#include <Storm3D_Ui.h>
#include "particle_typedef.h"
#include "iparticle_emitter.h"
#include "particle.h"
#include "particle_system.h"
#include "sprite_emitter.h"




struct SpriteEmitterData {

	SpriteEmitter* self;	
	Particle* mParticles;
	Particle* mFreeParticles;
	int mEndTime;
	SharedPtr<IPositionGen> mPosGen;
	IStorm3D_Material* mMaterial;
	std::string mName;
	int mTime;
	std::vector<Storm3D_PointParticle> mPointParticles;
	std::vector<Storm3D_LineParticle> mLineParticles;
	int mNumParticles;
	float mRemainder;

	SpriteEmitterData(SpriteEmitter* _self) : self(_self), mTime(0), mNumParticles(0),
		mMaterial(NULL), mParticles(NULL), mFreeParticles(NULL), mRemainder(0.0f) {
		
		mEndTime = 10 * 1000;
	}

	~SpriteEmitterData() {
		
	}

	void copy(SpriteEmitterData& data) {
		mMaterial = data.mMaterial;	
	}
	
	void setName(const std::string& name) {
		mName = name;
	}

	const std::string& getName() {
		return mName;
	}
	
	void setPositionGen(SharedPtr<IPositionGen> gen) {
		mPosGen = gen;
	}
	
	SharedPtr<IPositionGen> getPositionGen() {
		return mPosGen;
	}

	void loadTexture(const std::string& name, const std::string& path, IStorm3D* s3d) {
		if(mMaterial == NULL) {
			mMaterial = s3d->CreateNewMaterial((mName + name).c_str());
		}
		IStorm3D_Texture* tex = s3d->CreateNewTexture(name.c_str());
		if(tex != NULL) {
			mMaterial->SetBaseTexture(tex);
			mMaterial->SetAlphaType(IStorm3D_Material::ATYPE_ADD);
		}
	}

	bool tick(IStorm3D_Scene* scene, const Matrix& tm, const Vector& velocity, int timeDif) {

		float rnd = (float)(rand() % RAND_MAX) / (float)RAND_MAX;

		mTime += 10;
		if(mTime > mEndTime) {
			if(self->loop) {
				mTime -= mEndTime;
				mEndTime = (int)((self->minEmitTime + (self->maxEmitTime - self->minEmitTime) * rnd) * 1000.0f);
			} else {
				if(mNumParticles == 0) {
					return false;
				}
			}
		}

		Particle** pp = &mParticles;
		while(*pp) {
			Particle* p = *pp;
			float t = (self->maxLife - p->life) / self->maxLife; 
			p->position += p->velocity;	
			p->velocity.y -= (self->gravity * 0.01f);
			p->size = self->sizeTrack.eval(t);
			Vector c = self->colorTrack.eval(t);
			p->color.r = c.x;
			p->color.g = c.y;
			p->color.b = c.z;
			p->alpha = self->alphaTrack.eval(t);				
			p->life -= (1.0f / 100.0f);
			if(p->life < 0) {
				*pp = p->next;
				p->next = mFreeParticles;
				mFreeParticles = p;
				mNumParticles--;
			} else {
				pp = &p->next;
			}
		}
	
		float t = (float)mTime / (float)mEndTime;
		
		if(mTime < mEndTime)
			mRemainder += self->emitRateTrack.eval(t) * 0.01f;		

		Vector baseVelocity = self->emitDirectionTrack.eval(t) + velocity * self->velocityFactor;
		Vector minVelocity = self->minVelocity;
		Vector maxVelocity = self->maxVelocity;
		//tm.TransformVector(baseVelocity);
		tm.TransformVector(minVelocity);
		tm.TransformVector(maxVelocity);
				
		while((mRemainder >= 1.0f) && (mNumParticles < self->maxParticles)) {
			Particle* p = NULL;
			if(mFreeParticles) {
				p = mFreeParticles;
				mFreeParticles = p->next;
			} else {
				p = new Particle;
			}
			
			p->next = mParticles;
			mParticles = p;
			mNumParticles++;
			mRemainder -= 1.0f;
			
			mPosGen->gen(p->position);
			p->position = self->position;
			p->position += tm.GetTranslation();
			p->oldPos = p->position;
			p->velocity = baseVelocity * 0.01f;
			//p->velocity += (minVelocity + (maxVelocity - minVelocity) * rnd);
			p->size = 0.0f;
			p->color = COL(0.0f, 0.0f, 0.0f);
			p->alpha = 0.0f;
			p->angle = self->minAngle + (self->maxAngle - self->minAngle) * rnd;
			p->bounce = self->bounceFactor;
			p->collisionType = self->collisionType;
			p->frame = self->texInfo.startFrame;
			p->life = self->minLife + (self->maxLife - self->minLife) * rnd;
		}

//		OutputDebugString("tick\n");

		return true;

	}

	void render(IStorm3D_Scene* scene) {
		
//				OutputDebugString("render\n");

		switch(self->drawStyle) {
		case SpriteEmitter::DS_POINT:
			// not supported
			break;
		case SpriteEmitter::DS_QUAD:
			{
				if(mPointParticles.empty()) {
					mPointParticles.resize(self->maxParticles);
				}				
				int nParts = 0;
				Particle* p = mParticles;
				while(p) {
					Storm3D_PointParticle& shape = mPointParticles[nParts++];
					shape.center.position = p->position;
					shape.center.color = p->color;
					shape.center.alpha = 1.0f;
					shape.center.size = p->size;//1.0f;
					shape.alive = true;
					p = p->next;
				}
				char buffer[256];
				sprintf(buffer, "%d particles to render\n", nParts);
//				OutputDebugString(buffer);
				scene->GetParticleSystem()->RenderParticles(mMaterial, &mPointParticles[0], nParts);			
			}
			break;
		case SpriteEmitter::DS_LINE:
			{
				int nParts = 0;
				Particle* p = mParticles;
				while(p) {
					
					p = p->next;
				}
				scene->GetParticleSystem()->RenderParticles(mMaterial, &mLineParticles[0], nParts);
			}
			break;
		}

//	OutputDebugString("render end\n");

	}
	

};

SpriteEmitter::SpriteEmitter() {
	ScopedPtr<SpriteEmitterData> temp(new SpriteEmitterData(this));
	m.swap(temp);
}

IParticleEmitter* SpriteEmitter::clone() {
	
	SpriteEmitter* emitter = new SpriteEmitter();
	
	emitter->setName(getName());
	emitter->setPositionGen(getPositionGen());
	emitter->texInfo = texInfo;
	emitter->maxParticles = maxParticles;
	emitter->colorTrack = colorTrack;
	emitter->alphaTrack = alphaTrack;
	emitter->sizeTrack = sizeTrack;
	emitter->emitRateTrack = emitRateTrack;
	emitter->emitDirectionTrack = emitDirectionTrack;
	emitter->minVelocity = minVelocity;
	emitter->maxVelocity = maxVelocity;
	emitter->velocityFactor = velocityFactor;
	emitter->minLife = minLife;
	emitter->maxLife = maxLife;
	emitter->minAngle = minAngle;
	emitter->maxAngle = maxAngle;
	emitter->minAngleChange = minAngleChange;
	emitter->maxAngleChange = maxAngleChange;
	emitter->minEmitTime = minEmitTime;
	emitter->maxEmitTime = maxEmitTime;
	emitter->loop = loop;
	emitter->affectForces = affectForces;
	emitter->frictionFactor = frictionFactor;
	emitter->frictionTypeBySize = frictionTypeBySize;
	emitter->frictionTypeByVelocity = frictionTypeByVelocity;
	emitter->gravity = gravity;
	emitter->collisionType = collisionType;
	emitter->drawStyle = drawStyle;
	emitter->bounceFactor = bounceFactor;
	emitter->position = position;
	emitter->m->copy(*m);

	return emitter;
}

void SpriteEmitter::setName(const std::string& name) {
	m->setName(name);
}

const std::string& SpriteEmitter::getName() {
	return m->getName();
}
	
void SpriteEmitter::setPositionGen(SharedPtr<IPositionGen> gen) {
	m->setPositionGen(gen);
}

SharedPtr<IPositionGen> SpriteEmitter::getPositionGen() {
	return m->getPositionGen();
}

void SpriteEmitter::loadTexture(const std::string& name, const std::string& path, IStorm3D* s3d) {
	m->loadTexture(name, path, s3d);
}
	
bool SpriteEmitter::tick(IStorm3D_Scene* scene, const Matrix& tm, 
						 const Vector& velocity, int timeDif) {
	
	return m->tick(scene, tm, velocity, timeDif);
}
	
void SpriteEmitter::render(IStorm3D_Scene* scene) {
	m->render(scene);
}

void SpriteEmitter::TextureInfo::operator=(const SpriteEmitter::TextureInfo& other) {

	name = other.name;
	path = other.path;
	nFrames = other.nFrames;
	frameWidth = other.frameWidth;
	frameHeight = other.frameHeight;
	alphaType = other.alphaType;
	animType = other.animType;
	startFrame = other.startFrame;
	fps = other.fps;

}

void SpriteEmitter::TextureInfo::setDefaults() {
	nFrames = 0;
	frameWidth = 32;
	frameHeight = 32;
	alphaType = 0;
	animType = 0;
	startFrame = 0.0f;
	fps = 18.0f;
}


void SpriteEmitter::setDefaults() {

	texInfo.setDefaults();

	colorTrack.setNumKeys(2);
	colorTrack.setKey(0, 0, Vector(1.0f, 1.0f, 1.0f));
	colorTrack.setKey(1, 1.0f, Vector(0.0f, 0.0f, 0.0f));
	
	alphaTrack.setNumKeys(2);
	alphaTrack.setKey(0, 0, 1.0f);
	alphaTrack.setKey(1, 1.0f, 0.0f);

	sizeTrack.setNumKeys(2);
	sizeTrack.setKey(0, 0.0f, 1.0f);
	sizeTrack.setKey(1, 1.0f, 1.0f);
	
	emitRateTrack.setNumKeys(2);
	emitRateTrack.setKey(0, 0, 10.0f);
	emitRateTrack.setKey(1, 1.0f, 10.0f);

	emitDirectionTrack.setNumKeys(2);
	emitDirectionTrack.setKey(0, 0.0f, Vector(0.0f, 1.0f, 0.0f));
	emitDirectionTrack.setKey(1, 1.0f, Vector(0.0f, 1.0f, 0.0f));

	minVelocity = Vector(0.0f, 0.0f, 0.0f);
	maxVelocity = Vector(0.0f, 0.0f, 0.0f);

	velocityFactor = 0.0f;

	minAngle = 0.0f;
	maxAngle = 0.0f;

	minAngleChange = 0.0f;
	maxAngleChange = 0.0f;

	minLife = 0.5f;
	maxLife = 1.0f;
	
	minEmitTime = 10.0f;
	maxEmitTime = 10.0f;

	loop = false;

	affectForces = false;

	frictionFactor = 1.0f;

	frictionTypeByVelocity = 0;

	frictionTypeBySize = 0;

	collisionType = 0;

	drawStyle = SpriteEmitter::DS_QUAD;

	bounceFactor = 0.0f;

	position = Vector(0.0f, 0.0f, 0.0f);

	maxParticles = 100;

	gravity = 0.001f;

	SharedPtr<SphereGen> sg(new SphereGen());
	sg->innerRadius = 0.0f;
	sg->outerRadius = 1.0f;

	setPositionGen(sg);


}

/*
class ParticleGroup {

	ScopedPtr<ParticleGroupData> m;
	SharedPtr<ParticleGroup> ref;
	bool isInstanced;

	VectorTrack colorTrack;
	FloatTrack alphaTrack;
	FloatTrack sizeTrack;
	float minLife, maxLife;
	std::string textureName;
	int alphaType;


public:
	ParticleGroup();
	ParticleGroup(SharedPtr<ParticleGroup> instance);

	VectorTrack& getVectorTrack();

	FloatTrack& getAlphaTrack();

	FloatTrack& getSizeTrack();
	
		
	void setTexture(const std::string& filename) {
		ParticleGroup* ptr = isInstanced ? ref.get() : this;
		ptr->m->setTexture(filename);
	}

	void setAlphaType(IStorm3D_Material::ATYPE type) {
		ParticleGroup* ptr = isInstanced ? ref.get() : this;
		ptr->m->setAlphaType(type);
	}
	
	IStorm3D_Material* getMaterial() {

	}
	
	int getNumParticles();
	void addParticle(const Vector& pos, const Vector& vel);
	void moveParticles();
	Particle* getParticles();
};



class ISource {
public:
	virtual ~ISource();
	virtual float getEmitRate(float t);
	virtual void emit(ParticleGroup& group, int nParts);
};

class IRenderer {
public:
	virtual ~IRenderer();
	virtual void render(ParticleGroup& group);
};


struct ParticleEmitterData {
	
	SharedPtr<ParticleGroup> group;
	SharedPtr<ISource> source;
	SharedPtr<IRenderer> renderer;

	std::vector< SharedPtr<IModifier> > mods;

	ParticleEmitterData() {
		group = new ParticleGroup();
	}

	ParticleEmitterData(ParticleGroupData& other) {
		group = new ParticleGroup();
		group->setInstance(other.group);
		source = other.source;
		renderer = other.renderer;
		for(int i = 0; i < other.mods.size(); i++) {
			mods.push_back(other.mods[i]);
		}
	}

	bool tick() {
	
		if(mTime > mEndTime) {
			if(source->loop) {
				mTime -= mEndTime;
				mEndTime = source->getEndTime();
			} else {
				if(group.getNumParticles()==0)
					return false;
			}
		}

		group.moveParticles();
		for(mods) {
			mod->modify(group.getParticles());
		}
					
		float t = mTime / mEndTime;
				
		mParticleRemainder += source->getEmitRate(t);
		
		if(mParticleRemainder >= 1.0f)	
			source->emit(group, t, (int)mParticleRemainder);

		mParticleRemainder -= (int)mParticleRemainder;
	}

	void render() {

	}

};

class ParticleEmitter {
	ScopedPtr<ParticleEmitterData> m;
public:

	ParticleEmitter();	
	ParticleEmitter(SharedPtr<ParticleEmitter> other);

	SharedPtr<ParticleGroup> getParticleGroup();

	void setSource(SharedPtr<ISource> source);
	void setRenderer(SharedPtr<IRenderer> renderer);
	void addModifier(SharedPtr<IModifier> mod);

	bool tick(const Matrix& mtx, const Vector& velocity);
	void render();
};

ParticleEmitter::ParticleEmitter() {
	ScopedPtr<ParticleEmitterData> temp(new ParticleEmitterData());
	m.swap(temp);
}

ParticleEmitter::ParticleEmitter(SharedPtr<ParticleEmitter> other) {
	ScopedPtr<ParticleEmitterData> temp(new ParticleEmitterData());
	m.swap(temp);
	m->copy(other.m);
}


struct ParticleSystemData {
	
	std::vector< SharedPtr<ParticleEmitter> > mEmitters;
	
	ParticleSystemData() {
		
	}

	void copy(ParticleSystemData& other) {
		for(int i = 0; i < other.mEmitter; i++) {
			SharedPtr<ParticleEmitter> emitter = new ParticleEmitter(other.mEmitters[i]);
			mEmitters.push_back(emitter);
		}
	}
	
	SharedPtr<ParticleEmitter> newEmitter() {

	}

	SharedPtr<ParticleEmitter> getEmitter() {

	}

	int getNumEmitters() {
		
	}

	void deleteEmitter() {

	}

	bool tick(int timeDif) {
	
		mTime += timeDif;
		if(mTime > 10) {
			bool alive = false;
			mTime -= 10;
			for(int i = 0; i < mEmitters.size(); i++) {
				if(mEmitters[i]->tick())
					alive = true;
			}
			if(!alive)
				return false;
		}
		
		return true;
	}

	void render() {

		for(int i = 0; i < mEmitters.size(); i++) {
			mEmitters[i]->render();
		}
	
	}

};

struct ParticleSystemData;
class ParticleSystem {
	ScopedPtr<ParticleSystemData> m;
public:
	ParticleSystem();
	ParticleSystem(SharedPtr<ParticleSystem> other);

	SharedPtr<ParticleEmitter> newEmitter();
	SharedPtr<ParticleEmitter> getEmitter(int i);
	int getNumEmitters();
	void deleteEmitter(int i);
	bool tick(int timeDif);
	void render();
};

ParticleSystem::ParticleSystem() {
	ScopedPtr<ParticleSystemData> temp(new ParticleSystemData());
	m.swap(temp);
}

ParticleSystem::ParticleSystem(SharedPtr<ParticleSystem> other) {
	ScopedPtr<ParticleSystemData> temp(new ParticleSystemData());
	m.swap(temp);
	m->copy(*other.m);	
}

SharedPtr<ParticleEmitter> ParticleSystem::newEmitter() {
	return m->newEmitter();
}

int ParticleSystem::getNumEmitters() {
	return m->getNumEmitters();
}

void ParticleSystem::deleteEmitter(int i) {
	return m->deleteEmitter(i);
}

bool ParticleSystem::tick(int timeDif) {
	m->tick(timeDif);
}

void ParticleSystem::render() {
	m->render();
}




class ParticleGroup {
	ScopedPtr<ParticleGroupData> m;
	SharedPtr<ParticleGroup> ref;
	bool isInstanced;

	VectorTrack colorTrack;
	FloatTrack alphaTrack;
	FloatTrack sizeTrack;
	float minLife, maxLife;
	std::string textureName;
	int alphaType;


public:
	ParticleGroup();
	ParticleGroup(SharedPtr<ParticleGroup> instance);

	VectorTrack& getVectorTrack();

	FloatTrack& getAlphaTrack();

	FloatTrack& getSizeTrack();
	
	
	void setTexture(const std::string& filename) {
		ParticleGroup* ptr = isInstanced ? ref.get() : this;
		ptr->m->setTexture(filename);
	}

	void setAlphaType(IStorm3D_Material::ATYPE type) {
		ParticleGroup* ptr = isInstanced ? ref.get() : this;
		ptr->m->setAlphaType(type);
	}
	
	int getNumParticles();
	void addParticle(const Vector& pos, const Vector& vel);
	void moveParticles();
	Particle* getParticles();
};





class ISource {
	SharedPtr<ISource> ref;
	bool isInstance;
public:
	
	virtual ~ISource();
	
	void emit(ParticleGroup& group, int nParts) {
		for(int i = 0; i < nParts; i++) {
			group.addParticle(pos, vel);
		}
	}
	
	void setIntance(SharedPtr<ISource> source) {
		ref = source;
		isInstance = true;
	}
	
	void transform(const Matrix& tm, const Vector& velocity) {
		ISource* src = isInstance ? ref.get() : this;
		src->_transform(tm, velocity);
	}

	void emit(ParticleGroup& group) {
		ISource* src = isInstance ? ref.get() : this;
		src->_emit(group);
	}

	virtual void _transform(const Matrix& tm, const Vector& velocity)=0;
	virtual void _emit(ParticleGroup& group)=0;
};



struct SphereSourceData {

	FloatTrack emitRateTrack;
	
	void emit(ParticleGroup& group, float t) {

		remainder += emitRateTrack.eval(t);		
		while(remainder >= 1.0f) {
			Vector pos = ;
			Vector vel = ;
			group.addParticle(pos, vel);		
		}

	}

};

class SphereSource : public ISource {
	SharedPtr<SphereSourceData> m;	
public:

	FloatTrack& getEmitRateTrack();
	float getInnerRadius();
	float getOuterRadius();
	void setInnerRadius(float f);
	void setOuterRadius(float f);
	
	void _transform(const Matrix& tm, const Vector& velocity);
	
	void _emit(ParticleGroup& group);

};

void SphereSource::_emit(ParticleGroup& group) {
	m->emit(group);
}

void SphereSource::_transform(const Matrix& tm, const Vector& velocity) {
	m->transform(tm, velocity);
}




class IRenderer {
public:
	virtual void render(ParticleGroup& group);
};


class SpriteRenderer : public IRenderer {
public:
	void render(ParticleGroup& group);	
};





template <class T>
class IntancedObject {
	SharedPtr<T> original;
	bool isInstance;
public:
	void setInstance(SharedPtr<T> toInstance);
	T* getInstance();
};






class Emitter {
	boost::shared_ptr<Emitter> instance;
	bool instanced;
public:
		
	Emitter(boost::shared_ptr<ParticleSettings> settings) {
	
	}

	virtual void createInstance(boost::shared_ptr<Emitter> inst);

	void tick() {
		Emitter* emitter = instanced ? instance.get() : this;
		do stuff...
	}

};


ParticleSystem* createDefaultPS() {

	Emitter* emitter = ps->newEmitter();
	emitter->setSource(sphereSource);
	emitter->addMod(gravity);
	emitter->addMod(drag);
	emitter->setRenderer(spriteRenderer);
	
}*/