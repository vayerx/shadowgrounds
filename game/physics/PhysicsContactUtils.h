
#ifndef PHYSICSCONTACTUTILS_H
#define PHYSICSCONTACTUTILS_H

namespace game
{
	class AbstractPhysicsObject;
	class Unit;
	class Game;

	class PhysicsContactUtils
	{
	public:
		static void mapPhysicsObjectToUnitOrTerrainObject(Game *game, AbstractPhysicsObject *o, Unit **unit, int *terrObjModelId, int *terrObjInstanceId);

		static int calcCustomPhysicsObjectDataForUnit(Game *game, Unit *unit);
		static int calcCustomPhysicsObjectDataForTerrainObject(int terrObjModelId, int terrObjInstanceId);
	};
}

#endif
