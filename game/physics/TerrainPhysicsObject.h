
#ifndef TERRAINPHYSICSOBJECT_H
#define TERRAINPHYSICSOBJECT_H

#include "AbstractPhysicsObject.h"
#include "DatatypeDef.h"

#ifndef PHYSICS_NONE

namespace game
{
	class GamePhysics;

	class TerrainPhysicsObject : public AbstractPhysicsObject
	{
	public:
		TerrainPhysicsObject(GamePhysics *gamePhysics, const unsigned short *buffer, int samplesX, int samplesY, const VC3 &size);
		virtual ~TerrainPhysicsObject();

	protected:
		virtual boost::shared_ptr<frozenbyte::physics::ActorBase> createImplementationObject();

		virtual void syncImplementationObject(boost::shared_ptr<frozenbyte::physics::ActorBase> &obj);

		const unsigned short *buffer;
		int samplesX;
		int samplesY;
		VC3 size;
	};
}

#endif // #ifndef PHYSICS_NONE

#endif

