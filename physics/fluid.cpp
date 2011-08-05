
#include "precompiled.h"

#include "fluid.h"
#include "NxPhysics.h"

#ifndef NX_DISABLE_FLUIDS
namespace frozenbyte {
namespace physics {
namespace {

	int fluidCount = 0;
	int fluidParticleCount = 0;

} // unnamed

int getFluidBaseCount()
{
	return fluidCount;
}

int getFluidParticleCount()
{
	return fluidParticleCount;
}

void resetFluidParticleCount()
{
	fluidParticleCount = 0;
}

Fluid::Fluid(NxScene &scene_, const NxFluidDesc &desc, bool useHardware)
:	fluid(0),
	scene(scene_),
	activeParticles(0),
	addedParticles(0)
{
	++fluidCount;

	particles.resize(desc.maxParticles);
	bufferedParticles.resize(desc.maxParticles);

	NxParticleData data;
	int stride = sizeof(FluidParticle);
	//data.maxParticles = desc.maxParticles;
	data.numParticlesPtr = &activeParticles;
	data.bufferPos = &(particles[0].position.x);
	data.bufferPosByteStride = stride;
	data.bufferVel = &(particles[0].velocity.x);
	data.bufferVelByteStride = stride;
	data.bufferLife = &(particles[0].life);
	data.bufferLifeByteStride = stride;
	data.bufferDensity = &(particles[0].density);
	data.bufferDensityByteStride = stride;
	data.bufferId = &(particles[0].id);
	data.bufferIdByteStride = stride;

	NxFluidDesc finalDesc = desc;
	finalDesc.particlesWriteData = data;

	finalDesc.flags = NX_FF_ENABLED | NX_FF_VISUALIZATION | NX_FF_DISABLE_GRAVITY;
	if(useHardware)
		finalDesc.flags |= NX_FF_HARDWARE;

	fluid = scene.createFluid(finalDesc);
}

Fluid::~Fluid()
{
	--fluidCount;

	if(fluid)
		scene.releaseFluid(*fluid);
}

bool Fluid::isValid() const
{
	if(fluid && !particles.empty())
		return true;

	return false;
}

const FluidParticle *Fluid::getParticles(int &amount) const
{
	//amount = activeParticles;
	//return &particles[0];

	amount = bufferedAmount;
	return &bufferedParticles[0];
}

void Fluid::update()
{
	//bufferedAmount = activeParticles;
	//for(int i = 0; i < bufferedAmount; ++i)
	//	bufferedParticles[i] = particles[i];

	bufferedAmount = 0;
	for(unsigned int i = 0; i < activeParticles; ++i)
	{
		if(particles[i].life >= 0)
		{
			bufferedParticles[i] = particles[i];
			++bufferedAmount;
		}
	}

	fluidParticleCount += bufferedAmount;
	addedParticles = 0;
}

void Fluid::addParticles(void *buffer, int amount)
{
	if(activeParticles + amount + addedParticles > particles.size())
		amount = particles.size() - activeParticles - addedParticles;
	if(amount + addedParticles > 4000)
		amount = 4000 - addedParticles;
	if(amount <= 0)
		return;

	unsigned int uamount = amount;

	NxParticleData data;
	int stride = sizeof(VC3)*2 + 2 * sizeof(float) + sizeof(unsigned int);
	//data.maxParticles = amount;
	data.numParticlesPtr = &uamount;
	data.bufferPos = static_cast<float *> (buffer);
	data.bufferPosByteStride = stride;
	data.bufferVel = static_cast<float *> (buffer) + (sizeof(VC3) / sizeof(float));
	data.bufferVelByteStride = stride;
	data.bufferLife = static_cast<float *> (buffer) + (2 * sizeof(VC3) / sizeof(float));
	data.bufferLifeByteStride = stride;
	data.bufferDensity = static_cast<float *> (buffer) + (3 * sizeof(VC3) / sizeof(float));
	data.bufferDensityByteStride = stride;

	addedParticles += amount;
	fluid->addParticles(data);
}

void Fluid::setAcceleration(const VC3 &force)
{
	NxVec3 acceleration(force.x, force.y, force.z);
	fluid->setExternalAcceleration(acceleration);
}

} // physics
} // frozenbyte
#endif
