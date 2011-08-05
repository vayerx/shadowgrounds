#ifndef POINT_ARRAY_PARTICLE_SYSTEM_H
#define POINT_ARRAY_PARTICLE_SYSTEM_H

namespace frozenbyte
{
namespace particle
{


enum POINT_ARRAY_PARTICLE_SYSTEM_PARAMS
{
	PB_POINT_ARRAY_MODEL_FILE = GEN_PARTICLE_SYSTEM_PARAM_COUNT,
	PB_POINT_ARRAY_FIRST_VERTEX,
	PB_POINT_ARRAY_LAST_VERTEX
};

ParticleSystemClassDesc* getPointArrayParticleSystemClassDesc();

} // particle
} // frozenbyte


#endif