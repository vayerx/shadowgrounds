#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

namespace frozenbyte
{

namespace particle
{

class ParticleSystem;
class Particle;
class ParticleForce;
class ParticleSystemManager;
class GenParticleSystem;
class ParticleSystemClassDesc;
class Track;
class VectorTrack;
class FloatTrack;


class ParticleSystemClassDesc {
public:
	virtual ~ParticleSystemClassDesc() {}
	virtual void* create()=0;
	virtual const char* getClassName()=0;
};

class ParticleSystem {
public:
	virtual ~ParticleSystem() {}
	virtual const char* getClassName()=0;
	virtual const char* getSuperClassName()=0;		
	virtual ParticleSystem* launch()=0;	
	virtual int  getNumParticles()=0;
	virtual void setTarget(const Vector& target)=0;
	virtual void setTM(const Matrix& tm)=0;
	virtual void setVelocity(const Vector& vel)=0;
	virtual void parseFrom(const editor::ParserGroup& pg)=0;
	virtual void parseTo(editor::ParserGroup& pg)=0;
	virtual void kill()=0;
	virtual bool isDead()=0;
	virtual void create()=0;
	virtual void init(IStorm3D* s3d, IStorm3D_Scene* scene)=0;
	virtual void prepareForLaunch(IStorm3D* s3d, IStorm3D_Scene* scene)=0;
	virtual bool tick(IStorm3D_Scene* scene)=0;
	virtual void render(IStorm3D_Scene* scene)=0;


};


class Particle {
public:
	bool alive;
	Vector position;
	Vector velocity;
	float  age;
	float  life;
	float  frame;
	float  frameSpeed;
	float  angle;
	float  angleSpeed;
};


class ParticleForceClassDesc {
public:
	virtual ~ParticleForceClassDesc() {}
	virtual void* create()=0;
	virtual const char* getClassName()=0;
};

class ParticleForce {
public:
	virtual ~ParticleForce() {}
	virtual void parseFrom(const editor::ParserGroup& pg)=0;
	virtual void parseTo(editor::ParserGroup& pg)=0;
	virtual const char* getClassName()=0;
	virtual const char* getSuperClassName()=0;
	virtual void create()=0;
	virtual void preCalculate(float t)=0;
	virtual void calcForce(Vector& force, const Vector& pos, const Vector& vel)=0;
};

enum GEN_PARTICLE_FORCE_PARAMS
{
	GEN_PARTICLE_FORCE_PARAM_COUNT = 0
};

class GenParticleForce : public ParticleForce {
protected:
	boost::shared_ptr<ParamBlock> m_pb;
public:
	virtual ~GenParticleForce() {}
	boost::shared_ptr<ParamBlock> getParamBlock() { return m_pb; }
	void parseFrom(const editor::ParserGroup& pg);
	void parseTo(editor::ParserGroup& pg);
	virtual void create() {
		boost::shared_ptr<ParamBlock> pb(new ParamBlock());
		m_pb.swap(pb);
	}
	virtual void preCalculate(float t) {}
	virtual void calcForce(Vector& force, const Vector& pos, const Vector& vel) {}
};


enum GEN_PARTICLE_SYSTEM_PARAMS
{
	PB_EMIT_RATE = 0,
	PB_EMIT_START_TIME,
	PB_EMIT_STOP_TIME,
	PB_MAX_PARTICLES,
	PB_DIE_AFTER_EMISSION,
	PB_VELOCITY_INHERITANCE_FACTOR,
	PB_EMITTER_POSITION,
	PB_LAUNCH_SPEED,
	PB_LAUNCH_SPEED_VAR,
	PB_PARTICLE_COLOR,
	PB_PARTICLE_ALPHA,
	PB_PARTICLE_SIZE,
	PB_PARTICLE_LIFE,
	PB_PARTICLE_LIFE_VAR,
	PB_PARTICLE_START_ANGLE,
	PB_PARTICLE_START_ANGLE_VAR,
	PB_PARTICLE_SPIN,
	PB_PARTICLE_SPIN_VAR,
	PB_PARTICLE_TEXTURE,
	PB_PARTICLE_TEXTURE_SUB_DIV_U,
	PB_PARTICLE_TEXTURE_SUB_DIV_V,
	PB_PARTICLE_TEXTURE_ALPHA_TYPE,
	PB_PARTICLE_ANIMATION_FRAME_COUNT,
	PB_PARTICLE_ANIMATION_START_FRAME,
	PB_PARTICLE_ANIMATION_START_FRAME_VAR,
	PB_PARTICLE_ANIMATION_TYPE,
	PB_PARTICLE_ANIMATION_FPS,
	GEN_PARTICLE_SYSTEM_PARAM_COUNT
};

class GenParticleSystem : public ParticleSystem {
protected:

	std::vector<Particle> m_parts;	
	std::vector< boost::shared_ptr<ParticleForce> > m_forces;
	std::vector<Storm3D_PointParticle> m_mesh;
	Storm3D_ParticleAnimationInfo m_animInfo;
	boost::shared_ptr<ParamBlock> m_pb;
	Matrix m_tm;
	Vector m_velocity;
	Vector m_target;
	bool m_trackTarget;
	bool m_alive;
	bool m_shutdown;
	float m_time;
	float m_particleResidue;
	int m_maxParticles;
	int m_numParts;
	IStorm3D_Material* m_mtl;
//	boost::shared_ptr<VectorTrack> m_colorTrack;
//	boost::shared_ptr<FloatTrack> m_sizeTrack;
//	boost::shared_ptr<FloatTrack> m_alphaTrack;
	
//	VectorTrack m_colorTrack;
//	FloatTrack m_sizeTrack;
//	FloatTrack m_alphaTrack;

	void baseCopy(GenParticleSystem* ps);

public:
		
	GenParticleSystem();
	virtual ~GenParticleSystem();

//	void addForce(ParticleForce* force);
//	int getNumForces();
//	void removeForce(int i);
//	void clearForces();
	
	boost::shared_ptr<ParamBlock> getParamBlock();

	virtual const char* getClassName();
	virtual const char* getSuperClassName();
	
	virtual void setParticlePosition(Vector& v);
	virtual void setParticleVelocity(Vector& v, float speed);
		
	int  getNumParticles();
	void setTarget(const Vector& target);
	void setTM(const Matrix& tm);
	void setVelocity(const Vector& vel);
	void parseFrom(const editor::ParserGroup& pg);
	void parseTo(editor::ParserGroup& pg);
	void kill();
	bool isDead();
		
	virtual void create();
	virtual void init(IStorm3D* s3d, IStorm3D_Scene* scene);
	virtual void prepareForLaunch(IStorm3D* s3d, IStorm3D_Scene* scene);
	virtual bool tick(IStorm3D_Scene* scene);
	virtual void render(IStorm3D_Scene* scene);
};

} // particle
 
} // frozenbyte

#endif