#ifndef INCLUDED_FROZENBYTE_SPHERICAL_JOINT_H
#define INCLUDED_FROZENBYTE_SPHERICAL_JOINT_H

#include "joint_base.h"

class NxScene;
class NxSphericalJointDesc;

namespace frozenbyte {
namespace physics {

class SphericalJoint: public JointBase
{
private:
	boost::shared_ptr<NxSphericalJointDesc> jointDesc;

public:
	SphericalJoint(NxScene &scene, const NxSphericalJointDesc &desc, boost::shared_ptr<ActorBase> &a, boost::shared_ptr<ActorBase> &b);
	~SphericalJoint();

	void reconnect(boost::shared_ptr<ActorBase> actor1, boost::shared_ptr<ActorBase> actor2);

	bool isValid() const;
};

} // physics
} // frozenbyte

#endif
