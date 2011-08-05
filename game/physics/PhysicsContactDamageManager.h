
#ifndef PHYSICSCONTACTDAMAGEMANAGER_H
#define PHYSICSCONTACTDAMAGEMANAGER_H

#include <DatatypeDef.h>
#include "IPhysicsContactListener.h"

namespace game
{
	class IGamePhysicsObject;
	class Game;
	class PhysicsContactDamageManagerImpl;


	class PhysicsContactDamageManager : public IPhysicsContactListener
	{
	public:
		PhysicsContactDamageManager(Game *game);
		~PhysicsContactDamageManager();

		void reloadConfiguration();

		virtual void physicsContact(const PhysicsContact &contact);

	private:
		PhysicsContactDamageManagerImpl *impl;
	};
}

#endif
