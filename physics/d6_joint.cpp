#include "precompiled.h"
#include "physics_lib.h"
#include "d6_joint.h"
#include "actor_base.h"
#include "NxPhysics.h"

namespace frozenbyte {
namespace physics {

struct D6Joint::DeformData
{
	DeformData(float bendAngle, float breakAngle, float durability, int resetCollisionGroupOnBreak, float resetAngularDampingOnBreak) :
		bendCosAngle(cosf(bendAngle)),
		breakCosAngle(cosf(breakAngle)),
		durability(durability),
		resetCollisionGroupOnBreak(resetCollisionGroupOnBreak),
		resetAngularDampingOnBreak(resetAngularDampingOnBreak),
		originalAxisSet(false)
	{
	}

	float bendCosAngle;
	float breakCosAngle;
	float durability;
	int resetCollisionGroupOnBreak;
	float resetAngularDampingOnBreak;

	NxVec3 originalAxis[2];
	bool originalAxisSet;
	JointDeformingListener *listener;
};

D6Joint::D6Joint(NxScene &scene, const NxD6JointDesc &desc, boost::shared_ptr<ActorBase> &a, boost::shared_ptr<ActorBase> &b)
:	JointBase(scene, a, b), jointDesc(new NxD6JointDesc(desc))
{
	joint = scene.createJoint(desc);
}

D6Joint::~D6Joint()
{
}

bool D6Joint::isValid() const
{
	if(joint)
		return true;

	return false;
}

void D6Joint::reconnect(boost::shared_ptr<ActorBase> a1, boost::shared_ptr<ActorBase> a2)
{
	NxScene &scene = joint->getScene();
	scene.releaseJoint(*joint);

	jointDesc->actor[0] = a1 && a1->getActor() && a1->getActor()->isDynamic() ? a1->getActor() : NULL;
	jointDesc->actor[1] = a2 && a2->getActor() && a2->getActor()->isDynamic() ? a2->getActor() : NULL;
	joint = scene.createJoint(*jointDesc);
	this->actor1 = a1;
	this->actor2 = a2;
}

void D6Joint::setJointDeforming(const JointDeformingInfo *info)
{
	deformData.reset(new DeformData(info->bendAngle, info->breakAngle, info->durability, info->resetCollisionGroupOnBreak, info->resetAngularDampingOnBreak));
}

void D6Joint::setJointDeformingListener(JointDeformingListener *listener)
{
	if(deformData)
		deformData->listener = listener;
}
void D6Joint::handleDeforming(NxScene &scene)
{
	if(!deformData || !joint)
		return;

	NxD6Joint *d6Joint = joint->isD6Joint();
	if(!d6Joint)
		return;

	NxActor *a1 = NULL;
	NxActor *a2 = NULL;
	d6Joint->getActors(&a1, &a2);

	int actorNum = 0;
	int staticNum = 1;
	NxActor *actor = NULL;
	if(a1 && a1->isDynamic())
	{
		actorNum = 0;
		staticNum = 1;
		actor = a1;
	}
	else
	{
		actorNum = 1;
		staticNum = 0;
		actor = a2;
	}
	if(!actor || !actor->isDynamic())
		return;

	if(!deformData->originalAxisSet)
	{
		deformData->originalAxis[0] = d6Joint->getGlobalAxis();
		deformData->originalAxis[1] = jointDesc->localAxis[actorNum];
		deformData->originalAxisSet = true;
	}

	NxVec3 jointAxis = d6Joint->getGlobalAxis();
	NxVec3 anchorPoint = d6Joint->getGlobalAnchor();
	// this is a bit of a hack..
	NxVec3 bodyPoint = actor->getGlobalPose() * (jointDesc->localAnchor[actorNum] + deformData->originalAxis[1]);
	NxVec3 dir(bodyPoint - anchorPoint);
	dir.normalize();

	float dot = dir.dot(jointAxis);
	float dotToOriginal = dir.dot(deformData->originalAxis[0]);

	// angle to original axis exceeds breaking limit (or joint worn out)
	if(dotToOriginal < deformData->breakCosAngle || deformData->durability < 0 || joint->getState() == NX_JS_BROKEN)
	{
		// break
		scene.releaseJoint(*joint);
		joint = NULL;

		if(deformData->resetCollisionGroupOnBreak >= 0)
		{
			if(this->actor1 && this->actor1->isDynamic())
				this->actor1->setCollisionGroup(deformData->resetCollisionGroupOnBreak);
			else if(this->actor2 && this->actor2->isDynamic())
				this->actor2->setCollisionGroup(deformData->resetCollisionGroupOnBreak);
		}

		if(deformData->resetAngularDampingOnBreak >= 0.0f)
		{
			if(this->actor1 && this->actor1->isDynamic())
				this->actor1->setAngularDamping(deformData->resetAngularDampingOnBreak);
			else if(this->actor2 && this->actor2->isDynamic())
				this->actor2->setAngularDamping(deformData->resetAngularDampingOnBreak);
		}

		// call listener
		if(deformData->listener)
		{
			VC3 breakPoint(anchorPoint.x, anchorPoint.y, anchorPoint.z);
			deformData->listener->onJointBreak(this, breakPoint);
		}
	}
	// angle to current axis exceeds bending limit
	else if(dot < deformData->bendCosAngle)
	{
		// recalculate a new axis so that the joint
		// is resting with the current orientation

		jointDesc->localAxis[staticNum] = dir;
		jointDesc->localNormal[staticNum].cross(dir, jointAxis);
		jointDesc->localNormal[staticNum].normalize();

		NxQuat orientation = actor->getGlobalOrientationQuat();
		orientation.invert();

		NxVec3 localAxis = jointDesc->localAxis[staticNum];
		NxVec3 localNormal = jointDesc->localNormal[staticNum];
		orientation.rotate(localAxis);
		orientation.rotate(localNormal);

		jointDesc->localAxis[actorNum] = localAxis;
		jointDesc->localNormal[actorNum] = localNormal;

		d6Joint->loadFromDesc(*jointDesc.get());

		deformData->durability -= deformData->bendCosAngle - dot;
	}
}

} // physics
} // frozenbyte
