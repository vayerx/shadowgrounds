#ifndef INCLUDED_FROZENBYTE_FLUID_H
#define INCLUDED_FROZENBYTE_FLUID_H

#include <DatatypeDef.h>
#include <vector>

class NxFluid;
class NxScene;
class NxFluidDesc;

#ifndef NX_DISABLE_FLUIDS
namespace frozenbyte {
namespace physics {

int getFluidBaseCount();
int getFluidParticleCount();
void resetFluidParticleCount();

struct FluidParticle
{
	VC3 position;
	VC3 velocity;
	float life;
	float density;
	unsigned int id;

	FluidParticle()
	:	life(0),
		density(0),
		id(0)
	{
	}
};

class Fluid
{
	NxFluid *fluid;
	NxScene &scene;

	std::vector<FluidParticle> particles;
	unsigned int activeParticles;

	std::vector<FluidParticle> bufferedParticles;
	int bufferedAmount;
	int addedParticles;

public:
	Fluid(NxScene &scene, const NxFluidDesc &desc, bool useHardware);
	~Fluid();

	bool isValid() const;
	const FluidParticle *getParticles(int &amount) const;

	void update();

	// Buffer contains elements in { position(VC3), velocity(VC3) } format
	void addParticles(void *buffer, int amount);
	void setAcceleration(const VC3 &force);
};

} // physics
} // frozenbyte

#endif

#endif
