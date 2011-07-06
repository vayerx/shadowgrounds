
#include "precompiled.h"

#include "RopeJoint.h"
#include "../Game.h"
#include "../unified_handle.h"
#include "../UnifiedHandleManager.h"

namespace game
{

	RopeJoint::RopeJoint(Game *game)
	{
		this->game = game;
	}

	void RopeJoint::attachEnd(int ropeEndNumber, UnifiedHandle obj, const VC3 &localOffset)
	{
		assert(ropeEndNumber >= 0 && ropeEndNumber < ROPEJOINT_ENDS);
		assert(obj == UNIFIED_HANDLE_NONE || VALIDATE_UNIFIED_HANDLE_BITS(obj));

		this->objHandle[ropeEndNumber] = obj;
		this->localOffset[ropeEndNumber] = localOffset;
	}

	void RopeJoint::setParameters(float springFactor, float dampingFactor)
	{
		this->springFactor = springFactor;
		this->dampingFactor = dampingFactor;
	}

	void RopeJoint::setLength(float length)
	{
		this->length = length;
	}

	RopeJoint::~RopeJoint()
	{
	}

	void RopeJoint::run()
	{
		for (int i = 0; i < ROPEJOINT_ENDS; i++)
		{
			if (game->unifiedHandleManager->doesObjectExist(this->objHandle[i]))
			{

			} else {
				// the object must have broken or something? detach.
				// TODO: should either connect to self or maybe to last known global position for this end...
				attachEnd(i, UNIFIED_HANDLE_NONE, VC3(0,0,0));
			}
		}
	}

}

