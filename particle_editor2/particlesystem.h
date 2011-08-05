// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include <IStorm3D_Particle.h>

namespace util {
	class SoundMaterialParser;
} // util

namespace frozenbyte {
namespace particle {

const int FLUID_PARTICLE_FACTOR = 40;

class IParticleSystem;
class IParticleCollision;
class IParticleArea;
class Particle;
class IParticleForce;
class ParticleSystemManager;
class GenParticleSystemEditables;
class GenParticleSystem;
class ParticleSystemClassDesc;
class Track;
class VectorTrack;
class FloatTrack;
class ParticlePhysics;
class PhysicsFluid;

class Particle {
public:
	bool alive;
	Vector position;
	Vector origin;
	COL	   originColor;
	float  originAlpha;
	float  originSize;
	COL	   previousColor;
	float  previousAlpha;
	float  previousSize;
	Vector velocity;
	float  age;
	float  life;
	float  frame;
	float  frameSpeed;
	float  angle;
	float  angleSpeed;
	float forceAlpha;
};

class IParticleSystem {
public:
	virtual ~IParticleSystem() {}
	virtual boost::shared_ptr<IParticleSystem> clone()=0;
	virtual bool isDead()=0;
	virtual void kill()=0;		
	virtual void init(IStorm3D* s3d, IStorm3D_Scene* scene)=0;
	virtual void tick(IStorm3D_Scene* scene)=0;
	virtual void render(IStorm3D_Scene* scene)=0;
	virtual void parseFrom(const editor::ParserGroup& pg, const util::SoundMaterialParser &materialParser)=0;
	virtual void prepareForLaunch(IStorm3D* s3d, IStorm3D_Scene* scene)=0;
	virtual int  getNumParticles()=0;
	virtual void setPosition(const Vector& pos)=0;
	virtual void setVelocity(const Vector& vel)=0;
	virtual void setExplosion(const Vector& pos, bool enable)=0;
	virtual void setRotation(const Vector& rot)=0;
	virtual void setLenght(float lenght)=0;

	virtual void setSpawnModel(IStorm3D_Model *model) = 0;
	virtual void setEmitFactor(float factor) = 0;
	virtual void setLighting(const COL &ambient, const signed short int *lightIndices)=0;
	virtual void setCollision(boost::shared_ptr<IParticleCollision> &collision) = 0;
	virtual void setArea(boost::shared_ptr<IParticleArea> &area) = 0;
	virtual void setPhysics(boost::shared_ptr<ParticlePhysics> &physics) {}
	virtual void setEmitterRotation(const QUAT &rotation) = 0;
	virtual void releasePhysicsResources() = 0;
//	virtual int getTypeID()=0;
};


class GenParticleSystemEditables {
public:
	enum DIRECTION_TYPE
	{
		DIRECTION_DEFAULT,
		DIRECTION_VELOCITY,
		DIRECTION_NEGATIVE_VELOCITY,
		DIRECTION_EXPLOSION,
		DIRECTION_NEGATIVE_EXPLOSION
	};
	enum ANIMATION_TYPE
	{
		ANIMATION_PARTICLE_LIFE_TIME,
		ANIMATION_LOOP
	};
	enum ELEMENT_TYPE
	{
		PARTICLE_POINT,
		PARTICLE_QUAD,
		PARTICLE_LINE1,
		PARTICLE_LINE2
	};
	enum PHYSICS_TYPE
	{
		PHYSICS_TYPE_NONE,
		PHYSICS_TYPE_PARTICLE,
		PHYSICS_TYPE_FLUID,
		PHYSICS_TYPE_FLUID_INTERACTION
	};

	GenParticleSystemEditables()
	:	darkening(0), physicsType(PHYSICS_TYPE_NONE)
	{
		fluidStaticRestitution = 0.5f;
		fluidStaticAdhesion = 0.05f;
		fluidDynamicRestitution = 0.5f;
		fluidDynamicAdhesion = 0.5f;
		fluidDamping = 0.0f;
		fluidMotionLimit = 0.5f;
		fluidPacketSizeMultiplier = 8;
		fluidMaxEmitterAmount = 40;
		fluidDetailCollisionGroup = 0;

		fluidStiffness = 200.f;
		fluidViscosity = 22.f;
		fluidKernelRadiusMultiplier = 2.3f;
		fluidRestParticlesPerMeter = 10.f;
		fluidRestDensity = 1000.f;
	}

	virtual ~GenParticleSystemEditables() {}

	float emitRate;
	float emitStartTime;
	float emitStopTime;
	float lineLenght;
	int maxParticles;
	bool dieAfterEmission;
	float velocityInheritanceFactor;	
	Vector emitterPosition;
	Vector emitterVariation;
	float launchSpeed;
	float launchSpeedVar;
	Vector defaultLaunchDirection;
	DIRECTION_TYPE launchDirectionType;
	VectorTrack particleColor;
	FloatTrack particleAlpha;
	//FloatTrack particleSize;
	VectorTrack particleSize;
	float particleLife;
	float particleLifeVar;
	float particleStartAngle;
	float particleStartAngleVar;
	float particleSpin;
	float particleSpinVar;
	std::string particleTexture;
	int particleTextureUSubDivs;
	int particleTextureVSubDivs;
	int particleTextureAlphaType;
	int particleAnimationFrameCount;
	float particleAnimationStartFrame;
	float particleAnimationStartFrameVar;
	ANIMATION_TYPE particleAnimationType;
	float particleAnimationFps;
	ELEMENT_TYPE particleType;
	bool useLenghtControl;
	float darkening;
	bool distortion;
	bool faceUpward;
	bool outsideOnly;
	bool obstacleHeight;
	bool heightmapHeight;
	bool outsideFade;

	PHYSICS_TYPE physicsType;
	float fluidStaticRestitution;
	float fluidStaticAdhesion;
	float fluidDynamicRestitution;
	float fluidDynamicAdhesion;
	float fluidDamping;
	float fluidMotionLimit;
	int fluidPacketSizeMultiplier;
	int fluidMaxEmitterAmount;
	int fluidDetailCollisionGroup;

	float fluidStiffness;
	float fluidViscosity;
	float fluidKernelRadiusMultiplier;
	float fluidRestParticlesPerMeter;
	float fluidRestDensity;
};

// simple force interface for GenParticleSystems and classes that inherit it
class IParticleForce {
public:
	virtual ~IParticleForce() {}
	virtual void parseFrom(const editor::ParserGroup& pg)=0;
	virtual void preCalc(float t)=0;
	virtual void calcForce(Vector& force, const Vector& pos, const Vector& vel)=0;
	virtual int getTypeId() const = 0;
};


class IParticleRenderer {
public:
	virtual ~IParticleRenderer() {}
	virtual boost::shared_ptr<IParticleRenderer> clone()=0;
	virtual void prepareForLaunch(int maxParts)=0;
	virtual void render(IStorm3D_Scene* scene, IStorm3D_Material* mtl,
		GenParticleSystemEditables& eds, std::vector<Particle>& parts, const COL &factor, bool distortion, bool faceUpward)=0;
};

class PointParticleRenderer : public IParticleRenderer {
	std::vector<Storm3D_PointParticle> m_points;
public:
	boost::shared_ptr<IParticleRenderer> clone();
	void prepareForLaunch(int maxParts);
	void render(IStorm3D_Scene* scene, IStorm3D_Material* mtl,
		GenParticleSystemEditables& eds, std::vector<Particle>& parts, const COL &factor, bool distortion, bool faceUpward);
};


class QuadParticleRenderer : public IParticleRenderer {
	std::vector<Storm3D_PointParticle> m_points;
	Storm3D_ParticleTextureAnimationInfo m_animInfo;
public:
	boost::shared_ptr<IParticleRenderer> clone();
	void prepareForLaunch(int maxParts);
	void render(IStorm3D_Scene* scene, IStorm3D_Material* mtl,
		GenParticleSystemEditables& eds, std::vector<Particle>& parts, const COL &factor, bool distortion, bool faceUpward);
};

class LineParticleRenderer1 : public IParticleRenderer {
	std::vector<Storm3D_LineParticle> m_lines;
	Storm3D_ParticleTextureAnimationInfo m_animInfo;
public:
	boost::shared_ptr<IParticleRenderer> clone();
	void prepareForLaunch(int maxParts);
	void render(IStorm3D_Scene* scene, IStorm3D_Material* mtl,
		GenParticleSystemEditables& eds, std::vector<Particle>& parts, const COL &factor, bool distortion, bool faceUpward);
};

class LineParticleRenderer2 : public IParticleRenderer {
	std::vector<Storm3D_LineParticle> m_lines;
	Storm3D_ParticleTextureAnimationInfo m_animInfo;
public:
	boost::shared_ptr<IParticleRenderer> clone();
	void prepareForLaunch(int maxParts);
	void render(IStorm3D_Scene* scene, IStorm3D_Material* mtl,
		GenParticleSystemEditables& eds, std::vector<Particle>& parts, const COL &factor, bool distortion, bool faceUpward);
};


class GenParticleSystem : public IParticleSystem {
protected:

	std::vector<Particle> m_parts;	
	std::vector< boost::shared_ptr<IParticleForce> > m_forces;
//	std::vector<Storm3D_PointParticle> m_mesh;
//	std::vector<Storm3D_LineParticle> m_lineMesh;

//	boost::shared_ptr<ParamBlock> m_pb;
//	Matrix m_tm;
	Vector m_position;
	Vector m_velocity;
	Vector m_target;
	MAT m_rotation;
	QUAT m_rotation_quat;
	bool m_trackTarget;
	bool m_alive;
	bool m_shutdown;
	float m_lenght;
	float m_time;
	float m_particleResidue;
	int m_maxParticles;
	int m_numParts;
	int lastParticleIndex;
	VC3 explosion_position;
	bool use_explosion;
	COL ambient;
	IStorm3D_Material* m_mtl;
	boost::shared_ptr<IParticleRenderer> m_renderer;
	boost::shared_ptr<IParticleArea> area;
	float emit_factor;

	boost::shared_ptr<std::vector<Particle> > m_render_fluid_parts;	
	boost::shared_ptr<PhysicsFluid> fluid;
	boost::shared_ptr<ParticlePhysics> physics;

	IStorm3D_Model *spawnModel;
	std::vector<IStorm3D_Helper *> spawnHelpers;

	void copyTo(GenParticleSystem& other);
	void defaultInit(IStorm3D* s3d, IStorm3D_Scene* scene, const GenParticleSystemEditables& eds);
	void defaultPrepareForLaunch(IStorm3D* s3d, IStorm3D_Scene* scene, const GenParticleSystemEditables& eds);
	void defaultTick(IStorm3D_Scene* scene, const GenParticleSystemEditables& eds);
	void defaultRender(IStorm3D_Scene* scene, GenParticleSystemEditables& eds);
	bool moveAndAnimateSystem(const GenParticleSystemEditables& eds);
	void emitParticles(const GenParticleSystemEditables& eds);
	void moveAndExpireParticles(const GenParticleSystemEditables& eds);
	void applyForces(const GenParticleSystemEditables& eds);
	void defaultParseFrom(const editor::ParserGroup& pg, GenParticleSystemEditables& eds);
	void setDistortion(bool distortion);

public:

	GenParticleSystem();

	IParticleForce* addForce(const std::string& className);
	int getNumForces() const;
	IParticleForce* getForce(int i);
	void removeForce(int i);
	
	virtual void setParticlePosition(Vector& pos)=0;
	virtual void setParticleVelocity(Vector& vel, const Vector& direction, float speed, const GenParticleSystemEditables& eds)=0;
	virtual void *getId() const = 0;

	bool isDead();
	void kill();
	void setPosition(const Vector& pos);
	void setVelocity(const Vector& vel);
	void setRotation(const Vector& rot);
	void setLenght(float lenght);
	int getNumParticles();

	void setSpawnModel(IStorm3D_Model *model);
	void setEmitFactor(float factor);
	void setLighting(const COL &ambient, const signed short int *lightIndices);
	void setExplosion(const Vector& pos, bool enable);
	void setArea(boost::shared_ptr<IParticleArea> &area);
	void setPhysics(boost::shared_ptr<ParticlePhysics> &physics);
	void releasePhysicsResources();
};


} // particle
 
} // frozenbyte

#endif
