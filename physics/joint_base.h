#ifndef INCLUDED_FROZENBYTE_JOINT_BASE_H
#define INCLUDED_FROZENBYTE_JOINT_BASE_H

#include <DatatypeDef.h>
#include <boost/shared_ptr.hpp>

class NxJoint;
class NxScene;

namespace frozenbyte {
namespace physics {

class ActorBase;

class JointBase
{
protected:
	NxJoint *joint;
	NxScene &scene;

	boost::shared_ptr<ActorBase> &actor1;
	boost::shared_ptr<ActorBase> &actor2;

	JointBase(NxScene &scene, boost::shared_ptr<ActorBase> &a, boost::shared_ptr<ActorBase> &b);
	virtual ~JointBase();

	void init();

public:
	// Interface
	virtual bool isValid() const = 0;
	NxJoint *getJoint() const;
};

} // physics
} // frozenbyte

#endif
