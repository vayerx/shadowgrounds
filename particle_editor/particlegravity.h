#ifndef PARTICLE_GRAVITY_H
#define PARTICLE_GRAVITY_H


namespace frozenbyte
{
namespace particle
{

enum PARTICLE_GRAVITY_PARAMS
{
	PB_GRAVITY = GEN_PARTICLE_FORCE_PARAM_COUNT// float, gravity, default=0
};

ParticleForceClassDesc* getParticleGravityClassDesc();

} // particle
} // frozenbyte



#endif