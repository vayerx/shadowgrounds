
#ifndef PHYSICSCONTACTSCRIPTMANAGER_H
#define PHYSICSCONTACTSCRIPTMANAGER_H

#include <DatatypeDef.h>
#include "IPhysicsContactListener.h"

namespace game
{
	class IGamePhysicsObject;
	class GameUI;
	class PhysicsContactScriptManagerImpl;


	class PhysicsContactScriptManager : public IPhysicsContactListener
	{
	public:
		PhysicsContactScriptManager(Game *game);
		~PhysicsContactScriptManager();

		virtual void physicsContact(const PhysicsContact &contact);

		void reloadConfiguration();

	private:
		PhysicsContactScriptManagerImpl *impl;
	};
}

#endif
