#ifndef DRAG_PARTICLE_FORCE_H
#define DRAG_PARTICLE_FORCE_H

namespace frozenbyte
{
namespace particle
{

enum DRAG_PARTICLE_FORCE_PARAMS {
	PB_DRAG_FACTOR = GEN_PARTICLE_FORCE_PARAM_COUNT,
};

ParticleForceClassDesc* getDragParticleForceClassDesc();

} // particle
} // frozenbyte


#endif