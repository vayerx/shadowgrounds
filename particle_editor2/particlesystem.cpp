
#include "precompiled.h"

// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning( disable : 4800 )
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Storm3D_UI.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>
#include <string>
#include <map>
#include <list>
#include "../editor/string_conversions.h"
#include "../editor/parser.h"
#include "track.h"
//#include "paramblock.h"
#include "parseutil.h"
#include "particletiming.h"
#include "../ui/IPointableObject.h"
#include "particlesystem.h"
#include "particlesystemmanager.h"
#include "particleforces.h"
#include "sprayparticlesystem.h"
#include "iparticlearea.h"
#include "pointarrayparticlesystem.h"

//#include "../physics/fluid.h"
//#include "../physics/physics_lib.h"
#include "ParticlePhysics.h"
#include "../game/GameRandom.h"

//#include "particlerandom.h"

namespace frozenbyte {
namespace particle {

using namespace frozenbyte::editor;

void PointParticleRenderer::prepareForLaunch(int maxParts) {

}
	
void PointParticleRenderer::render(IStorm3D_Scene* scene, IStorm3D_Material* mtl,
		GenParticleSystemEditables& eds, std::vector<Particle>& parts, const COL &factor, bool distortion, bool faceUpward) {
	
}

boost::shared_ptr<IParticleRenderer> PointParticleRenderer::clone() {
	PointParticleRenderer* p = new PointParticleRenderer();
	boost::shared_ptr<IParticleRenderer> ptr(p);
	return ptr;
}


boost::shared_ptr<IParticleRenderer> QuadParticleRenderer::clone() {
	QuadParticleRenderer* p = new QuadParticleRenderer();
	boost::shared_ptr<IParticleRenderer> ptr(p);
	return ptr;
}

void QuadParticleRenderer::prepareForLaunch(int maxParts) {
	m_points.resize(maxParts);
}
	
void QuadParticleRenderer::render(IStorm3D_Scene* scene, IStorm3D_Material* mtl,
			GenParticleSystemEditables& eds, std::vector<Particle>& parts, const COL &factor, bool distortion, bool faceUpward) {

	m_animInfo.textureUSubDivs = eds.particleTextureUSubDivs;
	m_animInfo.textureVSubDivs = eds.particleTextureVSubDivs;
	m_animInfo.numFrames = eds.particleAnimationFrameCount;

	IStorm3D_Camera *camera = scene->GetCamera();

	int a = 0;
	for(int i = 0; i < (int)parts.size(); i++) 
	{
		Particle &p = parts[i];
		if(p.alive) 
		{
			VC3 size;
			float alpha = 0;
			Vector c;
			float t = p.age / p.life;
			eds.particleSize.getValue(&size, t);
			if(!camera->TestSphereVisibility(p.position, std::max(size.x, size.y)))
				continue;

			eds.particleColor.getValue(&c, t);
			eds.particleAlpha.getValue(&alpha, t);

			if(alpha > 0.001f)
			{
				Storm3D_PointParticle &shape = m_points[a++];
				shape.color = COL(c.x, c.y, c.z);
				shape.position = p.position;
				shape.size = size.x;
				shape.alpha = alpha;
				shape.alpha -= p.forceAlpha;
				shape.alpha = std::max(0.f, shape.alpha);
				shape.angle = p.angle;
				shape.frame = p.frame;
			}
		}
	}

	scene->GetParticleSystem()->renderQuads(mtl, &m_points[0], a, &m_animInfo, factor, distortion, faceUpward);
}

boost::shared_ptr<IParticleRenderer> LineParticleRenderer1::clone() {
	LineParticleRenderer1* p = new LineParticleRenderer1();
	boost::shared_ptr<IParticleRenderer> ptr(p);
	return ptr;
}


void LineParticleRenderer1::prepareForLaunch(int maxParts) {
	m_lines.resize(maxParts);
}
	
void LineParticleRenderer1::render(IStorm3D_Scene* scene, IStorm3D_Material* mtl,
			GenParticleSystemEditables& eds, std::vector<Particle>& parts, const COL &factor, bool distortion, bool faceUpward) 
{
	m_animInfo.textureUSubDivs = eds.particleTextureUSubDivs;
	m_animInfo.textureVSubDivs = eds.particleTextureVSubDivs;
	m_animInfo.numFrames = eds.particleAnimationFrameCount;

	IStorm3D_Camera *camera = scene->GetCamera();

	int a = 0;
	for(int i = 0; i < (int)parts.size(); i++) 
	{
		Particle& p = parts[i];
		if(p.alive) 
		{
			VC3 size;
			float alpha = 0;
			Vector c;
			float t = p.age / p.life;
			eds.particleSize.getValue(&size, t);

float radius = std::max(std::max(size.x, fabsf(size.y)), p.originSize);
float sqRange = p.origin.GetSquareRangeTo(p.position);
if(sqRange > radius * radius)
	radius = sqrtf(sqRange);
if(!camera->TestSphereVisibility(p.position, radius))
	continue;

			eds.particleColor.getValue(&c, t);
			eds.particleAlpha.getValue(&alpha, t);

			Storm3D_LineParticle& shape = m_lines[a++];

			shape.color[0] = p.originColor;
			shape.position[0] = p.origin;
			shape.size[0] = p.originSize;
			
			// psd: cant fade whole line using alpha if it always uses 1.0 for start point
			//shape.alpha[0] = p.originAlpha;
			shape.alpha[0] = p.previousAlpha;
			p.previousAlpha = alpha;

			shape.color[1] = COL(c.x, c.y, c.z);
			shape.position[1] = p.position;
			shape.size[1] = size.x;
			shape.alpha[1] = alpha;

			shape.alpha[0] -= p.forceAlpha;
			shape.alpha[0] = std::max(0.f, shape.alpha[0]);
			shape.alpha[1] -= p.forceAlpha;
			shape.alpha[1] = std::max(0.f, shape.alpha[1]);
		}
	}

	scene->GetParticleSystem()->renderLines(mtl, &m_lines[0], a, &m_animInfo, factor, distortion);
}


boost::shared_ptr<IParticleRenderer> LineParticleRenderer2::clone() {
	LineParticleRenderer2* p = new LineParticleRenderer2();
	boost::shared_ptr<IParticleRenderer> ptr(p);
	return ptr;
}

void LineParticleRenderer2::prepareForLaunch(int maxParts) {
	m_lines.resize(maxParts);
}
	
void LineParticleRenderer2::render(IStorm3D_Scene* scene, IStorm3D_Material* mtl,
			GenParticleSystemEditables& eds, std::vector<Particle>& parts, const COL &factor, bool distortion, bool faceUpward) 
{
	m_animInfo.textureUSubDivs = eds.particleTextureUSubDivs;
	m_animInfo.textureVSubDivs = eds.particleTextureVSubDivs;
	m_animInfo.numFrames = eds.particleAnimationFrameCount;

	IStorm3D_Camera *camera = scene->GetCamera();

	int a = 0;
	for(int i = 0; i < (int)parts.size(); i++) {
		Particle& p = parts[i];
		if(p.alive) {
			//float size;
			VC3 size;
			float alpha = 0;
			Vector c;
			float t = p.age / p.life;
			
			eds.particleSize.getValue(&size, t);
float radius = std::max(std::max(size.x, fabsf(size.y)), p.previousSize);
if(!camera->TestSphereVisibility(p.position, radius))
	continue;

			eds.particleColor.getValue(&c, t);
			eds.particleAlpha.getValue(&alpha, t);

			Storm3D_LineParticle& shape = m_lines[a++];			
			shape.color[0] = p.previousColor;
			Vector v = p.velocity;
			float speed = v.GetLength();
			if(speed > 0.0001f)
				v.Normalize();

			shape.position[0] = p.position;
			//shape.position[0] = p.position + v * len;
			shape.size[0] = p.previousSize;
			shape.alpha[0] = p.previousAlpha;

			shape.alpha[0] -= p.forceAlpha;
			shape.alpha[0] = std::max(0.f, shape.alpha[0]);

			shape.color[1] = COL(c.x, c.y, c.z);
			//shape.position[1] = p.position;
			shape.position[1] = p.position + (v * size.y) + (v * speed * size.z);
			shape.size[1] = size.x;
			shape.alpha[1] = alpha;

			shape.alpha[1] -= p.forceAlpha;
			shape.alpha[1] = std::max(0.f, shape.alpha[1]);
	
			p.previousColor = shape.color[1];
			p.previousSize = size.x;
			p.previousAlpha = alpha;

		}
	}

	scene->GetParticleSystem()->renderLines(mtl, &m_lines[0], a, &m_animInfo, factor, distortion);
}


GenParticleSystem::GenParticleSystem()
{
	m_trackTarget = false;
	m_alive = true;
	m_shutdown = false;
	m_lenght = 0;
	m_time = 0;
	m_particleResidue = 0;
	m_maxParticles = 0;
	m_numParts = 0;
	m_mtl = 0;
	lastParticleIndex = 0;
	use_explosion = false;
	emit_factor = 1.f;
	spawnModel = 0;
}

IParticleForce* GenParticleSystem::addForce(const std::string& className) {
	if(className == "drag") {
		boost::shared_ptr<IParticleForce> f(new DragParticleForce);
		m_forces.push_back(f);
		return f.get();
	}
	if(className == "gravity") {
		boost::shared_ptr<IParticleForce> f(new GravityParticleForce);
		m_forces.push_back(f);
		return f.get();
	}
	if(className == "sidegravity") {
		boost::shared_ptr<IParticleForce> f(new SideGravityParticleForce);
		m_forces.push_back(f);
		return f.get();
	}
	if(className == "wind") {
		boost::shared_ptr<IParticleForce> f(new WindParticleForce);
		m_forces.push_back(f);
		return f.get();
	}
	assert(!"tried to add unkown force type to particle system");
	return NULL;
}

int GenParticleSystem::getNumForces() const {
	return m_forces.size();
}
	
IParticleForce* GenParticleSystem::getForce(int i) {
	return m_forces[i].get();
}

void GenParticleSystem::removeForce(int i) {
	assert(i >= 0 && i < (int)m_forces.size());
	m_forces.erase(m_forces.begin() + i);
}


void GenParticleSystem::setVelocity(const Vector& vel) {
	m_velocity = vel;
}

void GenParticleSystem::setPosition(const Vector& pos) {
	if (this->spawnModel == NULL)
	{
		m_position = pos;
	//} else {
		// ???
	}
}

void GenParticleSystem::setRotation(const Vector& rot)
{
	if (this->spawnModel == NULL)
	{
		m_rotation_quat.MakeFromAngles(rot.x / 180.f * PI, rot.y / 180.f * PI, rot.z / 180.f * PI);
		//q.MakeFromAngles(0, rot.y, 0);

		m_rotation.CreateRotationMatrix(m_rotation_quat);
	//} else {
		// ???
	}
}

void GenParticleSystem::setLenght(float lenght) {
	m_lenght = lenght;
}

/*
void GenParticleSystem::setTM(const Matrix& tm) {
	m_tm = tm;
}
*/
int GenParticleSystem::getNumParticles() {
	return m_numParts;
}

void GenParticleSystem::copyTo(GenParticleSystem& other) {
	for(int i = 0; i < (int)m_forces.size(); i++) {
		other.m_forces.push_back(m_forces[i]);
	}
	other.m_mtl = m_mtl;
	other.m_renderer = m_renderer->clone();
}


void GenParticleSystem::defaultPrepareForLaunch(IStorm3D* s3d, IStorm3D_Scene* scene, const GenParticleSystemEditables& eds) 
{
	int reserveAmount = eds.maxParticles;
	if(eds.physicsType == GenParticleSystemEditables::PHYSICS_TYPE_FLUID || eds.physicsType == GenParticleSystemEditables::PHYSICS_TYPE_FLUID_INTERACTION)
		reserveAmount *= FLUID_PARTICLE_FACTOR;
	
	m_renderer->prepareForLaunch(reserveAmount);
	m_maxParticles = eds.maxParticles;
	
	m_time = 0;
	m_particleResidue = 0;
	m_numParts = 0;
	m_alive = true;
	m_shutdown = false;

	m_parts.resize(m_maxParticles);
	for(int i = 0; i < (int)m_parts.size(); i++)
		m_parts[i].alive = false;

#ifdef PHYSICS_PHYSX
	/*
	if(physics && (eds.physicsType == GenParticleSystemEditables::PHYSICS_TYPE_FLUID || eds.physicsType == GenParticleSystemEditables::PHYSICS_TYPE_FLUID_INTERACTION))
	{
		fluid = physics->createFluid(eds.physicsType - 2, eds.maxParticles, eds.fluidStaticRestitution, eds.fluidStaticRestitution, eds.fluidDynamicRestitution, eds.fluidDynamicAdhesion, eds.fluidDamping, eds.fluidStiffness, eds.fluidViscosity, eds.fluidKernelRadiusMultiplier, eds.fluidRestParticlesPerMeter, eds.fluidRestDensity);
	}
	*/
#endif
}


void GenParticleSystem::defaultInit(IStorm3D* s3d,
									IStorm3D_Scene* scene, const GenParticleSystemEditables& eds) {

	m_mtl = NULL;
	if(eds.particleTexture.empty())
		return;

	std::string mtlName = "particle" + eds.particleTexture;
	m_mtl = s3d->CreateNewMaterial(mtlName.c_str());
	IStorm3D_Texture* tex = s3d->CreateNewTexture(eds.particleTexture.c_str());
	m_mtl->SetBaseTexture(tex);
}

void GenParticleSystem::defaultTick(IStorm3D_Scene* scene, const GenParticleSystemEditables& eds) 
{
	if(!moveAndAnimateSystem(eds))
		return;
	
	// move and expire particles
	moveAndExpireParticles(eds);

	if(!m_shutdown)
		emitParticles(eds);
		
	applyForces(eds);

}


void GenParticleSystem::defaultRender(IStorm3D_Scene* scene, GenParticleSystemEditables& eds) 
{
	bool usingFluids = false;
	if(eds.physicsType == GenParticleSystemEditables::PHYSICS_TYPE_FLUID || eds.physicsType == GenParticleSystemEditables::PHYSICS_TYPE_FLUID_INTERACTION)
		usingFluids = true;
	
	//if(!m_alive && !usingFluids)
	//	return;
	if(!m_alive)
		return;

	if(m_mtl)
		m_mtl->SetAlphaType((IStorm3D_Material::ATYPE)eds.particleTextureAlphaType);
	
	COL darkness(1.f, 1.f, 1.f);
	darkness -= ambient;

	COL factor(1.f, 1.f, 1.f);
	factor -= darkness * eds.darkening;

#ifdef PHYSICS_PHYSX
#ifndef NX_DISABLE_FLUIDS
	if(usingFluids)
	{
		/*
		if(fluid)
		{
			int particleAmount = 0;
			const PhysicsFluid::Particle *buffer = fluid->getParticles(particleAmount);

			float frameSpeed = 0.f;
			if(eds.particleAnimationType == GenParticleSystemEditables::ANIMATION_LOOP)
				frameSpeed = eds.particleAnimationFps / PARTICLE_TIME_SCALE;
			else
				frameSpeed = ((float)eds.particleAnimationFrameCount - eds.particleAnimationStartFrame) / eds.particleLife;

			for(int i = 0; i < m_maxParticles; ++i)
			{
				Particle &p = m_parts[i];
				if(i >= particleAmount)
				{
					p.alive = false;
					continue;
				}

				p.alive = true;
				const PhysicsFluid::Particle &fd = buffer[i];
				p.position = fd.position;
				p.velocity = fd.velocity / 67.f;

				float time = eds.particleLife - fd.life;
				if(time < 0)
					time = 0;
				float relativeTime = time / eds.particleLife;
				float previousRelativeTime = (time - PARTICLE_TIME_SCALE) / eds.particleLife;
				if(previousRelativeTime < 0)
					previousRelativeTime = 0.f;

				p.originAlpha = 0.5f;
				p.originSize = 1.f;
				p.previousAlpha = 0.5f;
				p.previousSize = 1.f;
				p.age = time;
				p.life = eds.particleLife;
				p.frameSpeed = frameSpeed;
				p.frame = eds.particleAnimationStartFrame + (time * frameSpeed);
				p.forceAlpha = 0.0f;
				p.angle = eds.particleStartAngle + (eds.particleSpin * time / PARTICLE_TIME_SCALE);
				p.angleSpeed = 0.f;

				Vector c;
				eds.particleColor.getValue(&c, 0.0f);
				p.originColor = COL(c.x, c.y, c.z);
				eds.particleAlpha.getValue(&p.originAlpha, 0.0f);
				eds.particleSize.getValue(&p.originSize, 0.0f);

				eds.particleColor.getValue(&c, previousRelativeTime);
				p.previousColor = COL(c.x, c.y, c.z);
				eds.particleAlpha.getValue(&p.previousAlpha, previousRelativeTime);
				eds.particleSize.getValue(&p.previousSize, previousRelativeTime);

				p.previousAlpha = p.originAlpha;
				p.previousColor = p.originColor;
				p.previousSize = p.originSize;
			}
		}
		*/

		if(fluid && !fluid->renderFlag)
		{

// TEMP TEST
static game::GameRandom *gameRand = 0;
if (gameRand == 0)
{
	gameRand = new game::GameRandom();
}


			fluid->renderFlag = true;
			int particleAmount = 0;
			const PhysicsFluid::Particle *buffer = fluid->getParticles(particleAmount);

			float frameSpeed = 0.f;
			if(eds.particleAnimationType == GenParticleSystemEditables::ANIMATION_LOOP)
				frameSpeed = eds.particleAnimationFps / PARTICLE_TIME_SCALE;
			else
				frameSpeed = ((float)eds.particleAnimationFrameCount - eds.particleAnimationStartFrame) / eds.particleLife;

			std::vector<Particle> &fluid_parts = *m_render_fluid_parts.get();
			int fluid_part_amount = fluid_parts.size();
			for(int i = 0; i < fluid_part_amount; ++i)
			{
				Particle &p = fluid_parts[i];
				if(i >= particleAmount)
				{
					p.alive = false;
					continue;
				}

				p.alive = true;
				const PhysicsFluid::Particle &fd = buffer[i];
				gameRand->seed(fd.id);

				p.position = fd.position;
				p.velocity = fd.velocity / 67.f;

				float time = eds.particleLife - fd.life;
				if(time < 0)
					time = 0;
				float previousRelativeTime = (time - PARTICLE_TIME_SCALE) / eds.particleLife;
				if(previousRelativeTime < 0)
					previousRelativeTime = 0.f;

				p.originAlpha = 0.5f;
				p.originSize = 1.f;
				p.previousAlpha = 0.5f;
				p.previousSize = 1.f;
				p.age = time;
				p.life = eds.particleLife;
				p.frameSpeed = frameSpeed;
				p.frame = eds.particleAnimationStartFrame + (time * frameSpeed);
				p.forceAlpha = 0.0f;
				p.angle = eds.particleStartAngle + (eds.particleSpin * time / PARTICLE_TIME_SCALE);
				p.angleSpeed = 0.f;

				// Hax the variation
				{
					float spin = eds.particleSpinVar * (float)(gameRand->nextInt() % 10301) / 10300.0f;
					p.angle = eds.particleStartAngle + ((spin + eds.particleSpin) * time / PARTICLE_TIME_SCALE);
					p.angle += eds.particleStartAngleVar * (float)(gameRand->nextInt() % 10301) / 10300.0f;

					p.frame += eds.particleAnimationStartFrameVar * (float)(gameRand->nextInt() % 10301) / 10300.0f;
				}

				Vector c;
				eds.particleColor.getValue(&c, 0.0f);
				p.originColor = COL(c.x, c.y, c.z);
				eds.particleAlpha.getValue(&p.originAlpha, 0.0f);
				eds.particleSize.getValue(&p.originSize, 0.0f);

				eds.particleColor.getValue(&c, previousRelativeTime);
				p.previousColor = COL(c.x, c.y, c.z);
				eds.particleAlpha.getValue(&p.previousAlpha, previousRelativeTime);
				eds.particleSize.getValue(&p.previousSize, previousRelativeTime);

				p.previousAlpha = p.originAlpha;
				p.previousColor = p.originColor;
				p.previousSize = p.originSize;
			}

			m_renderer->render(scene, m_mtl, eds, fluid_parts, factor, eds.distortion, eds.faceUpward);
			/*
			m_renderer->render(scene, m_mtl, eds, fluid_parts, factor, false);

			for(int i = 0; i < fluid_part_amount; ++i)
			{
				Particle &p = fluid_parts[i];
				float factor = 0.05f;
				p.forceAlpha = factor;
				//p.originAlpha *= factor;
				//p.previousAlpha *= factor;
			}

			m_renderer->render(scene, m_mtl, eds, fluid_parts, factor, true);
			*/
		}
	}
	else
#endif
#endif

	m_renderer->render(scene, m_mtl, eds, m_parts, factor, eds.distortion, eds.faceUpward);
}

bool GenParticleSystem::moveAndAnimateSystem(const GenParticleSystemEditables& eds) 
{
	// tick inner timer	
 	m_time += PARTICLE_TIME_SCALE;
	if(m_time < eds.emitStartTime)
		return false;
	
	if(m_time > eds.emitStopTime) 
	{
		m_shutdown = true;
		if(m_numParts == 0) 
		{
			m_alive = false;
			return false;
		}
	}

	return true;
}

struct P
{
	VC3 position;
	VC3 velocity;
	float life;
	float density;
	unsigned int id;
};

void GenParticleSystem::emitParticles(const GenParticleSystemEditables& eds) 
{
	if(m_time < eds.emitStartTime)
		return;
	if(m_time > eds.emitStopTime)
		return;

	int bufferSize = m_parts.size();
	if(m_numParts >= bufferSize)
	{
		m_particleResidue += eds.emitRate * emit_factor;
		return;
	}

	float rnd1 = (float)rand() / (float)RAND_MAX;
	float rnd2 = (float)rand() / (float)RAND_MAX;

	Vector dir;
	if(eds.launchDirectionType == GenParticleSystemEditables::DIRECTION_NEGATIVE_VELOCITY) {
		dir = -m_velocity;
	} 
	else if(eds.launchDirectionType == GenParticleSystemEditables::DIRECTION_VELOCITY) {
		dir = m_velocity;
	} 
	else {
		dir = eds.defaultLaunchDirection;
	}

	if(dir.GetSquareLength() < 0.000001f)
	{
		if(eds.defaultLaunchDirection.GetSquareLength() > 0.000001f)
			dir = eds.defaultLaunchDirection;
		else
			dir = VC3(0,0,1.f);
	}

	if(
		( getId() == SprayParticleSystem::getType() || getId() == PointArrayParticleSystem::getType() ) &&
		eds.launchDirectionType == GenParticleSystemEditables::DIRECTION_DEFAULT)
	{
		m_rotation.RotateVector(dir);
	}

	float needed = (eds.emitRate * emit_factor) + m_particleResidue;
	while(needed >= 1.0f) 
	{
		bool found = false;
		int a = 0;

		if(m_numParts >= bufferSize)
			break;

		if(eds.physicsType == GenParticleSystemEditables::PHYSICS_TYPE_FLUID || eds.physicsType == GenParticleSystemEditables::PHYSICS_TYPE_FLUID_INTERACTION)
		{
#ifdef PHYSICS_PHYSX
#ifndef NX_DISABLE_FLUIDS
			if(fluid && fluid->canSpawn())
			{
				int create_amount = int(needed);
				if(create_amount > 600)
					create_amount = 600;
				
				//int activeParticles = 0;
				//fluid->getParticles(activeParticles);
				//assert(activeParticles >= 0);
				int activeParticles = m_numParts;
				assert(activeParticles >= 0);

				if(create_amount + activeParticles > m_maxParticles)
					create_amount = m_maxParticles - activeParticles;

				if(create_amount > 0)
				{
					std::vector<P> buffer(create_amount);
					for(int i = 0; i < create_amount; ++i)
					{
						P &p = buffer[i];

						float speed = eds.launchSpeed + eds.launchSpeedVar * rnd2;
						setParticlePosition(p.position);
						if(eds.launchDirectionType == GenParticleSystemEditables::DIRECTION_EXPLOSION || eds.launchDirectionType == GenParticleSystemEditables::DIRECTION_NEGATIVE_EXPLOSION)
						{
							dir = p.position;
							dir -= explosion_position;

							if(dir.GetSquareLength() > 0.01f)
							{
								dir.Normalize();
								if(eds.launchDirectionType == GenParticleSystemEditables::DIRECTION_NEGATIVE_EXPLOSION)
									dir = -dir;
							}
							else
								dir = VC3();
						}

						setParticleVelocity(p.velocity, dir, speed, eds);
						
						if(!spawnModel)
							p.position += eds.emitterPosition;

						if (eds.emitterVariation.x != 0
							|| eds.emitterVariation.y != 0
							|| eds.emitterVariation.z != 0)
						{
							p.position.x += ((float)(rand() % 1001) / 1000.0f) * eds.emitterVariation.x * 2 - eds.emitterVariation.x;
							p.position.y += ((float)(rand() % 1001) / 1000.0f) * eds.emitterVariation.y * 2 - eds.emitterVariation.y;
							p.position.z += ((float)(rand() % 1001) / 1000.0f) * eds.emitterVariation.z * 2 - eds.emitterVariation.z;
						}
	m_rotation.RotateVector(p.position);
						p.position += m_position;
						p.velocity += m_velocity * eds.velocityInheritanceFactor;
						p.velocity *= 67;
						p.life = eds.particleLife;
						p.density = eds.fluidRestDensity;
					}

					assert(create_amount >= 0);
					m_numParts += create_amount;

					fluid->addParticles(&buffer[0], create_amount);
					needed -= float(create_amount);

					int startIndex = 0;
					for(int i = 0; i < create_amount; ++i)
					{
						for(int j = startIndex; j < int(m_parts.size()); ++j)
						{
							Particle &p = m_parts[j];
							if(p.alive)
								continue;

							p.alive = true;
							p.age = 0;
							p.life = eds.particleLife;
							startIndex = j;

							break;
						}
					}
				}
			}
#endif
#endif

			break;
		}
		else
		{
			for(int i = 1; i < bufferSize + 1; ++i) 
			{
				a = (i + lastParticleIndex) % bufferSize;

				Particle& p = m_parts[a];
				if(!p.alive) 
				{
					found = true;
					lastParticleIndex = a;

					p.forceAlpha = 0.f;
					p.alive = true;
					p.age = 0;
					//p.life = eds.particleLife +  eds.particleLifeVar * rnd1;
					//p.angle = eds.particleStartAngle + eds.particleStartAngleVar * rnd2;
					//p.angleSpeed = eds.particleSpin + eds.particleSpinVar * rnd3;
					p.life = eds.particleLife +  eds.particleLifeVar * (float)(rand() - RAND_MAX/2) / (float)RAND_MAX;
					p.angle = eds.particleStartAngle + eds.particleStartAngleVar * (float)(rand() - RAND_MAX/2) / (float)RAND_MAX;
					p.angleSpeed = eds.particleSpin + eds.particleSpinVar * (float)(rand() - RAND_MAX/2) / (float)RAND_MAX;
					p.frame = eds.particleAnimationStartFrame + eds.particleAnimationStartFrameVar * rnd1;
					
					if(eds.particleAnimationType == GenParticleSystemEditables::ANIMATION_LOOP) {
						p.frameSpeed = eds.particleAnimationFps;
					} else {
						p.frameSpeed = ((float)eds.particleAnimationFrameCount - p.frame) / p.life * PARTICLE_TIME_SCALE;
					}
					
					float speed = eds.launchSpeed + eds.launchSpeedVar * rnd2;
									
					setParticlePosition(p.position);
					if(eds.launchDirectionType == GenParticleSystemEditables::DIRECTION_EXPLOSION || eds.launchDirectionType == GenParticleSystemEditables::DIRECTION_NEGATIVE_EXPLOSION)
					{
						dir = p.position;
						dir -= explosion_position;

						if(dir.GetSquareLength() > 0.01f)
						{
							dir.Normalize();
							if(eds.launchDirectionType == GenParticleSystemEditables::DIRECTION_NEGATIVE_EXPLOSION)
								dir = -dir;
						}
						else
							dir = VC3();
					}

					setParticleVelocity(p.velocity, dir, speed, eds);
					
					p.position += eds.emitterPosition;
					if (eds.emitterVariation.x != 0
						|| eds.emitterVariation.y != 0
						|| eds.emitterVariation.z != 0)
					{
						p.position.x += ((float)(rand() % 1001) / 1000.0f) * eds.emitterVariation.x * 2 - eds.emitterVariation.x;
						p.position.y += ((float)(rand() % 1001) / 1000.0f) * eds.emitterVariation.y * 2 - eds.emitterVariation.y;
						p.position.z += ((float)(rand() % 1001) / 1000.0f) * eds.emitterVariation.z * 2 - eds.emitterVariation.z;
					}
	m_rotation.RotateVector(p.position);

					p.position += m_position;
					p.origin = p.position;

					if(eds.outsideOnly && area && area->isInside(p.position))
					{
						needed -= 1.0f;
						p.alive = false;
						continue;
					}

					if(eds.heightmapHeight && area)
						p.position.y += area->getBaseHeight(p.position) - m_position.y;

					if(eds.obstacleHeight && area)
						p.position.y = area->getObstacleHeight(p.position);

					Vector c;
					eds.particleColor.getValue(&c, 0.0f);
					p.originColor = COL(c.x, c.y, c.z);
					eds.particleAlpha.getValue(&p.originAlpha, 0.0f);
					eds.particleSize.getValue(&p.originSize, 0.0f);

					p.previousAlpha = p.originAlpha;
					p.previousColor = p.originColor;
					p.previousSize = p.originSize;
					
					p.velocity += m_velocity * eds.velocityInheritanceFactor;

					m_numParts++;
					needed -= 1.0f;
					break;
				}
			}
		}

		//	if(a >= (int)m_parts.size())
		//		break;			
		if(!found)
			break;
	}

	m_particleResidue = needed;
}


void GenParticleSystem::moveAndExpireParticles(const GenParticleSystemEditables& eds) 
{
	bool hasParticles = false;
	assert(m_numParts >= 0);

	for(int i = 0; i < (int)m_parts.size(); ++i)
	{
		Particle &p = m_parts[i];

		if(p.alive) 
		{
			hasParticles = true;

			p.age += PARTICLE_TIME_SCALE;
			if(p.age >= p.life) 
			{
				p.alive = false;
				m_numParts--;

				assert(m_numParts >= 0);
			} 
			else 
			{
				p.position += p.velocity;
				p.origin = m_position;
				p.angle += p.angleSpeed;
				p.frame += p.frameSpeed;

				if(eds.outsideFade && area)
				{
					if(area->isInside(p.position))
					{
						p.forceAlpha += 0.05f;
						if(p.forceAlpha > 1.f)
						{
							p.alive = false;
							m_numParts--;
						}
					}
				}
			}
		}
	}

	if(m_shutdown && !hasParticles)
		m_alive = false;

	assert(m_numParts >= 0);
}


void GenParticleSystem::applyForces(const GenParticleSystemEditables &eds) 
{
	VC3 f;

	// apply forces
	int forceAmount = m_forces.size();
	int particleAmount = m_parts.size();

#ifdef PHYSICS_PHYSX
#ifndef NX_DISABLE_FLUIDS
	if(eds.physicsType == GenParticleSystemEditables::PHYSICS_TYPE_FLUID || eds.physicsType == GenParticleSystemEditables::PHYSICS_TYPE_FLUID_INTERACTION)
	{
		if(fluid)
		{
			VC3 pos;
			VC3 vel;
			VC3 force;
			VC3 finalForce;

			for(int i = 0; i < forceAmount; ++i)
			{
				boost::shared_ptr<IParticleForce> &iforce = m_forces[i];
				iforce->preCalc(m_time);

				iforce->calcForce(force, pos, vel);
				finalForce += force / PARTICLE_TIME_SCALE;
			}

			fluid->setAcceleration(finalForce);
		}
	}
	else
#endif
#endif
	{
		for(int i = 0; i < forceAmount; ++i)
		{
			boost::shared_ptr<IParticleForce> &iforce = m_forces[i];

			int forceType = iforce->getTypeId();
			if(forceType == DragParticleForce::getType())
			{
				DragParticleForce *force = static_cast<DragParticleForce *> (iforce.get());
				force->preCalc(m_time);

				for(int j = 0; j < particleAmount; ++j)
				{
					Particle &p = m_parts[j];

					if(p.alive) 
					{
						force->calcForce(f, p.position, p.velocity);
						p.velocity += f;
					}
				}
			}
			else if(forceType == GravityParticleForce::getType())
			{
				GravityParticleForce *force = static_cast<GravityParticleForce *> (iforce.get());
				force->preCalc(m_time);

				for(int j = 0; j < particleAmount; ++j)
				{
					Particle &p = m_parts[j];

					if(p.alive) 
					{
						force->calcForce(f, p.position, p.velocity);
						p.velocity += f;
					}
				}
			}
			else if(forceType == SideGravityParticleForce::getType())
			{
				SideGravityParticleForce *force = static_cast<SideGravityParticleForce *> (iforce.get());
				force->preCalc(m_time);

				for(int j = 0; j < particleAmount; ++j)
				{
					Particle &p = m_parts[j];

					if(p.alive) 
					{
						force->calcForce(f, p.position, p.velocity);
						p.velocity += f;
					}
				}
			}
			else if(forceType == WindParticleForce::getType())
			{
				WindParticleForce *force = static_cast<WindParticleForce *> (iforce.get());
				force->preCalc(m_time);

				for(int j = 0; j < particleAmount; ++j)
				{
					Particle &p = m_parts[j];

					if(p.alive) 
					{
						force->calcForce(f, p.position, p.velocity);
						p.velocity += f;
					}
				}
			}
			/*
			iforce->preCalc(m_time);

			for(int j = 0; j < particleAmount; ++j)
			{
				Particle &p = m_parts[j];

				if(p.alive) 
				{
					iforce->calcForce(f, p.position, p.velocity);
					p.velocity += f;
				}
			}
			*/

		}
	}
}
/*
GenParticleSystemEditables::GenParticleSystemEditables() {

}

GenParticleSystemEditables::GenParticleSystemEditables(bool defaults) {
	emitRate = 10.0f;
	emitStartTime = 0.0f;
	emitStopTime = 5.0f;
	maxParticles = 100;
	emitterPosition = Vector(0.0f, 0.0f, 0.0f);
	emitterVariation = Vector(0.0f, 0.0f, 0.0f);
	launchSpeed = 1.0f;
	launchSpeedVar = 1.0f;
	// particleCOlor;
	// particleSize;
	// particleAlpha;
	particleLife = 1.0f;
	particleLifeVar = 1.0f;
	particleStartAngle = 0.0f;
	particleStartAngleVar = 0.0f;
	particleSpin = 0.0f;
	particleSpinVar = 0.0f;
	particleTexture = "data/particles/flare.jpg";
	particleTextureUSubDivs = 1;
	particleTextureVSubDivs = 1;
	particleTextureAlphaType = 3;
	particleAnimationFrameCount = 0;
	particleAnimationStartFrame = 0;
	particleAnimationStartFrameVar = 0;
	particleAnimationFps = 18.0f;	
}
*/

void GenParticleSystem::defaultParseFrom(const ParserGroup& pg, GenParticleSystemEditables& eds) {

	//static GenParticleSystemEditables def(true);
	
	// parse generic params
	// rememeber to multiply time dependant params with PARTICLE_TIME_SCALE

	eds.emitRate = convertFromString<float>(pg.getValue("emit_rate", ""), eds.emitRate) * PARTICLE_TIME_SCALE;
	eds.emitStartTime = convertFromString<float>(pg.getValue("emit_start", ""), eds.emitStartTime);
	eds.emitStopTime = convertFromString<float>(pg.getValue("emit_stop", ""), eds.emitStopTime);
	eds.emitterPosition = convertVectorFromString(pg.getValue("emitter_position", "0,0,0"));
	eds.emitterVariation = convertVectorFromString(pg.getValue("emitter_variation", "0,0,0"));
	eds.maxParticles = convertFromString<int>(pg.getValue("max_particles", ""), eds.maxParticles);

	// !!!!!!!!!!!!!!!
	//eds.maxParticles *= 10;

	eds.dieAfterEmission = static_cast<bool>(convertFromString<int>(pg.getValue("die_after_emission", ""), eds.dieAfterEmission));
	eds.velocityInheritanceFactor = convertFromString<float>(pg.getValue("velocity_inheritance_factor", ""), eds.velocityInheritanceFactor);
	eds.launchSpeed = convertFromString<float>(pg.getValue("launch_speed", ""), eds.launchSpeed) * PARTICLE_TIME_SCALE;
	eds.launchSpeedVar = convertFromString<float>(pg.getValue("launch_speed_var", ""), eds.launchSpeedVar) * PARTICLE_TIME_SCALE;	
	eds.darkening = convertFromString<float>(pg.getValue("emit_darkening", ""), eds.darkening);	
	const ParserGroup& cg = pg.getSubGroup("particle_color"); 
	parseVectorKeyControlFrom(cg, eds.particleColor.getKeyControl());	
	const ParserGroup& sg = pg.getSubGroup("particle_size"); 
	//parseFloatKeyControlFrom(sg, eds.particleSize.getKeyControl());	
	parseVectorKeyControlFrom(sg, eds.particleSize.getKeyControl());	
	const ParserGroup& ag = pg.getSubGroup("particle_alpha"); 
	parseFloatKeyControlFrom(ag, eds.particleAlpha.getKeyControl());
	eds.particleLife = convertFromString<float>(pg.getValue("particle_life", ""), eds.particleLife);
	eds.particleLifeVar = convertFromString<float>(pg.getValue("particle_life_var", ""), eds.particleLifeVar);
	eds.particleStartAngle = convertFromString<float>(pg.getValue("particle_start_angle", ""), eds.particleStartAngle);
	eds.particleStartAngleVar = convertFromString<float>(pg.getValue("particle_start_angle_var", ""), 0);
	eds.particleSpin = convertFromString<float>(pg.getValue("particle_spin", ""), eds.particleSpin) * PARTICLE_TIME_SCALE;
	eds.particleSpinVar = convertFromString<float>(pg.getValue("particle_spin_var", ""), eds.particleSpinVar) * PARTICLE_TIME_SCALE;
	eds.particleTexture = pg.getValue("texture", eds.particleTexture);
	eds.particleTextureUSubDivs = convertFromString<int>(pg.getValue("texture_sub_div_u", "1"), eds.particleTextureUSubDivs);
	eds.particleTextureVSubDivs = convertFromString<int>(pg.getValue("texture_sub_div_v", "1"), eds.particleTextureVSubDivs);
	eds.particleTextureAlphaType = convertFromString<int>(pg.getValue("texture_alpha_type", ""), eds.particleTextureAlphaType);
	eds.particleAnimationFrameCount = convertFromString<int>(pg.getValue("animation_frame_count", ""), eds.particleAnimationFrameCount);
	eds.particleAnimationStartFrame = convertFromString<float>(pg.getValue("animation_start_frame", ""), eds.particleAnimationStartFrame);
	eds.particleAnimationStartFrameVar = convertFromString<float>(pg.getValue("animation_start_frame_var", ""), eds.particleAnimationStartFrameVar);
	eds.particleAnimationFps = convertFromString<float>(pg.getValue("animation_fps", ""), eds.particleAnimationFps) * PARTICLE_TIME_SCALE;
	eds.distortion = static_cast<bool>(convertFromString<int>(pg.getValue("distortion", ""), false));
	eds.faceUpward = static_cast<bool>(convertFromString<int>(pg.getValue("face_upward", ""), false));
	eds.outsideOnly = static_cast<bool>(convertFromString<int>(pg.getValue("outside_only", ""), false));
	eds.obstacleHeight = static_cast<bool>(convertFromString<int>(pg.getValue("obstacle_spawn", ""), false));
	eds.heightmapHeight = static_cast<bool>(convertFromString<int>(pg.getValue("heightmap_spawn", ""), false));
	eds.outsideFade = static_cast<bool>(convertFromString<int>(pg.getValue("outside_fade", ""), false));

#ifdef _MSC_VER
#pragma message ("***************************************************")
#pragma message ("Quick fix. This really needs logging with filename.")
#pragma message ("***************************************************")
#endif
	if(eds.darkening < 0.f)
		eds.darkening = 0.f;
	if(eds.darkening > 1.f)
		eds.darkening = 1.f;

	if(pg.getValue("animation_type", "loop") == "loop") {
		eds.particleAnimationType = GenParticleSystemEditables::ANIMATION_LOOP;
	} else {
		eds.particleAnimationType = GenParticleSystemEditables::ANIMATION_PARTICLE_LIFE_TIME;
	}
	
	if(pg.getValue("launch_direction", "default") == "velocity") {
		eds.launchDirectionType = GenParticleSystemEditables::DIRECTION_VELOCITY;
	}
	else if(pg.getValue("launch_direction", "default") == "negative_velocity") {
		eds.launchDirectionType = GenParticleSystemEditables::DIRECTION_NEGATIVE_VELOCITY;
	} 
	else if(pg.getValue("launch_direction", "default") == "explosion") {
		eds.launchDirectionType = GenParticleSystemEditables::DIRECTION_EXPLOSION;
	}
	else if(pg.getValue("launch_direction", "default") == "negative_explosion") {
		eds.launchDirectionType = GenParticleSystemEditables::DIRECTION_NEGATIVE_EXPLOSION;
	} 
	else {
		eds.launchDirectionType = GenParticleSystemEditables::DIRECTION_DEFAULT;
	}
	eds.defaultLaunchDirection = convertVectorFromString(pg.getValue("default_direction", "0,1,0"));
	
	
	if(pg.getValue("element_type", "")=="quad") {
		boost::shared_ptr<IParticleRenderer> r(new QuadParticleRenderer());
		m_renderer.swap(r);
	}
	else if(pg.getValue("element_type", "")=="line_from_origin") {
		boost::shared_ptr<IParticleRenderer> r(new LineParticleRenderer1());
		m_renderer.swap(r);
	}
	else if(pg.getValue("element_type", "")=="line") {
		boost::shared_ptr<IParticleRenderer> r(new LineParticleRenderer2());
		m_renderer.swap(r);
		eds.lineLenght = convertFromString<float>(pg.getValue("line_lenght", ""), 0.0f);
	}
	else {
		boost::shared_ptr<IParticleRenderer> r(new PointParticleRenderer());
		m_renderer.swap(r);	
	}

	int lc = convertFromString<int>(pg.getValue("use_len_control", ""), 0);
	eds.useLenghtControl = (lc == 1) ? true : false;
	
	if(pg.getValue("physics_type", "default") == "Physics")
		eds.physicsType = GenParticleSystemEditables::PHYSICS_TYPE_PARTICLE;
	else if(pg.getValue("physics_type", "default") == "Fluid without interaction")
		eds.physicsType = GenParticleSystemEditables::PHYSICS_TYPE_FLUID;
	else if(pg.getValue("physics_type", "default") == "Fluid with interaction")
		eds.physicsType = GenParticleSystemEditables::PHYSICS_TYPE_FLUID_INTERACTION;

	eds.fluidStaticRestitution = convertFromString<float>(pg.getValue("fluid_static_restitution", ""), eds.fluidStaticRestitution);
	eds.fluidStaticAdhesion = convertFromString<float>(pg.getValue("fluid_static_adhesion", ""), eds.fluidStaticAdhesion);
	eds.fluidDynamicRestitution = convertFromString<float>(pg.getValue("fluid_dynamic_restitution", ""), eds.fluidDynamicRestitution);
	eds.fluidDynamicAdhesion = convertFromString<float>(pg.getValue("fluid_dynamic_adhesion", ""), eds.fluidDynamicAdhesion);
	eds.fluidDamping = convertFromString<float>(pg.getValue("fluid_damping", ""), eds.fluidDamping);
	eds.fluidMotionLimit = convertFromString<float>(pg.getValue("fluid_motion_limit", ""), eds.fluidMotionLimit);
	eds.fluidPacketSizeMultiplier = convertFromString<int>(pg.getValue("fluid_packet_size_multiplier", ""), eds.fluidPacketSizeMultiplier);
	eds.fluidMaxEmitterAmount = convertFromString<int>(pg.getValue("fluid_max_emitter_amount", ""), eds.fluidMaxEmitterAmount);
	eds.fluidDetailCollisionGroup = convertFromString<int>(pg.getValue("fluid_detail_collision_group", ""), eds.fluidDetailCollisionGroup);

	eds.fluidStiffness = convertFromString<float>(pg.getValue("fluid_stiffness", ""), eds.fluidStiffness);
	eds.fluidViscosity = convertFromString<float>(pg.getValue("fluid_viscosity", ""), eds.fluidViscosity);
	eds.fluidKernelRadiusMultiplier = convertFromString<float>(pg.getValue("fluid_kernel_radius_multiplier", ""), eds.fluidKernelRadiusMultiplier);
	eds.fluidRestParticlesPerMeter = convertFromString<float>(pg.getValue("fluid_rest_particles_per_meter", ""), eds.fluidRestParticlesPerMeter);
	eds.fluidRestDensity = convertFromString<float>(pg.getValue("fluid_rest_density", ""), eds.fluidRestDensity);


	// parse forces

	int nForces;
	nForces = convertFromString<int>(pg.getValue("num_forces", "0"), 0);	
	for(int i = 0; i < nForces; i++) {
		std::string str = "force" + boost::lexical_cast<std::string>(i);
		const ParserGroup& fg = pg.getSubGroup(str);
		std::string className = fg.getValue("class", "");
		if(className.empty())
			continue;
		IParticleForce* force = addForce(className);
		force->parseFrom(fg);
	}
	
}

	
void GenParticleSystem::kill() {
	m_shutdown = true;
}

bool GenParticleSystem::isDead() {
	return !m_alive;
}

/*	
void GenParticleSystem::init(IStorm3D* s3d, IStorm3D_Scene* scene) {

	std::string textureName;
	m_pb->getValue(PB_PARTICLE_TEXTURE, textureName);
	m_mtl = ParticleSystemManager::getSingleton().getMaterial(textureName);

}
*/
/*
void GenParticleSystem::prepareForLaunch(IStorm3D* s3d, IStorm3D_Scene* scene, 
										 const GenParticleSystemEditables& eds) {

	m_maxParticles = eds.maxParticles;
	m_mesh.resize(m_maxParticles);
	for(int i = 0; i < m_parts.size(); i++)
		m_parts[i].alive = false;
	
	m_animInfo.colums = eds.particleTextureUSubDivs;
	m_animInfo.rows = eds.particleTextureVSubDivs;
	m_animInfo.frames = eds.particleAnimationFrameCount;
	m_time = 0;
	m_particleResidue = 0;
	m_numParts = 0;
	m_alive = true;
	m_shutdown = false;

}
*/

void GenParticleSystem::setSpawnModel(IStorm3D_Model *model)
{
	spawnHelpers.clear();
	spawnModel = model;

	if(spawnModel)
	{
		//virtual IStorm3D_Helper *SearchHelper(const char *name)=0;

		for(int i = 1; i <= 50; ++i)
		{
			std::string name = "HELPER_BONE_ParticleSpawn";
			name += boost::lexical_cast<std::string> (i);
/*
if(i % 2)
	name = "HELPER_BONE_ShoulderLamp";
else
	name = "HELPER_BONE_Weapon";
*/
			IStorm3D_Helper *helper = spawnModel->SearchHelper(name.c_str());
			if(helper)
				spawnHelpers.push_back(helper);
		}
	}
}

void GenParticleSystem::setEmitFactor(float factor)
{
	emit_factor = factor;
}

void GenParticleSystem::setLighting(const COL &ambient_, const signed short int *lightIndices)
{
	//COL ref = ambient_ + lightCol;
	COL ref = ambient_;
	ref.Clamp();

	ambient = ref;
}

void GenParticleSystem::setExplosion(const Vector& pos, bool enable)
{
	explosion_position = pos;
	use_explosion = enable;
}

void GenParticleSystem::setArea(boost::shared_ptr<IParticleArea> &area_)
{
	area = area_;
}

void GenParticleSystem::setPhysics(boost::shared_ptr<ParticlePhysics> &physics_)
{
	physics = physics_;
}

void GenParticleSystem::releasePhysicsResources()
{
	//physics.reset();
	fluid.reset();
}

} // particle
} // frozenbyte
