#include "precompiled.h"
#include "joint_base.h"
#include "NxPhysics.h"

namespace frozenbyte {
namespace physics {

JointBase::JointBase(NxScene &scene_, boost::shared_ptr<ActorBase> &a, boost::shared_ptr<ActorBase> &b)
:	joint(0),
	scene(scene_),
	actor1(a),
	actor2(b)
{
}

JointBase::~JointBase()
{
	if(joint)
		scene.releaseJoint(*joint);

}

void JointBase::init()
{
}

NxJoint *JointBase::getJoint() const
{
	return joint;
}

} // physics
} // frozenbyte
