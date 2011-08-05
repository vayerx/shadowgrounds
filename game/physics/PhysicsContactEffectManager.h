
#ifndef PHYSICSCONTACTEFFECTMANAGER_H
#define PHYSICSCONTACTEFFECTMANAGER_H

#include <DatatypeDef.h>
#include "IPhysicsContactListener.h"

namespace game
{
	class IGamePhysicsObject;
	class Game;
	class PhysicsContactEffectManagerImpl;


	class PhysicsContactEffectManager : public IPhysicsContactListener
	{
	public:
		PhysicsContactEffectManager(Game *game);
		~PhysicsContactEffectManager();

		virtual void physicsContact(const PhysicsContact &contact);

		void reloadConfiguration();

		/*
		void deleteAllEffects();
		void updateEffects();
		*/

	private:
		PhysicsContactEffectManagerImpl *impl;
	};
}

#endif
