#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <stdio.h>
#include <list>
#include <vector>
#include <map>
#include <string>
#include <istream>
#include <ostream>
#include <Storm3D_Ui.h>
#include "particle_typedef.h"
#include "particle.h"
#include "float_track.h"
#include "vector_track.h"
#include "particle_desc.h"
#include "emitter_desc.h"
#include "particle_system.h"
#include "particle_system_manager.h"


const float ParticleSystem::TIME_SCALE = 0.01f;
const int ParticleSystem::TIME_STEP = 10;

struct ParticleSystemData {

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
		
		Entry(SharedPtr<EmitterDesc> _ed, SharedPtr<ParticleDesc> _pd, int& totParts, int& maxParts) 
			: ed(_ed), pd(_pd), totalParticles(totParts), maxParticles(maxParts),
			mParticles(0), mFreeParticles(0), mTime(0), mNumParticles(0), mRemainder(0) {
				
			mEndTime = ed->minEmitTime + (ed->maxEmitTime - ed->minEmitTime) * fRand();
			dead = false;
		}
		
		bool tick(const Matrix& tm, const Vector& velocity, IStorm3D_Scene* scene) {
			
			mTime += ParticleSystem::TIME_SCALE;
			if(mTime > mEndTime) {
				if(!ed->dieAfterEmission) {
					mTime -= mEndTime;					
					mEndTime = ed->minEmitTime + (ed->maxEmitTime - ed->minEmitTime) * fRand();
				} else {
					if(mNumParticles == 0)
						return false;
				}
			}

			Particle** pp = &mParticles;
			while(*pp) {
				Particle* p = *pp;
				float t = (pd->maxLife - p->life) / pd->maxLife; 
				p->t += ParticleSystem::TIME_SCALE;
				p->life -= ParticleSystem::TIME_SCALE;
				p->position += p->velocity;	
				p->velocity.y -= (ParticleSystemManager::getGravity() * pd->gravityMultiplier);
				//p->velocity *= pow(p->velocity, 1.0f - pd->dragFactor);
				p->size = pd->sizeTrack.eval(t);
				p->color = pd->colorTrack.eval(t);
				p->alpha = pd->alphaTrack.eval(t);
				p->frame += p->animSpeed;
				p->angle += p->rotSpeed;
				if(p->life < 0) {
					*pp = p->next;
					p->next = mFreeParticles;
					mFreeParticles = p;
					mNumParticles--;
					totalParticles--;
				} else {

					bool died = false;
					if(!p->collisionType == ParticleDesc::CTYPE_NONE) {
						Vector v = p->oldPos - p->position;
						float len = v.GetLength();					
						
						Iterator<IStorm3D_Terrain*>* ti = scene->ITTerrain->Begin();					
						if(ti) {
							IStorm3D_Terrain* t = ti->GetCurrent();
							Storm3D_CollisionInfo ci;
							ObstacleCollisionInfo oci;
							t->RayTrace(p->oldPos, v, len, ci, oci);
							if(ci.hit) {
								p->position = p->position - ci.plane_normal * ci.inside_amount;
								v.Normalize();
								p->velocity *= v.GetDotWith(ci.plane_normal) * p->bounce;
								p->bounce -= ParticleSystem::TIME_SCALE;
								if(p->bounce < 0) {
									died = true;
									*pp = p->next;
									p->next = mFreeParticles;
									mFreeParticles = p;
									mNumParticles--;
									totalParticles--;	
								}
							}
						}

						delete ti;
					}
					if(!died) {
						pp = &p->next;
					}
					
				}
			}
						
			float t = (float)mTime / (float)mEndTime;
			
			if((mTime < mEndTime) && (!dead))
				mRemainder += ed->emitRateTrack.eval(t) * ParticleSystem::TIME_SCALE;		
			
			Vector factorizedVelocity = velocity * ed->velocityFactor;

			while((mRemainder >= 1.0f) && (totalParticles < maxParticles)) {
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
				totalParticles++;
				mRemainder -= 1.0f;
				
				float rnd = fRand();
				
				ed->genPosition(p->position);
				tm.TransformVector(p->position);
				p->oldPos = p->position;
				p->origin = p->position;
				ed->genVelocity(p->velocity);
				p->velocity += factorizedVelocity;
				tm.RotateVector(p->velocity);
				
				p->velocity *= ParticleSystem::TIME_SCALE;
				p->size = pd->sizeTrack.getKeyValue(0);
				p->color = pd->colorTrack.getKeyValue(0);
				p->alpha = pd->alphaTrack.getKeyValue(0);
				p->angle = pd->minAngle + (pd->maxAngle - pd->minAngle) * rnd;
				p->bounce = pd->bounce;
				p->collisionType = pd->collisionType;
				p->frame = pd->texInfo.startFrame;
				p->life = pd->minLife + (pd->maxLife - pd->minLife) * rnd;
				p->t = 0.0f;
				if(pd->texInfo.animType == ParticleDesc::TextureInfo::ANIM_LOOP) {
					p->animSpeed = (pd->texInfo.nFrames / p->life) * ParticleSystem::TIME_SCALE;
				} else {
					p->animSpeed = pd->texInfo.fps * ParticleSystem::TIME_SCALE;
				}
				p->rotSpeed = pd->minSpin + (pd->maxSpin - pd->minSpin) * rnd;
			}
			
			
			return true;

		}

		int render(IStorm3D_Scene* scene, Storm3D_PointParticle* pointParticles, 
			Storm3D_LineParticle* lineParticles) {
			
			Storm3D_ParticleAnimationInfo info;
			info.frames = pd->texInfo.nFrames;
			info.columns = pd->texInfo.columns;
			info.rows = pd->texInfo.rows;
			
			switch(pd->drawStyle) {
			case ParticleDesc::DSTYLE_QUAD:
				{
					int nParts = 0;
					Particle* p = mParticles;
					while(p) {
						Storm3D_PointParticle& shape = pointParticles[nParts++];
						shape.center.position = p->position;
						shape.center.color = COL(p->color.x, p->color.y, p->color.z);
						shape.center.alpha = p->alpha;
						shape.center.size = p->size;
						shape.alive = true;
						shape.frame = p->frame;
						shape.angle = p->angle;
						p = p->next;
					}

//					char buffer[256];
//				sprintf(buffer, "%d particles to render\n", nParts);
//				OutputDebugString(buffer);

					scene->GetParticleSystem()->RenderParticles(pd->getMaterial(), pointParticles, nParts, &info);
					return nParts;
				} break;
			case ParticleDesc::DSTYLE_POINT:
				{
					// TODO: point sprite support
				} break;
			case ParticleDesc::DSTYLE_LINE:
				{
					int nParts = 0;
					Particle* p = mParticles;
					while(p) {
						Storm3D_LineParticle& shape = lineParticles[nParts++];
						
						shape.start.position = p->position;
						shape.start.color = COL(p->color.x, p->color.y, p->color.z);
						shape.start.alpha = p->alpha;
						shape.start.size = p->size;
						
						shape.end.position = p->oldPos;
						shape.end.color = COL(p->color.x, p->color.y, p->color.z);
						shape.end.alpha = p->alpha;
						shape.end.size = p->size;
							
						shape.alive = true;
						shape.frame = p->frame;
						p = p->next;
					}

					scene->GetParticleSystem()->RenderParticles(pd->getMaterial(), lineParticles, nParts, &info);
					return nParts;
				} break;
			}
		
			return 0;
		}
		
	};

	std::vector< SharedPtr<Entry> > mEntries;
	int mTimeCounter;
	int mMaxParticles;
	int mNumParticles;
	float mGravity;
	Matrix mTM;
	Vector mVelocity;
		std::string mName;

		std::vector<Storm3D_PointParticle> mPointParticles;
		std::vector<Storm3D_LineParticle> mLineParticles;


	bool dead;

	IStorm3D* s3d;

	ParticleSystemData(IStorm3D* _s3d, int maxParts) : 
		s3d(_s3d),
		mTimeCounter(0), mNumParticles(0) {
		mVelocity = Vector(0.0f, 0.0f, 0.0f);
		mTM.CreateIdentityMatrix();
		dead = false;
		setMaxParticles(maxParts);
	}
	
	void setName(const std::string& name) {
		mName = name;
	}

	const std::string& getName() {
		return mName;
	}
				
	void copy(const ParticleSystemData& other) {
		mName = other.mName;
		for(int i = 0; i < other.mEntries.size(); i++) {
			addEmitter(other.mEntries[i]->ed, other.mEntries[i]->pd);
		}
	}

	void setVelocity(const Vector& vel) {
		mVelocity = vel;
	}

	void setTM(const Matrix& tm) {
		mTM = tm;
	}

	void addEmitter(SharedPtr<EmitterDesc> ed, SharedPtr<ParticleDesc> pd) {
		SharedPtr<Entry> e(new Entry(ed, pd, mNumParticles, mMaxParticles));
		mEntries.push_back(e);
	}

	int getNumEmitters() {
		return mEntries.size();
	}

	SharedPtr<EmitterDesc> getEmitterDesc(int i) {
		return mEntries[i]->ed;
	}

	SharedPtr<ParticleDesc> getParticleDesc(int i) {
		return mEntries[i]->pd;
	}

	void removeEmitter(int i) {
		if(i < mEntries.size())
			mEntries.erase(mEntries.begin() + i);
	}

	void kill() {
		for(int i = 0; i < mEntries.size(); i++) {
			mEntries[i]->dead = true;
		}
	}

	bool isDead() {
		return dead;
	}

	bool tick(IStorm3D_Scene* scene, int timeDif) {
		
		mTimeCounter += timeDif;
		while(mTimeCounter > ParticleSystem::TIME_STEP) {
			bool b = false;
			mTimeCounter -= ParticleSystem::TIME_STEP;
						
			for(int i = 0; i < mEntries.size(); i++) {
				if(mEntries[i]->tick(mTM, mVelocity, scene))
					b = true;
			} 
			if(!b) {
				dead = true;
				return false;
			}
		}

		return true;
	}

	void setMaxParticles(int n) {
		mMaxParticles = n;
		mLineParticles.resize(n);
		mPointParticles.resize(n);
	}

	int getMaxParticles() {
		return mMaxParticles;
	}
	
	void render(IStorm3D_Scene* scene) {
		
		int n = 0;
		for(int i = 0; i < mEntries.size(); i++) {
			n += mEntries[i]->render(scene, &mPointParticles[n], &mLineParticles[n]);
		}

	}

	std::istream& readStream(std::istream& is) {

		Parser parser;
		is >> parser;

		ParserGroup& g = parser.getGlobals();	
		
		int version = 0;
		::parseIn(g, "version", version);
		::parseIn(g, "max_particles", mMaxParticles);
		::parseIn(g, "gravity", mGravity);
		int n;
		::parseIn(g, "num_emitters", n);	
		for(int i = 0; i < n; i++) {
			std::string str = "emitter";
//			str << n;
			ParserGroup& sub = g.getSubGroup("emitter");
			ParserGroup& eGroup = sub.getSubGroup("emission");
			int type;
			::parseIn(eGroup, "type", type);
			EmitterDesc* ed = NULL;
			switch(type) {
			case ED_SPRAY:
				ed = new SprayEmitterDesc();
				break;
			case ED_POINT_ARRAY:
				ed = new PointArrayEmitterDesc();
				break;
			case ED_CLOUD:
				ed = new CloudEmitterDesc();
				break;
			}
			SharedPtr<EmitterDesc> ped(ed);
			ped->parseIn(eGroup);
			ParserGroup& pGroup = sub.getSubGroup("particle");
			SharedPtr<ParticleDesc> ppd(new ParticleDesc());
			ppd->parseIn(s3d, pGroup);
			addEmitter(ped, ppd);
		}

		return is;
	}

	std::ostream& writeStream(std::ostream& os) {

		Parser parser;

		ParserGroup& g = parser.getGlobals();

		int version = 1;
		::parseOut(g, "version", version);
		::parseOut(g, "max_particles", mMaxParticles);
		::parseOut(g, "num_emitters", (int)mEntries.size());
		for(int i = 0; i < mEntries.size(); i++) {
			
			ParserGroup eGroup;
			::parseOut(eGroup, "type", mEntries[i]->ed->getType());
			mEntries[i]->ed->parseOut(eGroup);
			
			ParserGroup pGroup;
			mEntries[i]->pd->parseOut(pGroup);

			std::string str = "emitter";
		//	str << n;
			ParserGroup e;
			e.addSubGroup("emission", eGroup);
			e.addSubGroup("particle", pGroup);
			g.addSubGroup(str, e);
		
		}

		os << parser;
		
		return os;
	}

};


ParticleSystem::ParticleSystem(IStorm3D* s3d, int maxParticles) {
	ScopedPtr<ParticleSystemData> temp(new ParticleSystemData(s3d, maxParticles));
	m.swap(temp);
}

ParticleSystem::~ParticleSystem() {
	
}

const std::string& ParticleSystem::getTemplateName() {
	return m->getName();
}

void ParticleSystem::setName(const std::string& name) {
	m->setName(name);
}

void ParticleSystem::copy(SharedPtr<ParticleSystem> ps) {
	m->copy(*ps->m);
}

void ParticleSystem::setVelocity(const Vector& vel) {
	m->mVelocity = vel;
}

const Vector& ParticleSystem::getVelocity() {
	return m->mVelocity;
}

void ParticleSystem::setTM(const Matrix& tm) {
	m->mTM = tm;
}

const Matrix& ParticleSystem::getTM() {
	return m->mTM;
}


void ParticleSystem::addEmitter(SharedPtr<EmitterDesc> emitter, SharedPtr<ParticleDesc> particle) {
	m->addEmitter(emitter, particle);
}

int ParticleSystem::getNumEmitters() {
	return m->getNumEmitters();
}

SharedPtr<ParticleDesc> ParticleSystem::getParticleDesc(int i) {
	return m->getParticleDesc(i);
}

SharedPtr<EmitterDesc> ParticleSystem::getEmitterDesc(int i) {
	return m->getEmitterDesc(i);
}

void ParticleSystem::removeEmitter(int i) {
	m->removeEmitter(i);
}

bool ParticleSystem::tick(IStorm3D_Scene* scene, int timeDif) {
	return m->tick(scene, timeDif);
}

void ParticleSystem::render(IStorm3D_Scene* scene) {
	m->render(scene);
}

std::istream& ParticleSystem::readStream(std::istream& is) {
	return m->readStream(is);
}

std::ostream& ParticleSystem::writeStream(std::ostream& os) const {
	return m->writeStream(os);
}

bool ParticleSystem::isDead() {
	return m->isDead();
}

void ParticleSystem::kill() {
	m->kill();
}

void ParticleSystem::setMaxParticles(int n) {
	m->setMaxParticles(n);
}
	
int ParticleSystem::getMaxParticles() {
	return m->getMaxParticles();
}



/*struct ParticleSystemData {

	IStorm3D* mStorm;
	IStorm3D_Scene* mScene;
	
	std::vector< SharedPtr<IParticleEmitter> > mEmitters;
	Vector mPosition;
	Vector mVelocity;
	Matrix mTM;
	std::string mName;
	int mTimeCounter;

	ParticleSystemData(const std::string& name, IStorm3D* storm, IStorm3D_Scene* scene) 
		: mName(name), mStorm(storm), mScene(scene), mTimeCounter(0) {
		mTM.CreateIdentityMatrix();
		mPosition = Vector(0.0f, 0.0f, 0.0f);
		mVelocity = Vector(0.0f, 0.0f, 0.0f);
	}

	void copy(const ParticleSystemData& other) {
		for(int i = 0; i < other.mEmitters.size(); i++) {
			SharedPtr<IParticleEmitter> temp(other.mEmitters[i]->clone());
			mEmitters.push_back(temp);
		}
	}
	
	
	void addParticleEmitter(SharedPtr<IParticleEmitter> emitter) {
		mEmitters.push_back(emitter);
	}

	void removeParticleEmitter(int i) {
		mEmitters.erase(mEmitters.begin()+i);
	}

	SharedPtr<IParticleEmitter> getParticleEmitter(int i) {
		return mEmitters[i];
	}

	int getNumParticleEmitters() {
		return mEmitters.size();
	}

	bool tick(int timeDif) {
		
		OutputDebugString("ps tick\n");

		mTimeCounter += timeDif;
		while(mTimeCounter > 10) {
			bool alive = false;
			mTimeCounter -= 10;
			for(int i = 0; i < mEmitters.size(); i++) {
				if(mEmitters[i]->tick(mScene, mTM, mVelocity, timeDif))
					alive = true;
			}
			if(alive == false)
				return false;
		}	
		
		return true;
	}

	void render() {

		OutputDebugString("ps render\n");

		for(int i = 0; i < mEmitters.size(); i++) {
			mEmitters[i]->render(mScene);
		}

	}


};

ParticleSystem::ParticleSystem(const std::string& name, IStorm3D* storm, 
							   IStorm3D_Scene* scene) {

	ScopedPtr<ParticleSystemData> temp(new ParticleSystemData(name, storm, scene));
	m.swap(temp);
}

void ParticleSystem::operator=(const ParticleSystem& ps) {
	m->copy(*ps.m);
}

ParticleSystem::~ParticleSystem() {

}

IStorm3D* ParticleSystem::getStorm() {
	return m->mStorm;
}

IStorm3D_Scene* ParticleSystem::getScene() {
	return m->mScene;
}

void ParticleSystem::addParticleEmitter(SharedPtr<IParticleEmitter> group) {
	m->addParticleEmitter(group);
}

void ParticleSystem::removeParticleEmitter(int i) {
	m->removeParticleEmitter(i);
}

int ParticleSystem::getNumParticleEmitters() {
	return m->getNumParticleEmitters();
}

SharedPtr<IParticleEmitter> ParticleSystem::getParticleEmitter(int i) {
	return m->getParticleEmitter(i);
}

const std::string& ParticleSystem::getName() {
	return m->mName;
}

void ParticleSystem::setPosition(const Vector& pos) {
	m->mTM.CreateTranslationMatrix(pos);
	m->mPosition = pos;
}

const Vector& ParticleSystem::getPosition() {
	return m->mPosition;
}

void ParticleSystem::setVelocity(const Vector& vel) {
	m->mVelocity = vel;
}

const Vector& ParticleSystem::getVelocity() {
	return m->mVelocity;
}

bool ParticleSystem::tick(int timeDif) {
	return m->tick(timeDif);	
}

void ParticleSystem::render() {
	m->render();
}


*/
