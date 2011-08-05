
#ifndef PHYSICSCONTACTFEEDBACK_H
#define PHYSICSCONTACTFEEDBACK_H

#include <datatypedef.h>
#include "IPhysicsContactListener.h"

namespace game
{
	class IGamePhysicsObject;
	class PhysicsContactFeedbackImpl;
	class Game;


	class PhysicsContactFeedback : public IPhysicsContactListener
	{
	public:
		PhysicsContactFeedback(Game *game);
		~PhysicsContactFeedback();

		virtual void physicsContact(const PhysicsContact &contact);

		void tick();

	private:
		PhysicsContactFeedbackImpl *impl;
	};
}

#endif
