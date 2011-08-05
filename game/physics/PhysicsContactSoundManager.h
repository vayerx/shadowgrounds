
#ifndef PHYSICSCONTACTSOUNDMANAGER_H
#define PHYSICSCONTACTSOUNDMANAGER_H

#include <DatatypeDef.h>
#include "IPhysicsContactListener.h"

namespace game
{
	class IGamePhysicsObject;
	class GameUI;
	class PhysicsContactSoundManagerImpl;


	class PhysicsContactSoundManager : public IPhysicsContactListener
	{
	public:
		PhysicsContactSoundManager(GameUI *gameUI);
		~PhysicsContactSoundManager();

		virtual void physicsContact(const PhysicsContact &contact);

		void reloadConfiguration();

	private:
		PhysicsContactSoundManagerImpl *impl;
	};
}

#endif
