#include "precompiled.h"

#include <assert.h>
#include <map>
#include <string>

#include "ConvexPhysicsObject.h"

#include "GamePhysics.h"
#include "../../physics/physics_lib.h"
#include "../../physics/convex_actor.h"
#include "../../physics/actor_base.h"

#ifdef PHYSICS_PHYSX

namespace game
{
	typedef std::map<std::string, boost::shared_ptr<frozenbyte::physics::ConvexMesh> > ConvexMeshHash;

	class ConvexPhysicsObjectImpl
	{
	private:
		ConvexPhysicsObjectImpl(const char *filename)
		{
			this->filename = filename;
		}

		~ConvexPhysicsObjectImpl()
		{
			// nop?
		}

		boost::shared_ptr<frozenbyte::physics::ConvexMesh> getMesh(GamePhysics *gamePhysics)
		{
			ConvexMeshHash::iterator iter = meshHash.find(this->filename);
			if (iter != meshHash.end())
			{
				return (*iter).second;
			}

			boost::shared_ptr<frozenbyte::physics::ConvexMesh> m = gamePhysics->getPhysicsLib()->createConvexMesh(this->filename.c_str());
			meshHash.insert(std::pair<std::string, boost::shared_ptr<frozenbyte::physics::ConvexMesh> >(filename, m));
			return m;
		}

		std::string filename;

		static ConvexMeshHash meshHash;

		friend class ConvexPhysicsObject;
	};

	ConvexMeshHash ConvexPhysicsObjectImpl::meshHash = ConvexMeshHash();


	ConvexPhysicsObject::ConvexPhysicsObject(GamePhysics *gamePhysics, const char *filename, float mass, int collisionGroup, const VC3 &position) 
		: AbstractPhysicsObject(gamePhysics)
	{ 
		this->mass = mass;
		this->position = position;
		this->impl = new ConvexPhysicsObjectImpl(filename);
		this->collisionGroup = collisionGroup;
	}

	ConvexPhysicsObject::~ConvexPhysicsObject() 
	{
		delete impl;
	}

	boost::shared_ptr<frozenbyte::physics::ActorBase> ConvexPhysicsObject::createImplementationObject()
	{
		boost::shared_ptr<frozenbyte::physics::ConvexMesh> convexMeshSPtr = impl->getMesh(gamePhysics);
		boost::shared_ptr<frozenbyte::physics::ActorBase> actor = gamePhysics->getPhysicsLib()->createConvexActor(convexMeshSPtr, position);
		if(actor)
		{
			actor->setMass(mass);
			actor->setCollisionGroup(collisionGroup);
			actor->setIntData(soundMaterial);
		}

		return actor;
	}

	void ConvexPhysicsObject::syncImplementationObject(boost::shared_ptr<frozenbyte::physics::ActorBase> &obj)
	{
		AbstractPhysicsObject::syncImplementationObject(obj);
	}

	void ConvexPhysicsObject::clearImplementationResources()
	{
		ConvexPhysicsObjectImpl::meshHash.clear();
	}

}

#endif


