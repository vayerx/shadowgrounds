#ifndef SPRAY_PARTICLE_SYSTEM_H
#define SPRAY_PARTICLE_SYSTEM_H

namespace frozenbyte
{
namespace particle
{


enum SPRAY_PARTICLE_SYSTEM_PARAMS
{
	PB_SPREAD1 = GEN_PARTICLE_SYSTEM_PARAM_COUNT,
	PB_SPREAD2
};

ParticleSystemClassDesc* getSprayParticleSystemClassDesc();

} // particle
} // frozenbyte


#endif