
#include "precompiled.h"

#include "TerrainPhysicsObject.h"

#include "GamePhysics.h"
#include "../../physics/physics_lib.h"
#include "../../physics/heightmap_actor.h"
#include "../../physics/actor_base.h"

#ifndef PHYSICS_NONE

namespace game
{
	TerrainPhysicsObject::TerrainPhysicsObject(GamePhysics *gamePhysics, const unsigned short *buffer_, int samplesX_, int samplesY_, const VC3 &size_) 
	:	AbstractPhysicsObject(gamePhysics)
	{
		buffer = buffer_;
		samplesX = samplesX_;
		samplesY = samplesY_;
		size = size_;
		dynamicActor = false;
	}

	TerrainPhysicsObject::~TerrainPhysicsObject() 
	{
		// nop, ~AbstractPhysicsObject handles everything of any interest...
	}

	boost::shared_ptr<frozenbyte::physics::ActorBase> TerrainPhysicsObject::createImplementationObject()
	{
		boost::shared_ptr<frozenbyte::physics::ActorBase> actor = gamePhysics->getPhysicsLib()->createHeightmapActor(buffer, samplesX, samplesY, size);
		if(actor)
			actor->setIntData(soundMaterial);

		return actor;
	}

	void TerrainPhysicsObject::syncImplementationObject(boost::shared_ptr<frozenbyte::physics::ActorBase> &obj)
	{
		AbstractPhysicsObject::syncImplementationObject(obj);
		// TODO: sync mass if it has changed... (in future, if mass changeable)
	}
}

#endif // #ifndef PHYSICS_NONE
