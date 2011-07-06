#include "precompiled.h"
#include "spherical_joint.h"
#include "actor_base.h"
#include "NxPhysics.h"

namespace frozenbyte {
namespace physics {

SphericalJoint::SphericalJoint(NxScene &scene, const NxSphericalJointDesc &desc, boost::shared_ptr<ActorBase> &a, boost::shared_ptr<ActorBase> &b)
:	JointBase(scene, a, b), jointDesc(new NxSphericalJointDesc(desc))
{
	joint = scene.createJoint(desc);
}

SphericalJoint::~SphericalJoint()
{
}

bool SphericalJoint::isValid() const
{
	if(joint)
		return true;

	return false;
}

void SphericalJoint::reconnect(boost::shared_ptr<ActorBase> a1, boost::shared_ptr<ActorBase> a2)
{
	NxScene &scene = joint->getScene();
	scene.releaseJoint(*joint);
	
	jointDesc->actor[0] = a1 && a1->getActor() && a1->getActor()->isDynamic() ? a1->getActor() : NULL;
	jointDesc->actor[1] = a2 && a2->getActor() && a2->getActor()->isDynamic() ? a2->getActor() : NULL;
	joint = scene.createJoint(*jointDesc);
	this->actor1 = a1;
	this->actor2 = a2;
}

} // physics
} // frozenbyte
