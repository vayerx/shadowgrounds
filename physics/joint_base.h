#ifndef INCLUDED_FROZENBYTE_JOINT_BASE_H
#define INCLUDED_FROZENBYTE_JOINT_BASE_H

#include <datatypedef.h>
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

	boost::shared_ptr<ActorBase> actor1;
	boost::shared_ptr<ActorBase> actor2;

	JointBase(NxScene &scene, boost::shared_ptr<ActorBase> &a, boost::shared_ptr<ActorBase> &b);
	~JointBase();

	void init();
public:
	// Interface
	virtual bool isValid() const = 0;
	NxJoint *getJoint() const;

	boost::shared_ptr<ActorBase> getActor1() const { return actor1; }
	boost::shared_ptr<ActorBase> getActor2() const { return actor2; }
	
	virtual void reconnect(boost::shared_ptr<ActorBase> actor1, boost::shared_ptr<ActorBase> actor2) = 0;
	virtual void handleDeforming(NxScene &scene);
};

} // physics
} // frozenbyte

#endif
