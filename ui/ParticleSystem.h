/** @file ParticleSystem.h


	*/

#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

// For Vector
#include "DatatypeDef.h"


// Forward Declarations 
struct Storm3D_PointParticle;
struct Storm3D_LineParticle;
class IStorm3D_ParticleSystem;
class IStorm3D_Material;
class IStorm3D;

namespace ui {

/** ParticleSystem handles one particlesystem instance, such as an explosion etc.
		For now everything is hard-coded. Eventually this should evolve to flexible
		system that reads math from a file and theres a nice editor for it etc.
  */
class ParticleSystem {
public:


	typedef enum {
		EXPLOSION,
    EXPLOSION2,
    EXPLOSION3,
		SMOKE,
    RISINGSMOKE,
		DUST,
    ROCKETTAIL,
		SPARK,
		SPARK2,
		LASER,
		SWARM,
    FLAME,
    GLOWFLARE
	} ParticleSystemType;


	/** Constructor for ParticleSystem.
	    The parameters for this are type dependant. Kinda ugly yes.
			@param type The type of particle system.
		*/
	ParticleSystem(IStorm3D_ParticleSystem *particleSystem, ParticleSystemType type, Vector vector1, Vector vector2, float lifeTime );

	/** Deconstructor for the ParticleSystem.
		*/
	~ParticleSystem();
  
	/** solver is a integrator for the particle system.
	*/
	void solver(const Vector &velocity, const Vector &forces, Vector *result, float mass, float step);

	/** stepForwards steps forward the physics.
	    @param tick number of ticks since last call (should be always one)
			@return returns true if system is done and can be deleted.
	*/
	bool stepForwards(int tick);

	/** Renders particles.
	    Renders all the particles in the system.
		*/
	bool render();


	/* NOTE: We assume that ParticleManager calls this once.
		 Assumptions are always bad. Don't ASSUME or you'll make
		 an ASS out of U and ME .. hehe */
	static void setMaterials(IStorm3D *s3d);



private:

	
	ParticleSystemType   type_;
	Vector vector1_;
	Vector vector2_;
	float  lifeTime_;

	/* for explosion types. Not yet finalized .. */
	float mass_;
	float heat_;

	float  time_;

  /* Arrays of particles. Yes arrays are evil, but thats
	   the way storm wants it. So be it. We have arrays for both
		 type of particles, even if we don't need the other one,
		 because this same particlesystem is used for all particle
		 system types. */
	Storm3D_PointParticle *pointParticles_;
	Storm3D_LineParticle  *lineParticles_;

	/* Uh. A hack. particles store everything I need except velocities,
	   so I made an another array that does that. Not really elegant. */ 
	Vector *velocities_;

	/* The number of particles */
	int particleNo_;

	/* Storm particlesystem stuff, this array is the one that gets
	   given to storm when we render */
	IStorm3D_ParticleSystem *particleSystem_;

	/* Returns a value between [0,1]. This should really be 
		 in util or something, but for now this will suffice */
	float random();

	/* Static materials so that there won't be many instances of
		 the same material.. */
	static IStorm3D_Material *explosionMaterial;
	static IStorm3D_Material *sparkMaterial;
	static IStorm3D_Material *smokeMaterial;
	static IStorm3D_Material *dustMaterial;
	static IStorm3D_Material *laserMaterial;
	static IStorm3D_Material *swarmMaterial;
	static IStorm3D_Material *flameMaterial;
	static IStorm3D_Material *flareMaterial;
};

} // namespace ui

#endif /* PARTICLESYSTEM_H */
/* EOF */
