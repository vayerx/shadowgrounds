
#ifndef ROPEJOINT_H
#define ROPEJOINT_H

#include "joint_class_ids.h"
#include "IJoint.h"
#include "../unified_handle_type.h"

// a rope has 2 ends. duh. :)
#define ROPEJOINT_ENDS 2

namespace game
{
	class Game;

/**
 * Software rope type joint.
 */ 
class RopeJoint : public IJoint
{
	RopeJoint(Game *game);

	~RopeJoint();

	void attachEnd(int ropeEndNumber, UnifiedHandle obj, const VC3 &localOffset);

	void setParameters(float springFactor, float dampingFactor);

	void setLength(float length);

	virtual int getJointClassId() { return JOINT_CLASS_ID_ROPEJOINT; }

	virtual void run();

protected:
	Game *game;
	UnifiedHandle objHandle[ROPEJOINT_ENDS];
	VC3 localOffset[ROPEJOINT_ENDS];
	float length;
	float springFactor;
	float dampingFactor;

};

}

#endif

