#ifndef INCLUDED_FB_GAME_CLAW_CONTROLLER_H
#define INCLUDED_FB_GAME_CLAW_CONTROLLER_H

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <datatypedef.h>

class IStorm3D_Model;
class NxActor;

namespace frozenbyte {
namespace physics {

	class PhysicsLib;

} // physics
} // frozenbyte

namespace game {

class Game;
class AbstractPhysicsObject;

struct ControlSettings
{
	float padDeltaPower;
	float padDeltaScale;
	float padDeltaTurboFactor;
	float padTriggerMovementFactor;

	float mouseDeltaScale;
	int mouseDeltaLimit;

	float animationSwitchFarPoseRange;
	float animationSwitchNearPoseRange;

	bool drawHelperCursor;

	ControlSettings()
	:	padDeltaPower(0),
		padDeltaScale(0),
		padDeltaTurboFactor(0),
		padTriggerMovementFactor(0),

		mouseDeltaScale(0.025f),
		mouseDeltaLimit(5),

		animationSwitchFarPoseRange(0),
		animationSwitchNearPoseRange(0),
		drawHelperCursor(false)
	{
	}
};

class ClawController
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	ClawController();
	~ClawController();

	void setGame(Game *game);
	void createPhysics(frozenbyte::physics::PhysicsLib *physicsLib);
	void setModel(IStorm3D_Model *model);
	void setSpawnPosition(const VC3 &pos);
	void update();
	void syncClaw();
	void reloadConfig();

	// Temp controlling
	void removeActor(NxActor *actor);
	bool hasActor() const;
	void setActor( NxActor * ); 
	void pickActor();
	void dropActor();
	void setGrabFlag(bool value);
	void setClawEnabled(bool value);
	void setIgnoreActor(NxActor *actor);
	void setSprinting(bool enable);
	int getTerrainObjectInstanceId() const;
	int getTerrainObjectModelId() const;
	float getActorMass() const;

	enum TurboAction
	{
		TurboHit,
		TurboThrow
	};

	void setAction(TurboAction action);

	enum Action
	{
		ActionNone,
		ActionSwing,
		ActionThrow
	};

	void prepareAction(Action action);
	Action getPrepareAction() const;

	void setFlexPosition(const VC3 &point);
	void setRestPosition(const VC3 &point);
	void moveTowards(const VC3 &point);
	void moveByDelta(const VC2 &delta, float targetHeight);
	void clampTargetClawDistance();
	void setTargetPosition(const VC3 &position);
	VC3 getClawPosition() const;
	VC3 getClawVelocity() const;
	VC3 getTargetPosition(int advanceTicks = 0) const;
	VC3 getDirection() const;
	VC3 getRootPosition() const;
	VC3 getRestPosition() const;
	VC3 getModelPosition() const;
	void moveToRestPosition();

	const ControlSettings &getControlSettings() const;
	bool isClawMoving() const;

	float getClawLength() const;

	bool isClawObject(int modelId, int objectId) const;
	void setNewClawObject(boost::shared_ptr<game::AbstractPhysicsObject> physicsObject);

	unsigned int getNumClawPositions() const;
	VC3 getLastClawPosition(unsigned int i) const;
	float getLastClawGrabMass(unsigned int i) const;

	void setThrowBetaAngle(float angle);
	float getThrowBetaAngle() const;
	void getThrowBetaAngleLimits(float &minAngle, float &maxAngle) const;
};

} // game
//} // frozenbyte

#endif
