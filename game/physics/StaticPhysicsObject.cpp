#include "precompiled.h"

#include <assert.h>
#include <string>
#include <map>

#include "StaticPhysicsObject.h"

#include "GamePhysics.h"
#include "../system/FileTimestampChecker.h"
#include "../system/Logger.h"
#ifdef PHYSICS_PHYSX
#include "../../physics/physics_lib.h"
#include "../../physics/cooker.h"
#include "../../physics/static_mesh_actor.h"
#include "../../physics/actor_base.h"
#include <IStorm3D_Model.h>
#include <igios.h>
#endif

namespace game
{
#ifdef PHYSICS_PHYSX
	typedef std::map<std::string, boost::shared_ptr<frozenbyte::physics::StaticMesh> > MeshHash;
#endif

	class StaticPhysicsObjectImpl
	{
	private:
		StaticPhysicsObjectImpl(const char *filename, IStorm3D_Model *model)
		{
			this->filename = filename;
			this->model = model;
		}

#ifdef PHYSICS_PHYSX
		StaticPhysicsObjectImpl(boost::shared_ptr<frozenbyte::physics::StaticMesh> &mesh)
		{
			this->filename = std::string("");
			this->model = NULL;
			this->mesh = mesh;
		}
#endif

		~StaticPhysicsObjectImpl()
		{
			// nop?
		}

#ifdef PHYSICS_PHYSX
		boost::shared_ptr<frozenbyte::physics::StaticMesh> getMesh(GamePhysics *gamePhysics, const VC3 &position, const QUAT &rotation)
		{
			if(mesh)
				return mesh;

			MeshHash::iterator iter = meshHash.find(this->filename);
			if (iter != meshHash.end())
			{
				return (*iter).second;
			}

			frozenbyte::physics::Cooker cooker;
			std::string cookfile = filename + ".cook";

			if (!FileTimestampChecker::isFileUpToDateComparedTo(cookfile.c_str(), filename.c_str()))
			{
				// cooking failed
				if(!cooker.cookMesh(cookfile.c_str(), model))
				{
					// return NULL pointer
					{
						char msg[1024];
						sprintf( msg, "Failed generating cook file: %s", cookfile.c_str() );
					   Logger::getInstance()->error(msg);
					}
					return boost::shared_ptr<frozenbyte::physics::StaticMesh>();
				}
			}

			boost::shared_ptr<frozenbyte::physics::StaticMesh> m = gamePhysics->getPhysicsLib()->createStaticMesh(cookfile.c_str());

			// loading from cooked file failed
			if(!m || !m->getMesh())
			{
				// return NULL pointer
				::Logger::getInstance()->error("StaticPhysicsObject::getMesh - loading cooked file failed.");
				::Logger::getInstance()->error(cookfile.c_str());
				return boost::shared_ptr<frozenbyte::physics::StaticMesh>();
			}

			meshHash.insert(std::pair<std::string, boost::shared_ptr<frozenbyte::physics::StaticMesh> >(filename, m));

			// MOVED FROM BUILDINGADDER
			if(model)
			{
				model->FreeMemoryResources();
			}

			return m;
		}
#endif

		std::string filename;
		IStorm3D_Model *model;

#ifdef PHYSICS_PHYSX
		boost::shared_ptr<frozenbyte::physics::StaticMesh> mesh;
		static MeshHash meshHash;
#endif

		friend class StaticPhysicsObject;
	};

#ifdef PHYSICS_PHYSX
	MeshHash StaticPhysicsObjectImpl::meshHash = MeshHash();
#endif


	StaticPhysicsObject::StaticPhysicsObject(GamePhysics *gamePhysics, const char *filename, IStorm3D_Model *model, const VC3 &position, const QUAT &rotation) 
		: AbstractPhysicsObject(gamePhysics)
	{ 
		this->impl = new StaticPhysicsObjectImpl(filename, model);
		this->position = position;
		this->rotation = rotation;
		this->dynamicActor = false;
	}

#ifdef PHYSICS_PHYSX
	StaticPhysicsObject::StaticPhysicsObject(GamePhysics *gamePhysics, boost::shared_ptr<frozenbyte::physics::StaticMesh> &mesh, const VC3 &position, const QUAT &rotation)
		: AbstractPhysicsObject(gamePhysics)
	{
		this->impl = new StaticPhysicsObjectImpl(mesh);
		this->position = position;
		this->rotation = rotation;
		this->dynamicActor = false;
	}
#endif

	StaticPhysicsObject::~StaticPhysicsObject() 
	{
		delete impl;
	}

	PHYSICS_ACTOR StaticPhysicsObject::createImplementationObject()
	{
#ifdef PHYSICS_PHYSX
		boost::shared_ptr<frozenbyte::physics::StaticMesh> staticMeshSPtr = impl->getMesh(gamePhysics, position, rotation);
		
		// cooking failed
		if(!staticMeshSPtr)
		{
			// return NULL pointer
			return PHYSICS_ACTOR();
		}

		boost::shared_ptr<frozenbyte::physics::ActorBase> actor = gamePhysics->getPhysicsLib()->createStaticMeshActor(staticMeshSPtr, position, rotation);

		if(actor)
			actor->setIntData(soundMaterial);

		if(impl->mesh)
			impl->mesh.reset();

		return actor;
#endif
#ifdef PHYSICS_ODE
		PhysicsActorOde actor = PhysicsActorOde::createNewStaticPhysicsObject(impl->filename.c_str(), position, rotation);
		return actor;
#endif
#ifdef PHYSICS_NONE
		PhysicsActorNone actor;
		return actor;
#endif
	}

	void StaticPhysicsObject::syncImplementationObject(PHYSICS_ACTOR &obj)
	{
		// this is a static object, no syncing...
		//AbstractPhysicsObject::syncImplementationObject(obj);
	}

	void StaticPhysicsObject::clearImplementationResources()
	{
#ifdef PHYSICS_PHYSX
		StaticPhysicsObjectImpl::meshHash.clear();
#endif
	}

}


