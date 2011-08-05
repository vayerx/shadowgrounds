
#include "precompiled.h"

#include "PhysicsContactFeedback.h"
#include "AbstractPhysicsObject.h"
#include "GamePhysics.h"
#include "../Game.h"

#include <sstream>


namespace game
{

	class PhysicsContactFeedbackImpl
	{
	public:
		PhysicsContactFeedbackImpl(Game *game)
		{
			this->game = game;
		};

		~PhysicsContactFeedbackImpl()
		{
		};

		void tick()
		{
			if (!objectHandlesToReset.empty())
			{
				for (int i = 0; i < (int)objectHandlesToReset.size(); i++)
				{
					int handle = objectHandlesToReset[i];
					// WARNING: unsafe cast!
					AbstractPhysicsObject *o = (AbstractPhysicsObject *)this->game->getGamePhysics()->getInterfaceObjectForHandle(handle);
					if (o != NULL)
					{
						o->setFeedbackNormal(VC3(0,0,0));
						o->setFeedbackNormalLeft(VC3(0,0,0));
						o->setFeedbackNormalRight(VC3(0,0,0));
					}
				}
			}
			objectHandlesToReset.clear();
		}

		Game *game;
		std::vector<int> objectHandlesToReset;
	};



	PhysicsContactFeedback::PhysicsContactFeedback(Game *game)
	{
		impl = new PhysicsContactFeedbackImpl(game);
	}

	PhysicsContactFeedback::~PhysicsContactFeedback()
	{
		delete impl;
	}

	void PhysicsContactFeedback::tick()
	{
		impl->tick();
	}

	void PhysicsContactFeedback::physicsContact(const PhysicsContact &contact)
	{
#ifdef PHYSICS_PHYSX
		// do this for physx. (ODE implementation does not need this whole class...)

		// WARNING: unsafe IGamePhysicsObject -> AbstractPhysicsObject casts!
		AbstractPhysicsObject *o1 = (AbstractPhysicsObject *)contact.obj1;
		AbstractPhysicsObject *o2 = (AbstractPhysicsObject *)contact.obj2;
		//assert(o1 != NULL);
		//assert(o2 != NULL);
		assert(contact.physicsObject1);
		assert(contact.physicsObject2);

		if (o1 == NULL || o2 == NULL)
			return;

		for (int i = 0; i < 2; i++)
		{
			AbstractPhysicsObject *o = o1;
			if (i == 1) 
			{
				o = o2;
			}

			if(o && o->isFeedbackEnabled())
			{
				VC3 prev;

				impl->objectHandlesToReset.push_back(o->getHandle());

#ifdef PROJECT_CLAW_PROTO
				o->setFeedbackNormal(o->getFeedbackNormal() + (contact.contactNormal * contact.contactForceLen));
#else

				prev = o->getFeedbackNormal();
				if (prev.z < contact.contactNormal.z)
				{
					o->setFeedbackNormal(contact.contactNormal);
				}

				prev = o->getFeedbackNormalLeft();
				if (prev.x > contact.contactNormal.x)
				{
					o->setFeedbackNormalLeft(contact.contactNormal);
				}

				prev = o->getFeedbackNormalRight();
				if (prev.x < contact.contactNormal.x)
				{
					o->setFeedbackNormalRight(contact.contactNormal);
				}
#endif
			}
		}
#endif
	}
}


