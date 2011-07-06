
#include "precompiled.h"

// Copyright 2002-2004 Frozenbyte Ltd.


#pragma warning( disable : 4800 )

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>
#include <string>
#include <map>
#include <list>
#include <fstream>
#include <storm3d_ui.h>
#include "..\editor\string_conversions.h"
#include "..\editor\parser.h"
#include "track.h"
//#include "paramblock.h"
#include "parseutil.h"
#include "particlesystem.h"
#include "particleeffect.h"
#include "pointarrayparticlesystem.h"
#ifdef PHYSICS_PHYSX
#include "particlephysics.h"
#include "../game/physics/physics_collisiongroups.h"
#endif

// TEMP TEST
#include "..\game\GameRandom.h"


using namespace frozenbyte::editor;

namespace frozenbyte
{
namespace particle
{

namespace {
	int id = 0;
}

PointArrayParticleSystem::PointArrayParticleSystem() 
{
	m_index = 0;	
}
	
boost::shared_ptr<IParticleSystem> PointArrayParticleSystem::createNew() 
{
	PointArrayParticleSystem* ps = new PointArrayParticleSystem();
	boost::shared_ptr<IParticleSystem> ptr(ps);
	boost::shared_ptr<PointArrayParticleSystemEditables> e(new PointArrayParticleSystemEditables);
	ps->m_eds.swap(e);
	return ptr;
}

boost::shared_ptr<IParticleSystem> PointArrayParticleSystem::clone() 
{
	PointArrayParticleSystem* ps = new PointArrayParticleSystem();
	copyTo(*ps);
	ps->m_eds = m_eds;
	ps->m_parray = m_parray;
	boost::shared_ptr<IParticleSystem> ptr(ps);

#ifdef PHYSICS_PHYSX

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

	return ptr;
}
/*
void PointArrayParticleSystem::setRotation(const MAT &tm)
{
	rotation = tm;
}
*/

// HACK: ...
//static bool useHelperVel = false;
//static VC3 helperVel = VC3(0,0,0);

void PointArrayParticleSystem::setParticlePosition(Vector& pos) 
{	
	//useHelperVel = false;
	if(m_parray.get()==NULL)
	{
#ifdef PROJECT_PARTICLE_EDITOR
		static bool pointarraywarninggiven = false;
		if (!pointarraywarninggiven)
		{
			pointarraywarninggiven = true;
			MessageBox(0, "Null point array encountered (model load failed or no filename given)\r\nThis warning will only be shown once.\r\nRestart particle editor to see this message again.", "Warning", MB_ICONEXCLAMATION|MB_OK);
		}
#endif
		return;
	}
	
	if(spawnModel)
	{
		if(!spawnHelpers.empty())
		{
			int index = rand() % spawnHelpers.size();
			if(m_eds->randomizeBetweenVertices
				&& spawnHelperEnds[index] != -1)
			{
				float fact = (float)(rand() % 1000) / 1000.0f;
				pos = spawnHelpers[index]->GetGlobalPosition() * fact;
				pos += spawnHelpers[spawnHelperEnds[index]]->GetGlobalPosition() * (1.0f - fact);

				pos += lastModelVelocity;
			} else {
				pos = spawnHelpers[index]->GetGlobalPosition();
			}
		}
		else
			pos = spawnModel->GetPosition();
	}
	else if(m_eds->randomizeBetweenVertices)
	{
// TEMP TEST
#pragma message("FIXME! - PointArrayParticleSystem::setParticlePosition")
static game::GameRandom *gameRand;
if (gameRand == NULL)
{
	gameRand = new game::GameRandom();
	gameRand->seed(0);
}


		m_index = m_eds->firstVertex + gameRand->nextInt() % (m_eds->lastVertex - m_eds->firstVertex - 1);	
		pos = m_parray->verts[m_index];

		int secondaryIndex = m_index + 1;
//		float primaryFactor = (float)(rand() % 10301) / 10300.0f;
		float primaryFactor = (float)(gameRand->nextInt() % 10301) / 10300.0f;
		float secondaryFactor = 1.0f - primaryFactor;
		pos = (m_parray->verts[m_index] * primaryFactor) + (m_parray->verts[secondaryIndex] * secondaryFactor);

		if(m_eds->planePositions)
		{
			int tertiaryIndex = m_index + 2;
			if (tertiaryIndex >= m_eds->lastVertex)
				tertiaryIndex = m_index;
//			float tertiaryFactor = (float)(rand() % 7307) / 7306.0f;
			float tertiaryFactor = (float)(gameRand->nextInt() % 7307) / 7306.0f;
			float tertInvFactor = 1.0f - tertiaryFactor;
  			pos = (pos * tertiaryFactor) + (m_parray->verts[tertiaryIndex] * tertInvFactor);
		}
	} 
	else 
	{
		m_index = m_eds->firstVertex + rand() % (m_eds->lastVertex - m_eds->firstVertex);	
		pos = m_parray->verts[m_index];
	}

	//m_rotation.RotateVector(pos);
}
	
void PointArrayParticleSystem::setParticleVelocity(Vector& vel, const Vector& dir, float speed, const GenParticleSystemEditables& eds) 
{
	
	if(m_parray.get()==NULL)
		return;
	
	bool forceDirection = !use_explosion && (eds.launchDirectionType == GenParticleSystemEditables::DIRECTION_EXPLOSION || eds.launchDirectionType == GenParticleSystemEditables::DIRECTION_NEGATIVE_EXPLOSION);
	if(m_eds->useNormalsAsDirection || forceDirection)
	{
		if(m_eds->useBinormalsAsDirection)
		{
			//VC3 up = VC3(0,1,0);
			VC3 up = VC3((rand() % 1000) / 1000.0f, (rand() % 1000) / 1000.0f, (rand() % 1000) / 1000.0f);
			VC3 binormal = up.GetCrossWith(m_parray->normals[m_index]);
			vel = binormal * speed;	
			m_rotation.RotateVector(vel);
		} else {
			vel = m_parray->normals[m_index] * speed;	
			m_rotation.RotateVector(vel);
		}
	}
	else
		vel = dir * speed;
}

void *PointArrayParticleSystem::getId() const
{
	return &id;
}

void *PointArrayParticleSystem::getType()
{
	return &id;
}

void PointArrayParticleSystem::init(IStorm3D* s3d, IStorm3D_Scene* scene) 
{	
	defaultInit(s3d, scene, *m_eds);

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
	IStorm3D_Model* model = s3d->CreateNewModel();
	assert(model != NULL);

	if(model->LoadS3D(fileName.c_str())) 
	{
		Iterator<IStorm3D_Model_Object*>* obj = model->ITObject->Begin();
		assert(obj != NULL);

		boost::shared_ptr<PointArray> pm(new PointArray());
		for(; !obj->IsEnd(); obj->Next())
		{
			IStorm3D_Mesh* mesh = obj->GetCurrent()->GetMesh();
			VC3 opos = obj->GetCurrent()->GetPosition();

			if(mesh) 
			{
				int base = pm->verts.size();
				pm->verts.resize(base + mesh->GetVertexCount());
				pm->normals.resize(base + mesh->GetVertexCount());
				
				Storm3D_Vertex *v = mesh->GetVertexBuffer();
				for(int i = 0; i < mesh->GetVertexCount(); i++) 
				{
					Vector pos = v[i].position + opos;
					Vector nor = v[i].normal;
					sm.TransformVector(pos);
					rm.RotateVector(nor);

					pm->verts[base + i] = pos;
					pm->normals[base + i] = nor;
				}
			}
		}

		m_parray.swap(pm);
		if(m_eds->firstVertex < 0)
			m_eds->firstVertex = 0;
		if(m_eds->lastVertex >= (int)m_parray->verts.size())
			m_eds->lastVertex = m_parray->verts.size()-1;

		delete obj;
	}
	delete model;
}

void PointArrayParticleSystem::tick(IStorm3D_Scene* scene) {
	GenParticleSystem::defaultTick(scene, *m_eds);

	if (this->spawnModel)
	{
		modelPositionSamplingCounter++;
		if (modelPositionSamplingCounter >= 4)
		{
			modelPositionSamplingCounter = 0;
			VC3 modelPos = this->spawnModel->GetPosition();
			/*
			if (modelPos.x != lastModelPosition.x
				|| modelPos.y != lastModelPosition.y
				|| modelPos.z != lastModelPosition.z)
			{
				lastModelVelocity = modelPos - lastModelPosition;
			}
			*/
			if (modelPos.x < lastModelPosition.x)
			{
				lastModelVelocity.x = -0.2f;
			}
			else if (modelPos.x > lastModelPosition.x)
			{
				lastModelVelocity.x = 0.2f;
			}
			else
			{
				lastModelVelocity = 0.0f;
			}
			lastModelPosition = modelPos;
		}
	}
}

void PointArrayParticleSystem::prepareForLaunch(IStorm3D* s3d, IStorm3D_Scene* scene) {
	GenParticleSystem::defaultPrepareForLaunch(s3d, scene, *m_eds);
}

void PointArrayParticleSystem::render(IStorm3D_Scene* scene) {
	GenParticleSystem::defaultRender(scene, *m_eds);
}

	
void PointArrayParticleSystem::parseFrom(const ParserGroup& pg, const util::SoundMaterialParser &materialParser) {
	defaultParseFrom(pg, *m_eds);
	m_eds->modelFile = pg.getValue("model", "");
	m_eds->firstVertex = convertFromString<int>(pg.getValue("first_vertex", ""), 0);
	m_eds->lastVertex = convertFromString<int>(pg.getValue("last_vertex", ""), 0);
	m_eds->scale = convertVectorFromString(pg.getValue("scale", "1,1,1"));
	m_eds->rotation = convertVectorFromString(pg.getValue("rotation", "0,0,0"));
	m_eds->useNormalsAsDirection = static_cast<bool>(convertFromString<int>(pg.getValue("direction_from_normals", ""), 0));
	m_eds->useBinormalsAsDirection = static_cast<bool>(convertFromString<int>(pg.getValue("direction_from_binormals", ""), 0));
	m_eds->randomizeBetweenVertices = static_cast<bool>(convertFromString<int>(pg.getValue("positions_between_vertices", ""), 0));
	m_eds->planePositions = static_cast<bool>(convertFromString<int>(pg.getValue("plane_positions", ""), 0));
}
	
PointArrayParticleSystemEditables& PointArrayParticleSystem::getEditables() {
	return *m_eds;
}


const PointArrayParticleSystemEditables& PointArrayParticleSystem::getEditables() const {
	return *m_eds;
}




} // particle

} // frozenbyte