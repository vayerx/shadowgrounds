#include <boost/lexical_cast.hpp>

#include "precompiled.h"

// Copyright 2002-2004 Frozenbyte Ltd.

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
#include "parseutil.h"
#include "particlesystem.h"
#include "particleeffect.h"
#include "modelparticlesystem.h"
#include "particletiming.h"
#include "particleforces.h"
#ifdef PHYSICS_PHYSX
#include "ParticlePhysics.h"
#endif

#include "../physics/physics_lib.h"
#include "../physics/convex_actor.h"
#include "../physics/cooker.h"

#ifdef _MSC_VER
#pragma warning(disable: 4800)
#endif
using namespace frozenbyte::editor;

namespace frozenbyte {
namespace particle {
namespace {
	int MPSid = 0;

	void randomAxis(QUAT &q, float start)
	{
		VC3 v;
		v.x = float(rand()) / RAND_MAX - .5f;
		v.y = float(rand()) / RAND_MAX - .5f;
		v.z = float(rand()) / RAND_MAX - .5f;

		if(v.GetSquareLength() > 0.00000000001f)
			v.Normalize();
		else
		{
			v.z = 1.f;
		}

		q.x = v.x;
		q.y = v.y;
		q.z = v.z;
		q.w = start;
	}

	QUAT getRotation(const QUAT &q)
	{
		QUAT r;
		VC3 axis(q.x, q.y, q.z);

		r.MakeFromAxisRotation(axis, q.w);
		return r;
	}
}

void ModelParticleSystem::apply(Particle &p)
{
	if(!p.object)
		return;

	p.object->SetPosition(p.position);
	QUAT rot = m_rotation.GetRotation();
	QUAT r1 = getRotation(p.axis[0]);
	QUAT r2 = getRotation(p.axis[1]);
	p.object->SetRotation(rot * p.originalRotation * r1 * r2);

#ifdef PHYSICS_PHYSX
	if(p.actor && p.actor->isEnabled())
	{
		//p.object->SetNoRender(false);

		VC3 pos;
		p.actor->getPosition(pos);
		QUAT rot;
		p.actor->getRotation(rot);

		p.object->SetPosition(pos - position);
		p.object->SetRotation(rot);
	}
#endif
	float alpha = p.alpha;
	if(alpha < 0.011f)
		alpha = 0.011f;

	p.object->SetForceAlpha(1.f - alpha);
}

void ModelParticleSystem::spawn()
{
	if(!m_parray)
		return;
	if(time < m_eds->emitStartTime)
		return;

	Vector dir;
	if(m_eds->launchDirectionType == GenParticleSystemEditables::DIRECTION_NEGATIVE_VELOCITY)
		dir = -velocity;
	else if(m_eds->launchDirectionType == GenParticleSystemEditables::DIRECTION_VELOCITY)
		dir = velocity;
	else
		dir = m_eds->defaultLaunchDirection;

	float rnd2 = 0;

	int a = 0;
	float needed = (m_eds->emitRate * emit_factor) + residue;
	VC3 normVelocity;
	if(velocity.GetSquareLength() > 0.000000001f)
		normVelocity = velocity.GetNormalized();

	while(needed >= 1.f)
	{
		for(; a < int(particles.size()); ++a)
		{
			Particle &p = particles[a];
			if(p.alive)
				continue;

			float speed = m_eds->launchSpeed + m_eds->launchSpeedVar * rnd2;
			assert(m_parray->verts.size() > 0);
			int index = rand() % m_parray->verts.size();

			getPosition(p.position, index);

			// Temp - assume projectile rotation is objects rotation
			VC3 pos = p.originalPosition;
	//		m_rotation.RotateVector(pos);

			if(m_eds->launchDirectionType == GenParticleSystemEditables::DIRECTION_EXPLOSION || m_eds->launchDirectionType == GenParticleSystemEditables::DIRECTION_NEGATIVE_EXPLOSION)
			{
					//VC3 fudge;
					//fudge.x = ((float)(rand() - RAND_MAX/2) / (float)RAND_MAX/2) * 0.5f;
					//fudge.y = ((float)(rand() - RAND_MAX/2) / (float)RAND_MAX/2) * 0.5f;


				dir = pos;
				//dir = p.originalPosition + fudge;
				dir -= explosion_position;

				if(dir.GetSquareLength() > 0.01f)
				{
					dir.Normalize();
					if(m_eds->launchDirectionType == GenParticleSystemEditables::DIRECTION_NEGATIVE_EXPLOSION)
						dir = -dir;
				}
				else
					dir = VC3();
			}

			getVelocity(p.velocity, dir, speed, index);
			p.position += pos;

			p.position += m_eds->emitterPosition;
			m_eds->particleAlpha.getValue(&p.alpha, 0.0f);
			p.velocity += m_velocity * m_eds->velocityInheritanceFactor;

	m_rotation.RotateVector(p.position);
			p.position += position;

			if(collision)
			{
				// if physics use static collision from there
				//if(!collision->spawnPosition(position, normVelocity, p.position))
				//	continue;
			}

			p.position -= position;

			p.object->SetNoRender(false);
			p.alive = true;
			p.life = m_eds->particleLife + m_eds->particleLifeVar * (float)(rand() - RAND_MAX/2) / (float)RAND_MAX;
			p.age = 0;
			p.moving = true;

			randomAxis(p.axis[0], m_eds->rotateStart[0]);
			randomAxis(p.axis[1], m_eds->rotateStart[1]);

			p.rotateSpeed[0] = m_eds->rotateSpeed[0];
			p.rotateSpeed[1] = m_eds->rotateSpeed[1];

			apply(p);

#ifdef PHYSICS_PHYSX
			if(m_eds->collision && physics && p.shape)
			{
				QUAT rot = m_rotation.GetRotation();
				QUAT r1 = getRotation(p.axis[0]);
				QUAT r2 = getRotation(p.axis[1]);

				{
					//VC3 dist = p.originalPosition - explosion_position;
					//float len = dist.GetLength();
					//p.velocity *= 1.f / (len*len);
				}

				VC3 velocity = p.velocity / PARTICLE_TIME_SCALE;
				//float limit = 10.f;
				//float len = velocity.GetLength();
				//if(len > limit)
				//	velocity = velocity.GetNormalized() * limit;

				/*
				// Uhm
				velocity.x *= 0.40f;
				velocity.y *= 0.40f;
				velocity.z *= 0.40f;
				*/

				float mass = 0.1f;
				VC3 angularVelocity;

				if(p.velocity.GetSquareLength() > 0.001f)
				{
					QUAT rot;
					rotateToward(VC3(0, 1.f, 0), p.velocity.GetNormalized(), rot);

					VC3 test(rot.x, rot.y, rot.z);
					test.x += ((float)(rand() - RAND_MAX/2) / (float)RAND_MAX/2);
					test.z += ((float)(rand() - RAND_MAX/2) / (float)RAND_MAX/2);

					if(test.GetSquareLength() > 0.001f)
						test.Normalize();

					//p.actor->setAngularVelocity(test * 2.f);
					angularVelocity = test * 2.f;
				}
				else
				{
					VC3 test;
					test.x += ((float)(rand() - RAND_MAX/2) / (float)RAND_MAX/2);
					test.y += ((float)(rand() - RAND_MAX/2) / (float)RAND_MAX/2);
					test.z += ((float)(rand() - RAND_MAX/2) / (float)RAND_MAX/2);

					if(test.GetSquareLength() > 0.001f)
						test.Normalize();

					//p.actor->setAngularVelocity(test * 2.f);
					angularVelocity = test * 2.f;
				}

				int collisionGroup = 0;
				if(!m_eds->soundMaterialIndex)
				{
					if(m_eds->unitCollision)
						collisionGroup = 4;
					else
						collisionGroup = 3;
				}
				else
				{
					if(m_eds->unitCollision)
						collisionGroup = 2;
					else
						collisionGroup = 1;
				}

				p.actor = physics->createActor(p.shape, p.position + position, m_rotation_quat, velocity, angularVelocity, mass, collisionGroup, m_eds->soundMaterialIndex);
			}
#endif

			m_numParts++;
			needed -= 1.0f;
			break;
		}

		if(a >= int(particles.size()))
			break;
	}

	residue = needed;
}

void ModelParticleSystem::update()
{
	// Update forces
	{
		/*
		for(unsigned int i = 0; i < m_forces.size(); ++i) 
		{
			Vector f;	
			m_forces[i]->preCalc(m_time);

			for(unsigned int j = 0; j < particles.size(); ++j)
			{
				Particle &p = particles[j];
				if(p.alive) 
				{
					m_forces[i]->calcForce(f, p.position, p.velocity);
					p.velocity += f;
				}
			}
		}
		*/

		VC3 f;
		// apply forces
		int forceAmount = m_forces.size();
		int particleAmount = particles.size();

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
					Particle &p = particles[j];

					if(p.alive) 
					{
						force->calcForce(f, p.position, p.velocity);
						p.velocity += f;

#ifdef PHYSICS_PHYSX
						if(p.actor)
							p.actor->applyForce(f / PARTICLE_TIME_SCALE);
#endif
					}
				}
			}
			else if(forceType == GravityParticleForce::getType())
			{
				GravityParticleForce *force = static_cast<GravityParticleForce *> (iforce.get());
				force->preCalc(m_time);

				for(int j = 0; j < particleAmount; ++j)
				{
					Particle &p = particles[j];

					if(p.alive) 
					{
						force->calcForce(f, p.position, p.velocity);
						p.velocity += f;

#ifdef PHYSICS_PHYSX
						if(p.actor)
							p.actor->applyForce(f / PARTICLE_TIME_SCALE);
#endif
					}
				}
			}
			else if(forceType == SideGravityParticleForce::getType())
			{
				SideGravityParticleForce *force = static_cast<SideGravityParticleForce *> (iforce.get());
				force->preCalc(m_time);

				for(int j = 0; j < particleAmount; ++j)
				{
					Particle &p = particles[j];

					if(p.alive) 
					{
						force->calcForce(f, p.position, p.velocity);
						p.velocity += f;

#ifdef PHYSICS_PHYSX
						if(p.actor)
							p.actor->applyForce(f / PARTICLE_TIME_SCALE);
#endif
					}
				}
			}
			else if(forceType == WindParticleForce::getType())
			{
				WindParticleForce *force = static_cast<WindParticleForce *> (iforce.get());
				force->preCalc(m_time);

				for(int j = 0; j < particleAmount; ++j)
				{
					Particle &p = particles[j];

					if(p.alive) 
					{
						force->calcForce(f, p.position, p.velocity);
						p.velocity += f;

#ifdef PHYSICS_PHYSX
						if(p.actor)
							p.actor->applyForce(f / PARTICLE_TIME_SCALE);
#endif
					}
				}
			}
		}
	}

	// Update particles
	for(unsigned int i = 0; i < particles.size(); ++i)
	{
		Particle &p = particles[i];
		if(!p.alive)
			continue;

		p.age += PARTICLE_TIME_SCALE;
		if(p.age >= p.life)
		{
			--m_numParts;
			p.alive = false;
			if(p.object)
			{
				p.object->SetNoRender(true);
				if(p.actor)
					p.actor.reset();
			}

			continue;
		}

		if(p.moving)
		{
			VC3 oldPosition = p.position;
			p.position += p.velocity;

			//if(collision)
#ifdef PHYSICS_PHYSX
			if(collision && (!p.actor || !p.actor->isEnabled()))
#else
			if(collision)
#endif
			{
				oldPosition += position;
				p.position += position;

				if(m_eds->collision)
				{
					//if(collision->getCollision(oldPosition, p.position, p.velocity, .95f, .30f, .4f))
					if(collision->getCollision(oldPosition, p.position, p.velocity, m_eds->slowFactor, m_eds->slowFactorY, m_eds->slowFactorXZ))
					{
						p.rotateSpeed[0] *= .85f;
						p.rotateSpeed[1] *= .85f;
					}
				}

				p.position -= position;
			}

			p.axis[0].w += p.rotateSpeed[0];
			p.axis[1].w += p.rotateSpeed[1];

			if(collision && p.velocity.GetSquareLength() < 0.00001f && p.rotateSpeed[0] < 0.01f && p.rotateSpeed[1] < 0.01f)
				p.moving = false;
		}

		float t = p.age / p.life;
		m_eds->particleAlpha.getValue(&p.alpha, t);

		apply(p);
	}
}

void ModelParticleSystem::getPosition(VC3 &position, int index)
{
	if(!m_parray)
		return;

	position = m_parray->verts[index];
//	m_rotation.RotateVector(position);
//	emitterTm.RotateVector(position);
}

void ModelParticleSystem::getVelocity(VC3 &velocity, const VC3 &dir, float speed, int index)
{
	if(!m_parray)
		return;
	
	bool forceDirection = !use_explosion && (m_eds->launchDirectionType == GenParticleSystemEditables::DIRECTION_EXPLOSION || m_eds->launchDirectionType == GenParticleSystemEditables::DIRECTION_NEGATIVE_EXPLOSION);
	if(m_eds->useNormalsAsDirection || forceDirection)
	{
		velocity = m_parray->normals[index] * speed;
		emitterTm.RotateVector(velocity);
		m_rotation.RotateVector(velocity);
	}
	else
		velocity = dir * speed;
}

ModelParticleSystem::ModelParticleSystem()
:	time(0),
	residue(0)
{
}

boost::shared_ptr<IParticleSystem> ModelParticleSystem::createNew()
{
	ModelParticleSystem *ms = new ModelParticleSystem();
	boost::shared_ptr<ModelParticleSystemEditables> e(new ModelParticleSystemEditables);
	ms->m_eds.swap(e);

	return boost::shared_ptr<IParticleSystem> (ms);
}

boost::shared_ptr<IParticleSystem> ModelParticleSystem::clone()
{
	ModelParticleSystem *ms = new ModelParticleSystem();
	copyTo(*ms);
	ms->m_eds = m_eds;
	ms->original = original;

	if (m_parray)
	{
		ms->m_parray.reset(new PointArray());
		*ms->m_parray = *m_parray;
	} else {
		ms->m_parray.reset();
		assert(!"ModelParticleSystem::clone - null pointarray.");
	}

	//ms->physics = physics;
	return boost::shared_ptr<IParticleSystem>(ms);
}

void ModelParticleSystem::setLighting(const COL &ambient, const signed short int *lightIndices)
{
	if(model)
	{
		//model->SetSelfIllumination(ambient);

		if(lightIndices)
		{
			for(int i = 0; i < LIGHT_MAX_AMOUNT; ++i)
				model->SetLighting(i, lightIndices[i]);
		}
	}
}

void ModelParticleSystem::setCollision(boost::shared_ptr<IParticleCollision> &collision_)
{
	collision = collision_;
}

void ModelParticleSystem::setPhysics(boost::shared_ptr<ParticlePhysics> &physics_)
{
	physics = physics_;
}

void ModelParticleSystem::releasePhysicsResources()
{
	//physics.reset();

	for(unsigned int i = 0; i < particles.size(); i++)
	{
		Particle &p = particles[i];
		p.shape.reset();
		p.actor.reset();
	}

	original.shapes.clear();
}

void ModelParticleSystem::setEmitterRotation(const QUAT &rotation)
{
	emitterTm.CreateRotationMatrix(rotation);
}

void *ModelParticleSystem::getId() const
{
	return &MPSid;
}

void *ModelParticleSystem::getType()
{
	return &MPSid;
}

void ModelParticleSystem::init(IStorm3D *s3d, IStorm3D_Scene *scene)
{
	// Spawn point array
	{
		std::string fileName = m_eds->modelFile;
		if(fileName.empty())
			return;
		
		Matrix sm;
		Matrix rm;
		QUAT q;
		q.MakeFromAngles(m_eds->rotation.x, m_eds->rotation.y, m_eds->rotation.z);	
		rm.CreateRotationMatrix(q);
		sm.CreateScaleMatrix(m_eds->scale);
		sm.Multiply(rm);

		IStorm3D_Model *model = s3d->CreateNewModel();
		if(model && model->LoadS3D(fileName.c_str())) 
		{
			Iterator<IStorm3D_Model_Object*>* obj = model->ITObject->Begin();
			IStorm3D_Mesh *mesh = obj->GetCurrent()->GetMesh();
			if(mesh) 
			{
				boost::shared_ptr<PointArray> pm(new PointArray());
				pm->verts.resize(mesh->GetVertexCount());
				pm->normals.resize(mesh->GetVertexCount());

				Storm3D_Vertex *v = mesh->GetVertexBuffer();
				for(int i = 0; i < mesh->GetVertexCount(); i++) 
				{
					Vector pos = v[i].position;
					Vector nor = v[i].normal;
					sm.TransformVector(pos);
					rm.RotateVector(nor);				
					pm->verts[i] = pos;
					pm->normals[i] = nor;
				}

				m_parray.swap(pm);
			}

			delete obj;
		}
		
		delete model;
	}

	// Particle model
	{
		std::string fileName = m_eds->particleFile;
		if(fileName.empty())
			return;

		IStorm3D_Model *model = s3d->CreateNewModel();
		original.model.reset(model);

		if(model && model->LoadS3D(fileName.c_str())) 
		{
			Iterator<IStorm3D_Model_Object*> *obj = model->ITObject->Begin();
			for(; !obj->IsEnd(); obj->Next())
			{
				IStorm3D_Model_Object *o = obj->GetCurrent();
				if(o->GetNoRender())
					continue;

				IStorm3D_Mesh *m = o->GetMesh();
				Mesh mesh;
				mesh.position = o->GetPosition();
				mesh.rotation = o->GetRotation();
				mesh.mesh = m;
				original.meshes.push_back(mesh);

				if(m_eds->collision && physics && fileName.size() > 4)
				{
					/*
					std::string fname = fileName.substr(0, fileName.size() - 4);

					// Generate filename
					{
						fname += '_';
						const char *str = o->GetName();
						int nameLength = strlen(str);
						for(int i = 0; i < nameLength; ++i)
						{
							char next = str[i];

							if(next == ':')
								next = '_';
							if(next == ' ')
								next = '_';
							if(next == '*')
								next = '_';

							if(isascii(next) || next == '_' || next == '(' || next == ')' || next == '*')
								fname += tolower(next);
						}

						fname += ".fbp";
						fname.c_str();
					}
					*/

					/*
					boost::shared_ptr<physics::ConvexMesh> shape = physics->createConvexMesh(fname.c_str());
					if(!shape->isValid())
					{
						physics::Cooker cooker;
						cooker.cookApproxConvex(fname.c_str(), o);

						shape = physics->createConvexMesh(fname.c_str());
					}
					*/

#ifdef PHYSICS_PHYSX
					boost::shared_ptr<PhysicsMesh> shape = physics->createConvexMesh(0, o);
					original.shapes.push_back(shape);
#endif
				}
			}

			delete obj;
		}
	}
}

void ModelParticleSystem::prepareForLaunch(IStorm3D *s3d, IStorm3D_Scene *scene)
{
	model.reset(s3d->CreateNewModel());
	scene->AddModel(model.get());	
	model->CastShadows(false);
	model->SetNoCollision(true);

	int maxParticles = m_eds->maxParticles;
	particles.resize(maxParticles);

	for(unsigned int i = 0; i < particles.size(); i++)
	{
		Particle &p = particles[i];
		p.alive = false;

		p.object = model->Object_New("...");
		p.object->SetNoRender(true);
		p.object->SetSpotTransparencyFactor(0.25f);

		if(original.meshes.size())
		{
			int index = i % original.meshes.size();
			const Mesh &mesh = original.meshes[index];

			p.object->SetMesh(mesh.mesh);
			p.originalPosition = mesh.position;
			p.originalRotation = mesh.rotation;

			if(!original.shapes.empty())
				p.shape = original.shapes[index];
		}
	}

	time = 0;
	residue = 0;
}

void ModelParticleSystem::tick(IStorm3D_Scene *scene)
{
	if(!model)
		return;

	time += PARTICLE_TIME_SCALE;
	if(time > m_eds->emitStopTime)
	{
		m_shutdown = true;
		if(!m_numParts)
			m_alive = false;
	}
	else if(time > m_eds->emitStartTime && !m_shutdown)
		spawn();

	update();
}

void ModelParticleSystem::render(IStorm3D_Scene* scene)
{
	// nop
}

void ModelParticleSystem::parseFrom(const editor::ParserGroup& pg, const util::SoundMaterialParser &materialParser)
{
	defaultParseFrom(pg, *m_eds);
	m_eds->modelFile = pg.getValue("model", "");
	m_eds->scale = convertVectorFromString(pg.getValue("scale", "1,1,1"));
	m_eds->rotation = convertVectorFromString(pg.getValue("rotation", "0,0,0"));
	m_eds->useNormalsAsDirection = static_cast<bool>(convertFromString<int>(pg.getValue("direction_from_normals", ""), 0));
	m_eds->particleFile = pg.getValue("particle_model", "");

	m_eds->rotateStart[0] = convertFromString<float>(pg.getValue("particle1_angle_start", ""), 0);
	m_eds->rotateSpeed[0] = convertFromString<float>(pg.getValue("particle1_angle_speed", ""), 0) / 10.f;
	m_eds->rotateStart[1] = convertFromString<float>(pg.getValue("particle2_angle_start", ""), 0);
	m_eds->rotateSpeed[1] = convertFromString<float>(pg.getValue("particle2_angle_speed", ""), 0) / 10.f;

	//.95f, .30f, .4f
	m_eds->collision = convertFromString<bool>(pg.getValue("collision", ""), true);
	m_eds->slowFactor = convertFromString<float>(pg.getValue("slow_factor", ""), 0.95f);
	m_eds->slowFactorY = convertFromString<float>(pg.getValue("y_slow_factor", ""), 0.30f);
	m_eds->slowFactorXZ = convertFromString<float>(pg.getValue("xz_slow_factor", ""), 0.4f);
	m_eds->unitCollision = convertFromString<bool>(pg.getValue("unit_collision", ""), true);

	std::string soundMaterial = pg.getValue("sound_material", "NoSound");
	m_eds->soundMaterialIndex = materialParser.getMaterialIndexByName(soundMaterial);
}

void ModelParticleSystem::setPosition(const Vector &position_)
{
	position = position_;

	if(model)
		model->SetPosition(position);
}

void ModelParticleSystem::setVelocity(const Vector &velocity_)
{
	velocity = velocity_;
	m_velocity = velocity;
}

ModelParticleSystemEditables &ModelParticleSystem::getEditables()
{
	return *m_eds;
}

const ModelParticleSystemEditables &ModelParticleSystem::getEditables() const
{
	return *m_eds;
}

} // particle
} // frozenbyte
