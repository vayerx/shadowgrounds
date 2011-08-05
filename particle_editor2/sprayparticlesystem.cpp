#include <boost/lexical_cast.hpp>

#include "precompiled.h"

// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning( disable : 4800 )
#endif

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>
#include <string>
#include <map>
#include <list>
#include <fstream>
#include <Storm3D_UI.h>
#include "../editor/string_conversions.h"
#include "../editor/parser.h"
#include "track.h"
//#include "paramblock.h"
#include "parseutil.h"
#include "particlesystem.h"
#include "particleeffect.h"
#include "sprayparticlesystem.h"
#ifdef PHYSICS_PHYSX
#include "ParticlePhysics.h"
#include "../game/physics/physics_collisiongroups.h"
#endif

using namespace frozenbyte::editor;

namespace frozenbyte
{
namespace particle
{

namespace {
	int SPSid = 0;
}


SprayParticleSystem::SprayParticleSystem() 
{
}

boost::shared_ptr<IParticleSystem> SprayParticleSystem::createNew() {
	SprayParticleSystem* ps = new SprayParticleSystem();
	boost::shared_ptr<IParticleSystem> ptr(ps); 
	boost::shared_ptr<SprayParticleSystemEditables> eds(new SprayParticleSystemEditables);
	ps->m_eds.swap(eds);
	return ptr;
}

boost::shared_ptr<IParticleSystem> SprayParticleSystem::clone() {
	SprayParticleSystem* ps = new SprayParticleSystem();
	copyTo(*ps);
	ps->m_eds = m_eds;
	boost::shared_ptr<IParticleSystem> res(ps);

#ifdef PHYSICS_PHYSX
#ifndef NX_DISABLE_FLUIDS

	if(!fluid && physics && (m_eds->physicsType == GenParticleSystemEditables::PHYSICS_TYPE_FLUID || m_eds->physicsType == GenParticleSystemEditables::PHYSICS_TYPE_FLUID_INTERACTION))
	{
		int collGroup = PHYSICS_COLLISIONGROUP_FLUIDS;
		if (m_eds->fluidDetailCollisionGroup)
		{
			collGroup = PHYSICS_COLLISIONGROUP_FLUIDS_DETAILED;
		}
		fluid = physics->createFluid(m_eds->physicsType - 2, m_eds->maxParticles * m_eds->fluidMaxEmitterAmount, m_eds->fluidStaticRestitution, m_eds->fluidStaticRestitution, m_eds->fluidDynamicRestitution, m_eds->fluidDynamicAdhesion, m_eds->fluidDamping, m_eds->fluidStiffness, m_eds->fluidViscosity, m_eds->fluidKernelRadiusMultiplier, m_eds->fluidRestParticlesPerMeter, m_eds->fluidRestDensity, m_eds->fluidMotionLimit, m_eds->fluidPacketSizeMultiplier, collGroup);
		if(fluid)
			m_render_fluid_parts.reset(new std::vector<Particle> (m_eds->maxParticles * m_eds->fluidMaxEmitterAmount));
	}

	ps->fluid = fluid;
	ps->m_render_fluid_parts = m_render_fluid_parts;

#endif
#endif

	return res;
}


SprayParticleSystemEditables& SprayParticleSystem::getEditables() {
	return *m_eds;
}
	
const SprayParticleSystemEditables& SprayParticleSystem::getEditables() const {
	return *m_eds;
}

void SprayParticleSystem::init(IStorm3D* s3d, IStorm3D_Scene* scene) {
	GenParticleSystem::defaultInit(s3d, scene, *m_eds);
}

void SprayParticleSystem::prepareForLaunch(IStorm3D* s3d, IStorm3D_Scene* scene) {
	GenParticleSystem::defaultPrepareForLaunch(s3d, scene, *m_eds);
}

void SprayParticleSystem::tick(IStorm3D_Scene* scene) {
	GenParticleSystem::defaultTick(scene, *m_eds);
}

void SprayParticleSystem::render(IStorm3D_Scene* scene) {
	GenParticleSystem::defaultRender(scene, *m_eds);
}


void SprayParticleSystem::parseFrom(const ParserGroup& pg, const util::SoundMaterialParser &materialParser) {
	defaultParseFrom(pg, *m_eds);
	m_eds->spread1 = convertFromString<float>(pg.getValue("spread1", "0"), m_eds->spread1);
	m_eds->spread2 = convertFromString<float>(pg.getValue("spread2", "0"), m_eds->spread2);
}

void SprayParticleSystem::setParticlePosition(Vector& v) {
	v = Vector(0.0f, 0.0f, 0.0f);

	if(spawnModel)
	{
		if(!spawnHelpers.empty())
		{
			int index = rand() % spawnHelpers.size();
			v = spawnHelpers[index]->GetGlobalPosition();
		}
		else
			v = spawnModel->GetPosition();
	}
}

void SprayParticleSystem::setParticleVelocity(Vector& v, const Vector& direction, float speed, const GenParticleSystemEditables& eds) {
	
	float spread1 = m_eds->spread1;
	float spread2 = m_eds->spread2;
	
	Vector dir = direction;
	dir.Normalize();
	
	Vector up(0.0f, 0.0f, 1.0f);
	Vector u = dir.GetCrossWith(up);
	if(u.GetDotWith(u)<0.0001f) {
		up = Vector(0.0f, 1.0f, 0.0f);	
	}
	
	Vector left;
	left = dir.GetCrossWith(up);
	left.Normalize();
	up = left.GetCrossWith(dir);
	up.Normalize();
	
	int seed1 = rand() % RAND_MAX;
	int seed2 = rand() % RAND_MAX;
	
	float rnd1 = -1.0f + (float)seed1 / (float)RAND_MAX * 2.0f;
	float rnd2 = -1.0f + (float)seed2 / (float)RAND_MAX * 2.0f;
	
	float a1 = rnd1 * spread1;
	float a2 = rnd2 * spread2;
	
	QUAT q1;
	QUAT q2;
	
	q1.MakeFromAxisRotation(left, a1);
	q2.MakeFromAxisRotation(up, a2);
	
	q1.RotateVector(dir);
	q2.RotateVector(dir);
	
	v = dir * speed;
}

void SprayParticleSystem::setCollision(boost::shared_ptr<IParticleCollision> &collision_)
{
	collision = collision_;
}

void *SprayParticleSystem::getId() const
{
	return &SPSid;
}

void *SprayParticleSystem::getType()
{
	return &SPSid;
}


} // particle

} // frozenbyte
