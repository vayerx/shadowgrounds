#include "precompiled.h"
#include "ClawController.h"
#include "../physics/box_actor.h"
#include "../physics/spherical_joint.h"
#include "../physics/physics_lib.h"
#include "../physics/actor_base.h"
#include "unified_handle_type.h"
#include "physics/physics_collisiongroups.h"
#include "physics/PhysicsContactDamageManager.h"
#include "physics/PhysicsContactUtils.h"
#include "physics/GamePhysics.h"
#include "physics/AbstractPhysicsObject.h"
#include "Game.h"
#include "scripting/GameScripting.h"
#include "GameUI.h"
#include "Unit.h"
#include "UnitLevelAI.h"
#include <istorm3d_model.h>
#include <istorm3d_bone.h>
#include <istorm3d_scene.h>
#include <boost/lexical_cast.hpp>
#include "../util/SoundMaterialParser.h"
#include "../ui/AnimationSet.h"

#include "../editor/parser.h"
#include "../editor/string_conversions.h"
#include "../filesystem/input_file_stream.h"

#include "NxPhysics.h"

#include <sstream>

using namespace frozenbyte::editor;

//namespace frozenbyte {
namespace game {

	extern PhysicsContactDamageManager *gameui_physicsDamageManager;

namespace {
/*
	const int ACTION_PREPARE_TICKS = 31;
	const float ACTION_PREPARE_SPEED = 0.4f;
	const int ACTION_TICKS = 15;
	const float ACTION_HIT_SPEED = 0.6f;

	const int TURBO_COOLDOWN_TICKS = 67;
	const int TURBO_TICKS = 67;
	const float TURBO_CONTROLLER_FACTOR = 4.f;
	const float TURBO_MAXSPEED_FACTOR = 2.0f;

	const int REST_POSE_TICKS = 67 * 2;
	const int AUTO_GRAB_TICKS = 67;
	const float AUTO_GRAB_SPEED = 0.3f;
*/
	const float EXTRA_GRAB_DISTANCE = 0.5f;
	const float GRAB_REQUIRED_DOT = 0.95f;

	const float STATIC_OBJECT_MASS = 150.0f;

	VC3 toVC3(const NxVec3 &vec)
	{
		return VC3(vec.x, vec.y, vec.z);
	}

	NxVec3 fromVC3(const VC3 &vec)
	{
		return NxVec3(vec.x, vec.y, vec.z);
	}

	static bool isStreetLamp(Game *game, int id)
	{
		if(id == -1)
			return false;

		static int streetLampId[4] = { -1, -1, -1, -1 };
		static const char *streetLampNames[4] =
		{
			"Data\\Models\\Terrain_objects\\Lamps\\StreetLamp_01\\StreetLamp_01.s3d",
			"Data\\Models\\Terrain_objects\\Lamps\\StreetLamp_01\\StreetLamp_01Physic.s3d",
			"Data\\Models\\Terrain_objects\\Lamps\\Trafficlight_01\\trafficlight.s3d",
			"Data\\Models\\Terrain_objects\\Lamps\\Trafficlight_01\\trafficlight_Broken_01.s3d",
		};

		for(int i = 0; i < 4; i++)
		{
			if(streetLampId[i] == - 1)
				streetLampId[i] = game->gameUI->getTerrain()->getModelIdForFilename(streetLampNames[i]);

			if(id == streetLampId[i])
				return true;
		}

		return false;
	}

	struct ActorData
	{
		boost::shared_ptr<frozenbyte::physics::ActorBase> actor;
		IStorm3D_Bone *bone;

		// Filtering info etc
		VC3 position;
		QUAT rotation;

		explicit ActorData(boost::shared_ptr<frozenbyte::physics::ActorBase> &ptr)
		:	actor(ptr),
			bone(0)
		{
		}
	};

	typedef std::vector<ActorData> ActorList;
	typedef std::vector<boost::shared_ptr<frozenbyte::physics::SphericalJoint> > JointList;

	struct GrabInfo
	{
		NxActor *actor;
		NxJoint *joint;
		Unit *unit;
		float actorMass;
		float unitMass;

		int terrObjModelId;
		int terrObjInstanceId;
		
		float extraRange;
		VC3 clawSpaceDifference;

		bool streetLamp;
		int streetLampNoMoveDelay;

		/*
		QUAT dirTm;
		VC3 dir;
		VC3 up;
		*/

		GrabInfo()
		:	actor(0),
			joint(0),
			unit(0),
			actorMass(0),
			unitMass(1.f),
			extraRange(0.f),
			terrObjModelId(0),
			terrObjInstanceId(0),
			streetLamp(false),
			streetLampNoMoveDelay(0)
		{
		}

		void clear()
		{
			actor = 0;
			joint = 0;
			unit = 0;
			actorMass = 0;
			extraRange = 0.f;
			clawSpaceDifference = VC3();
			streetLamp = false;
			streetLampNoMoveDelay = 0;
		}
	};

	struct CollisionFadeInfo
	{
		NxActor *actor;
		int updates;

		CollisionFadeInfo()
		:	actor(0),
			updates(20)
		{
		}
	};

	struct TargetInfo
	{
		NxShape *shape;
		NxActor *actor;
		VC3 dir;
		VC3 up;

		bool streetLamp;

		TargetInfo()
		:	shape(0),
			actor(0),
			streetLamp(false)
		{
		}

		bool hasTarget() const
		{
			if(shape && actor)
				return true;

			return false;
		}

		void clear()
		{
			shape = 0;
			actor = 0;
			streetLamp = false;
		}

		void setDir(const VC3 &dir_, const VC3 &clawDir)
		{
			dir = dir_;
			up = VC3(0, 1.f, 0);

			/*
			VC3 side = clawDir.GetCrossWith(dir);
			if(side.GetLength() > 0.01f)
			{
				side.Normalize();
				VC3 newUp = dir.GetCrossWith(side);
				if(newUp.GetLength() > 0.01f)
				{
					newUp.Normalize();
					up = newUp;
				}
			}
			*/
			if(dir.y < -0.6f)
				up = clawDir;

			/*
			VC3 dir2 = dir;
			dir2.y = 0.f;
			if(dir2.GetLength

			VC3 side = dir2.GetCrossWith(dir);
			if(side.GetLength() > 0.0001f)
			{
				side.Normalize();
				up = dir.GetCrossWith(side);
				if(up.GetLength() > 0.00001f)
					up.Normalize();
			}
			*/
		}
	};

	struct ClawSettings
	{
		float clawLength;
		std::string tailHelper;
		int boxAmount;
		VC3 boxSize;
		float boxDamping;
		float boxMass;
		float clawMass;
		bool clawCenterForceEnabled;
		bool clawRotateAwayEnabled;
		float velocityScaleFactor;
		float maxVelocityFactor;
		float clawMaxDistanceOriginal;
		float clawMaxDistance;
		float heightWithoutActor;
		float heightWithActor;
		float bodyForceFactor;
		float bodyUpForce;

		float restPoseReturnMethod;
		float restPoseScaleFactor;
		float restPoseLinearSpeed;

		float maxThrowAngleAdjust;
		float maxAutoAimAdjust;
		float maxTurboAutoAimAdjust;
		float maxAimedThrowAutoAimAdjust;
		float maxAimedThrowBetaAngle;
		float minAimedThrowBetaAngle;
		float aimedThrowBetaAngleForceScale;

		int actionPrepareTicks;
		float actionPrepareSpeed;
		int actionTicks;
		float actionHitSpeed;
		int turboCooldownTicks;
		int turboTicks;
		float turboControllerFactor;
		float turboMaxspeedFactor;
		int restPoseTicks;
		int autoGrabTicks;
		float autoGrabSpeed;

		float preferObjectsFromClawDirectionNonlinearSpread;
		float preferObjectsFromClawDirectionDistanceFactor;
		float preferObjectsWithMassLimit;
		float preferObjectsWithMassFactor;

		ClawSettings()
		:	clawLength(0),
			boxAmount(0),
			boxDamping(0),
			boxMass(0),
			clawMass(0),
			clawCenterForceEnabled(false),
			clawRotateAwayEnabled(false),
			velocityScaleFactor(0),
			maxVelocityFactor(0),
			clawMaxDistanceOriginal(0),
			clawMaxDistance(0),
			heightWithoutActor(0),
			heightWithActor(0),
			bodyForceFactor(0),
			bodyUpForce(0),

			restPoseReturnMethod(0.5f),
			restPoseScaleFactor(0),
			restPoseLinearSpeed(0),

			maxThrowAngleAdjust(0),
			maxAutoAimAdjust(0),
			maxTurboAutoAimAdjust(0),
			maxAimedThrowAutoAimAdjust(0),

			actionPrepareTicks(0),
			actionPrepareSpeed(0),
			actionTicks(0),
			actionHitSpeed(0),
			turboCooldownTicks(0),
			turboTicks(0),
			turboControllerFactor(0),
			turboMaxspeedFactor(0),
			restPoseTicks(0),
			autoGrabTicks(0),
			autoGrabSpeed(0),

			preferObjectsFromClawDirectionNonlinearSpread(0),
			preferObjectsFromClawDirectionDistanceFactor(0),
			preferObjectsWithMassLimit(0),
			preferObjectsWithMassFactor(0)
		{
		}
	};


	struct MassEntry
	{
		std::string key;
		float mass;
		float minimumSpeedTowardsController;
		float liftSpeed;

		float accelerationDecayLinear;
		float accelerationDecayFactor;
		float accelerationDecayNonlinearFactor;
		float controllerAlignedAccelerationDecayLinear;
		float controllerAlignedAccelerationDecayFactor;
		float controllerAlignedAccelerationDecayNonlinearFactor;
		float inactiveAccelerationDecayLinear;
		float inactiveAccelerationDecayFactor;
		float inactiveAccelerationDecayNonlinearFactor;

		float accelerationIncreaseLinear;
		float accelerationIncreaseFactor;
		float accelerationIncreaseNonlinearFactor;
		float accelerationDisableLinearIncreaseSpeed;

		float velocityRotationHelperFactor;

		float maxVelocity;
		float dropForce;
		float releaseExtraSpeedFactor;
		bool clawCenterForceEnabled;
		bool clawRotateAwayEnabled;

		MassEntry()
		:	mass(0.f),
			minimumSpeedTowardsController(0),
			liftSpeed(0),
			accelerationDecayLinear(0),
			accelerationDecayFactor(0),
			accelerationDecayNonlinearFactor(0),
			accelerationIncreaseLinear(0),
			accelerationIncreaseFactor(0),
			accelerationIncreaseNonlinearFactor(0),
			accelerationDisableLinearIncreaseSpeed(10000.f),
			inactiveAccelerationDecayLinear(0),
			inactiveAccelerationDecayFactor(0),
			inactiveAccelerationDecayNonlinearFactor(0),
			controllerAlignedAccelerationDecayLinear(0),
			controllerAlignedAccelerationDecayFactor(0),
			controllerAlignedAccelerationDecayNonlinearFactor(0),
			velocityRotationHelperFactor(0),
			maxVelocity(10000.f),
			dropForce(0),
			releaseExtraSpeedFactor(0),
			clawCenterForceEnabled(true),
			clawRotateAwayEnabled(true)
		{
		}
	};

	struct MassEntrySorter
	{
		bool operator () (const MassEntry &a, const MassEntry &b) const
		{
			return a.mass < b.mass;
		}
	};

	struct MassSettings
	{
		std::vector<MassEntry> massEntries;

		const MassEntry &getMassEntry(const GrabInfo &grabInfo) const
		{
			float mass = grabInfo.actorMass;
			if(grabInfo.unit)
				mass = grabInfo.unitMass;

			if(!massEntries.empty())
			{
				for(unsigned int i = 0; i < massEntries.size() - 1; ++i)
				{
					if(mass >= massEntries[i].mass && mass < massEntries[i + 1].mass)
						return massEntries[i];
				}

				if(mass < massEntries[0].mass)
					return massEntries[0];

				return massEntries[massEntries.size() - 1];
			}

			static MassEntry none;
			return none;
		}
	};

	struct DebugLine
	{
		VC3 point;
		VC3 normal;
		NxVec3 dir;
	};

	NxQuat getRotationTowards(const NxVec3 &a, const NxVec3 &b)
	{
		NxVec3 axis = a.cross(b);
		float dot = a.dot(b);

		NxQuat result;
		result.id();

		//if(dot < -0.99999f)
		//	return result;

		result.x = axis.x;
		result.y = axis.y;
		result.z = axis.z;
		result.w = (dot + 1.0f);
		result.normalize();

		return result;
	}

	struct ActiveAction
	{
		ClawController::Action action;
		int strength;
		int time;

		VC3 dir;

		ActiveAction()
		:	action(ClawController::ActionNone),
			strength(100),
			time(0)
		{
		}

		void reset()
		{
			action = ClawController::ActionNone;
			strength = 100;
			time = 0;
		}
	};

} // unnamed

// ToDo: Sync claw to model on load/create

struct ClawController::Data : public NxUserRaycastReport
{
	frozenbyte::physics::PhysicsLib *physicsLib;
	Game *game;
	bool recreatePhysics;
	bool hasBones;

	ActorList actors;
	JointList joints;
	IStorm3D_Bone *clawBone;

	VC3 rootPosition;
	QUAT rootRotation;
	IStorm3D_Model *model;
	VC3 modelPosition;
	VC3 modelDelta;

	VC3 flexPosition;
	VC3 restPosition;
	VC3 targetPosition;
	float targetHeight;
	VC3 force;
	VC2 speedDelta;
	VC2 inputDelta;

	std::vector<VC3> lastPositions;
	std::vector<float> lastMasses;

	enum Mode
	{
		TargetPoint,
		Force
	};

	Mode mode;
	bool toggleGrab;
	bool grabFlag;

	GrabInfo grabInfo;
	TargetInfo targetInfo;

	ClawSettings clawSettings;
	ControlSettings controlSettings;
	MassSettings massSettings;

	mutable DebugLine debugLines[6];
	bool clawEnabled;

	std::vector<CollisionFadeInfo> collisionFades;
	NxActor *ignoreActor;

	bool sprinting;
	int turboCounter;
	VC2 turboVec;
	ClawController::TurboAction turboAction;
	int restPoseCounter;
	int autoGrabCounter;

	ClawController::Action prepareAction;
	int prepareActionCount;
	ActiveAction activeAction;
	int restPoseDelay;
	int jointBreakDelay;

	bool releaseExtraSpeed;
	bool clawDistanceClamp;

	float throwBetaAngle;

	boost::shared_ptr<game::AbstractPhysicsObject> newClawObject;
	int newClawObjectDelay;

	Data()
	:	physicsLib(0),
		game(0),
		recreatePhysics(false),
		hasBones(false),
		clawBone(0),
		model(0),
		targetHeight(0),
		mode(TargetPoint),
		toggleGrab(false),
		grabFlag(false),
		clawEnabled(false),
		ignoreActor(0),
		sprinting(false),
		turboCounter(0),
		turboAction(ClawController::TurboHit),
		restPoseCounter(0),
		autoGrabCounter(0),
		prepareAction(ClawController::ActionNone),
		prepareActionCount(0),
		restPoseDelay(0),
		jointBreakDelay(0),
		releaseExtraSpeed(false),
		clawDistanceClamp(false),
		throwBetaAngle(0.0f),

		newClawObjectDelay(0)
	{
		parseStuff();
	}

	~Data()
	{
		//NxScene *scene = physicsLib->getScene();
		//if(grabInfo.joint)
		//	scene->releaseJoint(*grabInfo.joint);
	}

	void setPhysics(frozenbyte::physics::PhysicsLib *physicsLib_)
	{
		NxScene *scene = 0;
		if(physicsLib)
			scene = physicsLib->getScene();

		physicsLib = physicsLib_;
		if(!physicsLib)
		{
			if(scene && grabInfo.joint)
				scene->releaseJoint(*grabInfo.joint);

			grabInfo.clear();
			actors.clear();
			joints.clear();

			return;
		}

		recreatePhysics = true;
	}

	void createPhysics()
	{
		assert(physicsLib);

		util::SoundMaterialParser materialParser;
		int clawMaterial = materialParser.getMaterialIndexByName("claw");
		int bodyMaterial = materialParser.getMaterialIndexByName("claw_body");
		
		lastPositions.resize(32, rootPosition);
		lastMasses.resize(32, 0.0f);

		const float HEIGHT = 1.f;
		for(int i = 0; i < clawSettings.boxAmount; ++i)
		{
			//VC3 pos(0, HEIGHT, (i + 1) * clawSettings.boxSize.z * 2.f);
			VC3 pos(0, HEIGHT, i * clawSettings.boxSize.z * 2.f);
			pos += rootPosition;

			VC3 localPos(0, -clawSettings.boxSize.y, clawSettings.boxSize.z);
			boost::shared_ptr<frozenbyte::physics::ActorBase> actor = physicsLib->createBoxActor(clawSettings.boxSize, pos, localPos);
			if(actor)
			{
				actor->enableFeature(frozenbyte::physics::ActorBase::DISABLE_GRAVITY, true);
				actor->setLinearDamping(clawSettings.boxDamping);
				actor->setAngularDamping(0.1f);

				if(i == clawSettings.boxAmount - 1)
					actor->setMass(clawSettings.clawMass);
					//actor->setMass(50.f);
				else
					actor->setMass(clawSettings.boxMass);

				if(i == 0)
				{
					actor->enableFeature(frozenbyte::physics::ActorBase::KINEMATIC_MODE, true);
					actor->setCollisionGroup(PHYSICS_COLLISIONGROUP_NOCOLLISION);
					//actor->setCollisionGroup(PHYSICS_COLLISIONGROUP_CLAW);				
				}
				else
				{
					if(i == clawSettings.boxAmount - 1)
						actor->setIntData(clawMaterial);
					else
						actor->setIntData(bodyMaterial);

					//actor->setCollisionGroup(PHYSICS_COLLISIONGROUP_CLAW);
					actor->setCollisionGroup(PHYSICS_COLLISIONGROUP_NOCOLLISION);
				}

				actors.push_back(ActorData(actor));
			}

			if(i != 0)
			{
				//VC3 anchor(0, HEIGHT, ((i + 1) * 2.f * clawSettings.boxSize.z)/* - BOX_SIZE.z * 2.f*/);
				VC3 anchor(0, HEIGHT, (i * 2.f * clawSettings.boxSize.z)/* - BOX_SIZE.z * 2.f*/);
				anchor += rootPosition;

/*				if(i == BOX_AMOUNT - 1)
				{
					boost::shared_ptr<frozenbyte::physics::SphericalJoint> joint = physicsLib->createSphericalJoint(actors[1].actor, actors[i].actor, anchor);
					joints.push_back(joint);
				}
				else
*/				{
					boost::shared_ptr<frozenbyte::physics::SphericalJoint> joint = physicsLib->createSphericalJoint(actors[i - 1].actor, actors[i].actor, anchor);
					joints.push_back(joint);
				}
			}
		}
	}

	void setModel(IStorm3D_Model *model_)
	{
		clawBone = 0;
		if(!model_ || actors.size() != clawSettings.boxAmount)
		{
			if(actors.size() == clawSettings.boxAmount)
			{
				for(int i = 1; i < clawSettings.boxAmount; ++i)
					actors[i].bone = 0;
			}

			model = 0;
			return;
		}

		//if(model == model_ && hasBones)
		//	return;

		model = model_;
		hasBones = false;

		for(int i = 1; i < clawSettings.boxAmount; ++i)
		{
			std::string boneName = "tail";
			boneName += boost::lexical_cast<std::string> (i + 1);
			
			IStorm3D_Bone *bone = model->SearchBone(boneName.c_str());
			actors[i].bone = bone;

			if(bone)
				hasBones = true;
		}

		clawBone = model->SearchBone("claw");
	}

	void syncClaw()
	{
		if(!model || actors.size() != clawSettings.boxAmount || !hasBones)
			return;

// HAX HAX VISUALIZATION
/*
{
	IStorm3D_Scene *scene = game->getGameUI()->getStormScene();
	float length = 1.0f;
	for(int i = 0; i < 6; ++i)
	{
		const DebugLine &l = debugLines[i];

		if(i < 5)
			scene->AddLine(l.point, l.point + (l.normal * length), COL(1.f, 1.f, 1.f));
		else
		{
			VC3 x(l.normal.x, 0, 0);
			VC3 z(0, 0, l.normal.x);

			scene->AddLine(l.point - x, l.point + x, COL(0.5f, 0.5f, 0.5f));
			scene->AddLine(l.point - z, l.point + z, COL(0.5f, 0.5f, 0.5f));
		}
	}
}
*/
		MAT tm = model->GetMX();
		tm.Inverse();
		QUAT rtm = tm.GetRotation();

		for(int i = 0; i < clawSettings.boxAmount; ++i)
		{
			ActorData &data = actors[i];
			if(!data.actor ||!data.bone)
				continue;

			VC3 position;
			QUAT rotation;
			data.actor->getPosition(position);
			data.actor->getRotation(rotation);

			if(i == 0)
			{
				position = rootPosition;
				rotation = rootRotation;
			}

			tm.TransformVector(position);
			rotation = rotation * rtm;

			// Filter rotation
			data.rotation = data.rotation.GetSLInterpolationWith(rotation, 0.5f);
			/*
			// Filter position
			data.position *= 0.9f;
			data.position += position * 0.1f;
			*/

			data.bone->SetForceTransformation(true, position, data.rotation);

			if(clawBone && i == clawSettings.boxAmount - 1)
			{
				VC3 dir(0, 0, 1.f);
				data.rotation.RotateVector(dir);
				position += dir * (clawSettings.boxSize.z * 2.f);

				/*
				// Filter position
				data.position *= 0.9f;
				data.position += position * 0.1f;
				*/

				clawBone->SetForceTransformation(true, position, data.rotation);
			}
		}
	}

	void moveClaw()
	{
		VC3 target = targetPosition;
		//if(prepareAction != ClawController::ActionNone)
		//	target = flexPosition;

		const MassEntry &massEntry = massSettings.getMassEntry(grabInfo);
		boost::shared_ptr<frozenbyte::physics::ActorBase> claw = actors[actors.size() - 1].actor;
		VC3 actorPos;
		{
			if(massEntry.clawCenterForceEnabled)
				claw->getMassCenterPosition(actorPos);
			else
			{
				VC3 pos;
				QUAT rot;
				claw->getPosition(pos);
				claw->getRotation(rot);

				MAT tm;
				tm.CreateRotationMatrix(rot);
				tm.Set(12, pos.x);
				tm.Set(13, pos.y);
				tm.Set(14, pos.z);

				actorPos.x = 0.f;
				actorPos.y = 0.f;
				actorPos.z = 2.f * clawSettings.boxSize.z;
				tm.TransformVector(actorPos);
			}
/*
if(model)
{
	IStorm3D_Helper *helper = model->SearchHelper("HELPER_BONE_Claw");
	if(helper)
		actorPos = helper->GetGlobalPosition();
}
*/

			// Override pos with actor
			actorPos = getClawTargetPosition();
		}

		//if(mode == TargetPoint)
		{
			VC3 targetForce = target - actorPos;
/*
//float targetForceY = targetForce.y;
//targetForce.y = 0;
			targetForce *= clawSettings.velocityScaleFactor;
			VC3 unscaledTargetForce = targetForce;

			VC3 currentVelocity;
			claw->getVelocity(currentVelocity);
//float currentVelocityY = currentVelocity.y;
//currentVelocity.y = 0.f;

			VC3 force = currentVelocity + targetForce;
			if(force.GetLength() > unscaledTargetForce.GetLength() * clawSettings.maxVelocityFactor)
			{
				force.Normalize();
				force *= unscaledTargetForce.GetLength() * clawSettings.maxVelocityFactor;
			}

			force -= currentVelocity;
			claw->addVelocityChange(actorPos, force);

//targetForceY *= 60.f;
//targetForceY -= currentVelocity.y;
//claw->addVelocityChange(actorPos, VC3(0, targetForceY, 0));
*/
VC3 force = targetForce * clawSettings.velocityScaleFactor;

// street lamp hack: when static turns into dynamic, the target position is completely wrong
if(grabInfo.streetLampNoMoveDelay > 0 && grabInfo.actor->isDynamic())
{
	// clamp massive forces
	float len = force.GetLength();
	if(len > 4.0f)
	{
		force *= 4.0f / len;
	}
	// stupid hack to fix target position
	targetPosition = actorPos;
	grabInfo.streetLampNoMoveDelay--;
}

VC3 clawVelocity;
claw->getVelocity(clawVelocity);
//VC3 forceDiff = force - clawVelocity;
claw->setVelocity(force);


if(grabInfo.actor)
{
	if(grabInfo.actor->isDynamic())
	{
		if(grabInfo.streetLamp)
		{
			if(force.y > 3.0f)
				force.y = 3.0f;
			else if(force.y < -3.0f)
				force.y = -3.0f;
		}
		grabInfo.actor->setLinearVelocity(fromVC3(force));
	}
	
	// hack: give damage directly to static objects	
	if(!grabInfo.actor->isDynamic() && force.GetSquareLength() > 1.0)
	{
		frozenbyte::physics::ActorBase *actorBase = static_cast<frozenbyte::physics::ActorBase *> (grabInfo.actor->userData);
		if(actorBase && actorBase->getUserData() && game->getGamePhysics())
		{
			AbstractPhysicsObject *object = static_cast<AbstractPhysicsObject *> (game->getGamePhysics()->getInterfaceObjectForHandle(reinterpret_cast<int> (actorBase->getUserData())));
			if(object && gameui_physicsDamageManager)
			{
				PhysicsContact contact;
				contact.obj1 = object;
				contact.obj2 = NULL;
				contact.physicsObject1 = actorBase;
				contact.physicsObject2 = NULL;
				contact.contactForceLen = force.GetSquareLength() * 50.0f;
				contact.contactNormal = force.GetNormalized();
				contact.contactPosition = toVC3(getActorCMassGlobalPosition(grabInfo.actor));
				gameui_physicsDamageManager->physicsContact(contact);
			}
		}
	}
}


			/*
			if(grabInfo.actor)
				grabInfo.actor->setLinearVelocity(fromVC3(force));
				//grabInfo.actor->addForce(fromVC3(force), NX_VELOCITY_CHANGE);
			*/

			if(massEntry.clawRotateAwayEnabled && model)
			{
				QUAT currentRotation;
				claw->getRotation(currentRotation);
/*
// Rotation test
if(grabInfo.actor)
{
	NxQuat nxquat = grabInfo.actor->getGlobalOrientationQuat();

	currentRotation.x = nxquat.x;
	currentRotation.y = nxquat.y;
	currentRotation.z = nxquat.z;
	currentRotation.w = nxquat.w;
	currentRotation.Inverse();
}
*/
				MAT currentTm;
				currentTm.CreateRotationMatrix(currentRotation);

				//const float TORQUE_SCALE = 10.f;
				const float TORQUE_SCALE = 20.f;
				VC3 finalTorque;
				VC3 targetDir;

				// Rotation around y
				{
					VC3 currentDir(currentTm.Get(8), currentTm.Get(9), currentTm.Get(10));
					currentDir.Normalize();

					targetDir = targetPosition - model->GetPosition();
					if(prepareAction != ClawController::ActionNone || activeAction.action != ClawController::ActionNone)
						targetDir = restPosition - model->GetPosition();

					targetDir.y = 0.f;
					if(targetDir.GetSquareLength() > 0.001f)
						targetDir.Normalize();
					else
						targetDir = VC3(0, 0, 1.f);

// Rotation test
//if(grabInfo.actor)
//	grabInfo.dirTm.RotateVector(targetDir);
//targetDir = grabInfo.dir;
					if(targetInfo.hasTarget())
						targetDir = targetInfo.dir;

					VC3 torque = currentDir.GetCrossWith(targetDir);
					if(torque.GetSquareLength() > 0.0001f)
					{
						torque.Normalize();
						torque *= acosf(currentDir.GetDotWith(targetDir));
						torque *= TORQUE_SCALE;
						finalTorque += torque;
					}
				}

				// Rotation around z
				{
					VC3 currentUp(currentTm.Get(4), currentTm.Get(5), currentTm.Get(6));
					currentUp.Normalize();
					VC3 targetUp = VC3(0, 1.f, 0);

//if(grabInfo.actor)
//	targetUp = grabInfo.up;

					if(targetInfo.hasTarget())
						targetUp = targetInfo.up;

					VC3 torque = currentUp.GetCrossWith(targetUp);
					if(torque.GetSquareLength() > 0.0001f)
					{
						torque.Normalize();
						torque *= acosf(currentUp.GetDotWith(targetUp));
						torque *= TORQUE_SCALE;
						finalTorque += torque;
					}
				}

				float len = finalTorque.GetLength();
				if(!(len >= 0 && len < 1000.f))
				{
					assert(!"NAN's on claw angular velocity");
				}

				VC3 angularVelocity;
				claw->getAngularVelocity(angularVelocity);


				// rotate lamp hack
				/*if(grabInfo.actor && grabInfo.actor->isDynamic() && grabInfo.streetLamp && grabInfo.joint)
				{
					NxQuat rot;
					grabInfo.actor->getGlobalPose().M.toQuat(rot);
					NxVec3 upVector = NxVec3(0, 1.0f, 0);
					rot.rotate(upVector);

					float dot = upVector.dot(NxVec3(0, 1.0f, 0));
					//if(dot > 0.1f)
					{
						NxVec3 torq(0, 0, 0.125f);
						rot.rotate(torq);

						finalTorque += toVC3(torq);
					}
				}*/

				claw->setAngularVelocity(finalTorque);
				if(grabInfo.actor)
				{
					grabInfo.actor->setAngularVelocity(fromVC3(finalTorque));
				}
			}

			/*
			if(massEntry.clawRotateAwayEnabled)
			{
				//force *= clawSettings.bodyForceFactor;
				//force.y += clawSettings.bodyUpForce;

				int actorStart = 1;
				int actorEnd = actors.size() - 1;
				int actorQ = (actorEnd - actorStart) / 4;
				//for(unsigned int i = 1; i < actors.size() - 1; ++i)
				for(unsigned int i = actorStart + actorQ; i < actorEnd - actorQ; ++i)
				{
					VC3 actorVelocity;
					actors[i].actor->getVelocity(actorVelocity);

					VC3 diff = force - actorVelocity;
					if(!clawDistanceClamp)
						diff *= clawSettings.bodyForceFactor;
					else
						diff *= 0.f;

					diff.y += clawSettings.bodyUpForce;
					actors[i].actor->addVelocityChange(diff);

					//actors[i].actor->addVelocityChange(force);
					//actors[i].actor->setVelocity(force);
				}
			}
			*/
		}
	}

	void updateGrab()
	{
		// Force grabbing if we already have a target
		if(autoGrabCounter > 0 && targetInfo.hasTarget())
			toggleGrab = true;

		if(!toggleGrab || !physicsLib || !model)
			return;
		toggleGrab = false;

		// Claw dir
		QUAT clawQuat;
		actors[actors.size() - 1].actor->getRotation(clawQuat);
		VC3 clawDir(0, 0, 1.f);
		clawQuat.RotateVector(clawDir);

		VC3 clawTargetDir = targetPosition - model->GetPosition();
		clawTargetDir.y = 0.f;
		if(clawTargetDir.GetLength() > 0.0001f)
			clawTargetDir.Normalize();

		NxScene *scene = physicsLib->getScene();

		if(!grabInfo.actor)
		{
			jointBreakDelay = 0;
			VC3 grabPos;
			{
				IStorm3D_Helper *helper = model->SearchHelper("HELPER_BONE_Claw");
				if(helper)
					grabPos = helper->GetGlobalPosition();
			}

			//boost::shared_ptr<frozenbyte::physics::ActorBase> claw = actors[actors.size() - 1].actor;
			//claw->getMassCenterPosition(grabPos);
			grabPos = getClawTargetPosition();

			if(!targetInfo.hasTarget()) // Auto aiming
			{
				NxPlane plane[5];
				const int MAX_ACTORS = 10;
				NxShape *shapes[MAX_ACTORS * 2] = { 0 };
				int activeAmount = 0;

				// Hack
				VC3 cullPosition = model->GetPosition();
				cullPosition.y = grabPos.y;

				// Find all shapes based on frustum/sphere around claw
				{
					NxQuat quat = actors[clawSettings.boxAmount - 1].actor->getActor()->getGlobalOrientationQuat();
					NxVec3 dir(0, 0, 1.f);
					quat.rotate(dir);

					dir.y = -1.f;
					dir.normalize();
					float angle = 45.f;
					float coneRange = clawSettings.clawMaxDistance + EXTRA_GRAB_DISTANCE;
					//activeAmount = getFrustumShapes(fromVC3(grabPos), dir, 45.f, coneRange, shapes, MAX_ACTORS);
					activeAmount = getFrustumShapes(fromVC3(cullPosition), dir, 45.f, coneRange, shapes, MAX_ACTORS);

					// Also add shapes from sphere around claw -- should have to be moved further from the claw?
					float sphereRange = 1.f;
					NxSphere worldSphere(NxVec3(grabPos.x,grabPos.y,grabPos.z), sphereRange);
					activeAmount += scene->overlapSphereShapes(worldSphere, NX_ALL_SHAPES, MAX_ACTORS, shapes + activeAmount, NULL, 0xffffffff, 0, true);
				}

				float targetValue = 999999999.f;
				for(int i = 0; i < activeAmount; ++i)
				{
					if(!shapes[i])
						continue;

					NxActor *actor = 0;

					Unit *unit = NULL;
					int terrObjModelId = -1;
					int terrObjInstanceId = -1;
					AbstractPhysicsObject *object = 0;
					bool streetLamp = false;

					// Discard based on various properties
					{ 
						int collisionGroup = shapes[i]->getGroup();
						if(collisionGroup == PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_WO_UNIT_COLL_NO_SOUND)
							continue;
						if(collisionGroup == PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_NO_SOUND)
							continue;
						if(collisionGroup == PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_WO_UNIT_COLL)
							continue;
						if(collisionGroup == PHYSICS_COLLISIONGROUP_MODEL_PARTICLES)
							continue;

						actor = &shapes[i]->getActor();
						if(!actor)
							continue;
						if(actor == ignoreActor)
							continue;

						// determine unit/object
						//
						frozenbyte::physics::ActorBase *actorBase = static_cast<frozenbyte::physics::ActorBase *> (actor->userData);
						if(actorBase && actorBase->getUserData() && game->getGamePhysics())
						{
							object = static_cast<AbstractPhysicsObject *> (game->getGamePhysics()->getInterfaceObjectForHandle(reinterpret_cast<int> (actorBase->getUserData())));
							if(object)
							{
								PhysicsContactUtils::mapPhysicsObjectToUnitOrTerrainObject(game, object, &unit, &terrObjModelId, &terrObjInstanceId);
							}
						}

						// determine if it's a streetlamp
						if(isStreetLamp(game, terrObjModelId))
						{
							streetLamp = true;
						}


						if(!actor->isDynamic())
						{
							// hack: allow grabbing static streetlamp
							if(!streetLamp)
							{
								continue;
							}
						}

						// hack: ignore unconscious units !!
						//
						if(unit && unit->getMoveState() == Unit::UNIT_MOVE_STATE_UNCONSCIOUS)
						{
							continue;
						}

						bool found = false;
						for(unsigned int j = 0; j < actors.size(); ++j)
						{
							if(actor == actors[j].actor.get()->getActor())
								found = true;
						}

						if(found)
							continue;
					}

					// Ray straight ahead
					NxRay rayClawDirection;
					const float rayClawDirectionFudge = 2.5f;
					rayClawDirection.orig = fromVC3(grabPos);
					rayClawDirection.dir = fromVC3(clawDir);
					rayClawDirection.orig -= rayClawDirection.dir * rayClawDirectionFudge;
					NxRaycastHit hitInfo;
					
					/*
					if(shapes[i]->raycast(rayClawDirection, clawSettings.clawMaxDistance + rayClawDirectionFudge + EXTRA_GRAB_DISTANCE, NX_RAYCAST_FACE_NORMAL | NX_RAYCAST_IMPACT | NX_RAYCAST_DISTANCE, hitInfo, false))
					{
						float distance = model->GetPosition().GetRangeTo(toVC3(hitInfo.worldImpact));
						if(distance < clawSettings.clawMaxDistance + EXTRA_GRAB_DISTANCE)
						{
							float clawDistance = grabPos.GetRangeTo(toVC3(hitInfo.worldImpact));

							// Prefer heavier objects
							{
								float mass = shapes[i]->getActor().getMass();

								// HACK: Units (humans, for the time being) are actually heavier than 1kg :)
								{
									NxActor &actor = shapes[i]->getActor();

									AbstractPhysicsObject *object = 0;
									frozenbyte::physics::ActorBase *actorBase = static_cast<frozenbyte::physics::ActorBase *> (actor.userData);
									if(actorBase && actorBase->getUserData() && game->getGamePhysics())
									{
										object = static_cast<AbstractPhysicsObject *> (game->getGamePhysics()->getInterfaceObjectForHandle(reinterpret_cast<int> (actorBase->getUserData())));
										if(object)
										{
											Unit *unit = 0;
											int foo1 = 0;
											int foo2 = 0;
											PhysicsContactUtils::mapPhysicsObjectToUnitOrTerrainObject(game, object, &unit, &foo1, &foo2);

											if(unit)
											{
												// Hax hax
												int unitOwner = unit->getOwner();
												if(game->isHostile(0, unitOwner))
												{
													mass = grabInfo.unitMass;
												}
											}
										}
									}
								}

								if(mass < clawSettings.preferObjectsWithMassLimit)
								{
									clawDistance += clawDistance * clawSettings.preferObjectsWithMassFactor;
								}
							}

							if(clawDistance < targetValue)
							{
								targetValue = clawDistance;
								targetInfo.shape = shapes[i];
								targetInfo.actor = actor;
								targetInfo.setDir(-toVC3(hitInfo.worldNormal), clawTargetDir);
							}
						}
					}
					*/

					// Ray towards object -> should really point to closest point of shape instead of mass center
					NxRay rayShapeDirection;
					float rayShapeDirectionFudge = 2.5f;
					rayShapeDirection.orig = fromVC3(grabPos);
					rayShapeDirection.dir = getActorCMassGlobalPosition(actor);
					rayShapeDirection.dir -= fromVC3(grabPos);
					rayShapeDirection.dir.normalize();
					rayShapeDirection.orig -= rayShapeDirection.dir * rayShapeDirectionFudge;

					hitInfo = NxRaycastHit();
					if(shapes[i]->raycast(rayShapeDirection, clawSettings.clawMaxDistance + rayShapeDirectionFudge + EXTRA_GRAB_DISTANCE, NX_RAYCAST_FACE_NORMAL | NX_RAYCAST_IMPACT | NX_RAYCAST_DISTANCE, hitInfo, false))
					{
						float distance = model->GetPosition().GetRangeTo(toVC3(hitInfo.worldImpact));
						if(distance < clawSettings.clawMaxDistance + EXTRA_GRAB_DISTANCE)
						{
							//float clawDistance = grabPos.GetRangeTo(toVC3(hitInfo.worldImpact));
							// Prefer previous objects directly from claw direction
							//clawDistance *= 1.5f;

							VC3 distanceVec = toVC3(hitInfo.worldImpact) - grabPos;
							float clawDistance = distanceVec.GetLength();
							float originalClawDistance = clawDistance;
							if(clawDistance > 0.001f)
								distanceVec /= clawDistance;

							// Prefer objects from claw direction
							{
								VC3 distanceVec2(distanceVec.x, 0.f, distanceVec.z);
								if(distanceVec2.GetSquareLength() > 0.001f)
									distanceVec2.Normalize();
								VC3 clawDir2(clawDir.x, 0, clawDir.z);
								if(clawDir2.GetSquareLength() > 0.001f)
									clawDir2.Normalize();

								//float dot = distanceVec.GetDotWith(clawDir);
								float dot = distanceVec2.GetDotWith(clawDir2);

								if(dot < 0)
									dot = 0.f;
								
								float addFactor = (1.f - powf(dot, clawSettings.preferObjectsFromClawDirectionNonlinearSpread)) * clawSettings.preferObjectsFromClawDirectionDistanceFactor;
								clawDistance += addFactor * originalClawDistance;
							}

							// Prefer heavier objects
							{
								float mass = shapes[i]->getActor().getMass();

								// hack for static
								if(!shapes[i]->getActor().isDynamic())
									mass = STATIC_OBJECT_MASS;

								// HACK: Units (humans, for the time being) are actually heavier than 1kg :)
								{
									NxActor &actor = shapes[i]->getActor();

									AbstractPhysicsObject *object = 0;
									frozenbyte::physics::ActorBase *actorBase = static_cast<frozenbyte::physics::ActorBase *> (actor.userData);
									if(actorBase && actorBase->getUserData() && game->getGamePhysics())
									{
										object = static_cast<AbstractPhysicsObject *> (game->getGamePhysics()->getInterfaceObjectForHandle(reinterpret_cast<int> (actorBase->getUserData())));
										if(object)
										{
											Unit *unit = 0;
											int foo1 = 0;
											int foo2 = 0;
											PhysicsContactUtils::mapPhysicsObjectToUnitOrTerrainObject(game, object, &unit, &foo1, &foo2);

											if(unit)
											{
												// Hax hax
												int unitOwner = unit->getOwner();
												if(game->isHostile(0, unitOwner))
												{
													mass = grabInfo.unitMass;
												}
											}
										}
									}
								}

								if(mass < clawSettings.preferObjectsWithMassLimit)
								{
									clawDistance += originalClawDistance * clawSettings.preferObjectsWithMassFactor;

									//float massFactor = 50.f / mass / 10.f;
									//clawDistance += massFactor * originalClawDistance;
								}
							}

							if(clawDistance < targetValue)
							{
								targetValue = clawDistance;
								targetInfo.shape = shapes[i];
								targetInfo.actor = actor;
								targetInfo.setDir(-toVC3(hitInfo.worldNormal), clawTargetDir);
								targetInfo.streetLamp = streetLamp;
							}
						}
					}
				}
			}

			// Can we pick up the object?
			if(targetInfo.hasTarget())
			{
				const float rayBehind = 0.2f;
				const float rayForward = EXTRA_GRAB_DISTANCE;
				// Ray straight ahead
				NxRay rayClawDirection;
				rayClawDirection.orig = fromVC3(grabPos);
				rayClawDirection.dir = fromVC3(clawDir);
				rayClawDirection.orig -= rayClawDirection.dir * rayBehind;

				bool allowGrab = true;
				bool forceGrab = false;


				if(targetInfo.streetLamp)
				{
					// street lamp middle aim hack
					VC3 target = toVC3(targetInfo.actor->getGlobalPose() * NxVec3(0, 2.0f, 0));
					VC3 dirVec(model->GetPosition().x - target.x, 0, model->GetPosition().z - target.z);
					dirVec.Normalize();
					target += dirVec * 0.5f;

					if((getClawTargetPosition() - target).GetSquareLength() < 0.1f)
					{
						forceGrab = true;
					}
					else
					{
						allowGrab = false;
					}
				}

				NxRaycastHit hitInfo;
				if(allowGrab && (forceGrab || targetInfo.shape->raycast(rayClawDirection, rayBehind + rayForward, NX_RAYCAST_FACE_NORMAL | NX_RAYCAST_IMPACT | NX_RAYCAST_DISTANCE, hitInfo, false)))
				{
					if(allowGrab && (forceGrab || hitInfo.worldNormal.dot(rayClawDirection.dir) < -GRAB_REQUIRED_DOT))
					{
						NxSphericalJointDesc jointDesc;
						jointDesc.setToDefault();
						jointDesc.actor[0] = actors[clawSettings.boxAmount - 1].actor.get()->getActor();
						if(targetInfo.actor->isDynamic())
							jointDesc.actor[1] = targetInfo.actor;
						else
							jointDesc.actor[1] = NULL;

						// Set joint axis
						{
							jointDesc.setGlobalAnchor(fromVC3(grabPos));
							jointDesc.setGlobalAxis(rayClawDirection.dir);
						}

						grabInfo.actor = targetInfo.actor;
						if(game)
						{
							AbstractPhysicsObject *object = 0;
							frozenbyte::physics::ActorBase *actorBase = static_cast<frozenbyte::physics::ActorBase *> (targetInfo.actor->userData);
							if(actorBase && actorBase->getUserData() && game->getGamePhysics())
								object = static_cast<AbstractPhysicsObject *> (game->getGamePhysics()->getInterfaceObjectForHandle(reinterpret_cast<int> (actorBase->getUserData())));

							//int a_terrObjModelId = 0;
							//int a_terrObjInstanceId = 0;
							if (object != 0)
							{
								object->setThrownByClaw(true);
								PhysicsContactUtils::mapPhysicsObjectToUnitOrTerrainObject(game, object, &grabInfo.unit, &grabInfo.terrObjModelId, &grabInfo.terrObjInstanceId);

								// determine if it's a streetlamp
								grabInfo.streetLamp = isStreetLamp(game, grabInfo.terrObjModelId);
							}
							

						}

						if(grabInfo.unit)
						{
							jointDesc.localAnchor[0] = NxVec3(0, 0, clawSettings.boxSize.z);
							jointDesc.localAnchor[1] = NxVec3(0, 1.3f, -0.4f);
							jointDesc.localAxis[1] = NxVec3(0, 0, 1.f);
							jointDesc.localNormal[1] = NxVec3(0, 1.f, 0);

							// Angle for unit
							NxQuat qHax1;
							qHax1.fromAngleAxis(30.f, NxVec3(1.f, 0, 0));
							qHax1.rotate(jointDesc.localAxis[1]);
							qHax1.rotate(jointDesc.localNormal[1]);

							// Angle for claw
							NxQuat qHax2;
							qHax2.fromAngleAxis(60.f, NxVec3(0, 0, 1.f));
							qHax2.rotate(jointDesc.localAxis[0]);
							qHax2.rotate(jointDesc.localNormal[0]);

							scene->setActorPairFlags(*actors[clawSettings.boxAmount - 1].actor->getActor(), *jointDesc.actor[1], NX_IGNORE_PAIR);
							scene->setActorPairFlags(*actors[clawSettings.boxAmount - 2].actor->getActor(), *jointDesc.actor[1], NX_IGNORE_PAIR);
							scene->setActorPairFlags(*actors[clawSettings.boxAmount - 3].actor->getActor(), *jointDesc.actor[1], NX_IGNORE_PAIR);
						}

						jointDesc.swingLimit.value = /*PI/20.f*/ 0.f;
						jointDesc.swingLimit.restitution = /*1.f*/ 0.f;
						jointDesc.flags |= NX_SJF_SWING_LIMIT_ENABLED;
						jointDesc.twistLimit.low.value = /*-PI/16.f*/ 0.f;
						jointDesc.twistLimit.high.value = /*PI/16.f*/ 0.f;
						jointDesc.flags |= NX_SJF_TWIST_LIMIT_ENABLED;

						grabInfo.joint = scene->createJoint(jointDesc);
						grabInfo.actorMass = grabInfo.actor->getMass();

						// hack for static
						if(!grabInfo.actor->isDynamic())
							grabInfo.actorMass = STATIC_OBJECT_MASS;

						// Hack to get more punch to polices
						if(grabInfo.unit)
							grabInfo.actorMass = grabInfo.unitMass;

//						grabInfo.actor->setMass(1.f);
//						if(grabInfo.actorMass > clawSettings.clawMass)
//							actors[clawSettings.boxAmount - 1].actor->setMass(grabInfo.actorMass);

						if(grabInfo.unit)
						{
							grabInfo.unit->setPhysicsObjectLock(true);

							// Claw proto
							UnitLevelAI *ai = (UnitLevelAI *)grabInfo.unit->getAI();
							ai->setEnabled(false);
							grabInfo.unit->targeting.clearTarget();

							game->gameScripting->runOtherScript("claw", "grab_unit", grabInfo.unit, VC3(0,0,0));
			
							grabInfo.actor->clearBodyFlag(NX_BF_FROZEN_POS_X);
							grabInfo.actor->clearBodyFlag(NX_BF_FROZEN_POS_Y);
							grabInfo.actor->clearBodyFlag(NX_BF_FROZEN_POS_Z);
							grabInfo.actor->clearBodyFlag(NX_BF_FROZEN_ROT_Y);
						}
						else
						{
							game->gameScripting->runOtherScript("claw", "grab_object", 0, grabPos);
						}

						if(grabInfo.actor->userData)
						{
							// WARNING: int to void * cast!
							frozenbyte::physics::ActorBase *actorBase = static_cast<frozenbyte::physics::ActorBase *> (grabInfo.actor->userData);
							if(actorBase && actorBase->getUserData() && game->getGamePhysics())
							{
								int handle = reinterpret_cast<int> (actorBase->getUserData());
								IGamePhysicsObject *gamePhysicsObject = game->getGamePhysics()->getInterfaceObjectForHandle(handle);

								if(gamePhysicsObject)
								{
									AbstractPhysicsObject *abstractObject = static_cast<AbstractPhysicsObject *> (gamePhysicsObject);
									abstractObject->setFeedbackEnabled(true);
								}
							}
						}

						// Move target point and add some extra range
						{
							VC3 clawPosition = targetPosition;
							VC3 objectPosition = getClawTargetPosition();

							VC3 diffVec = objectPosition - clawPosition;

							// Move target point and store delta in claw space
							{
								targetPosition += diffVec;

								//VC3 diff2D = diffVec;
								//diff2D.y  = 0.f;

								QUAT clawQuat;
								actors[clawSettings.boxAmount - 1].actor->getRotation(clawQuat);
								clawQuat.Inverse();
								grabInfo.clawSpaceDifference = clawQuat.GetRotated(diffVec);
							}

							// Whats the extra range to actual claw direction
							diffVec.y = 0.f;
							VC3 dir = targetPosition - model->GetPosition();
							dir.y = 0.f;
							if(dir.GetSquareLength() > 0.0001f)
								dir.Normalize();

							float dot = dir.GetDotWith(diffVec);
							grabInfo.extraRange = dot;
							clawSettings.clawMaxDistance += grabInfo.extraRange;
						}

						// ...
						{
							const MassEntry &massEntry = massSettings.getMassEntry(grabInfo);
							Logger::getInstance()->error(" ** New object ** ");
							std::string foo1 = "group name: " + massEntry.key;
							std::string foo2 = "mass: " + boost::lexical_cast<std::string> (grabInfo.actorMass);
							Logger::getInstance()->error(foo1.c_str());
							Logger::getInstance()->error(foo2.c_str());
						}


						//NxActor *clawActor = actors[clawSettings.boxAmount - 1].actor.get()->getActor();
						//grabInfo.actor->setLinearVelocity(clawActor->getLinearVelocity());
						targetInfo.clear();
					}
				}
			}

			// Remove collision from grabbed object
			if(grabInfo.actor)
			{
				for(int j = 1; j < clawSettings.boxAmount; ++j)
					scene->setActorPairFlags(*actors[j].actor->getActor(), *grabInfo.actor, NX_IGNORE_PAIR);

				if(ignoreActor)
					scene->setActorPairFlags(*ignoreActor, *grabInfo.actor, NX_IGNORE_PAIR);
			}
		}
		else
		{
			if(grabInfo.actor)
			{
				if(grabInfo.unit)
				{
					game->gameScripting->runOtherScript( "claw", "ungrab_unit", grabInfo.unit, VC3(0,0,0) );
				}
				else
				{
					grabInfo.actor->clearBodyFlag(NX_BF_FROZEN_ROT_X);
					grabInfo.actor->clearBodyFlag(NX_BF_FROZEN_ROT_Z);
				}

				grabInfo.terrObjModelId =-1;

				// ToDo: height based on targetHeight and targetPosition.y difference
				VC3 objectVelocity;
				objectVelocity.x = speedDelta.x * 60.f;
				objectVelocity.z = speedDelta.y * 60.f;

				VC3 originalObjectVelocity = objectVelocity;;

				// Fudge release velocity to controller direction
				{
					//const float maxAngleChange = PI / 4.f;
					const float maxAngleChange = clawSettings.maxThrowAngleAdjust;

					float objectMag = objectVelocity.GetLength();
					float inputMag = inputDelta.GetLength();

					if(objectMag > 0.0001f && inputMag > 0.0001f)
					{
						VC3 velocityDir = objectVelocity / objectMag;
						VC3 inputDir = VC3(inputDelta.x, 0, inputDelta.y) / inputMag;

						VC3 axis = velocityDir.GetCrossWith(inputDir);
						if(axis.GetSquareLength() > 0.0001f)
						{
							axis.Normalize();
							float dot = velocityDir.GetDotWith(inputDir);
							if(dot > 0)
							{
								float angle = acosf(dot);
								if(angle > maxAngleChange)
									angle = maxAngleChange;
								else if(angle < -maxAngleChange)
									angle = -maxAngleChange;

								QUAT q;
								q.MakeFromAxisRotation(axis, -angle);
								q.RotateVector(objectVelocity);
							}
						}
					}
				}

				// Fudge release velocity to autoaim direction
				{
					//const float maxAngleChange = PI / 10.f;
					const float maxAngleChange = clawSettings.maxAutoAimAdjust;
					float objectMag = originalObjectVelocity.GetLength();

					if(objectMag > 0.0001f)
					{
						VC3 velocityDir = originalObjectVelocity / objectMag;
						VC3 autoaimDir;

						if(getAutoAimDirection(getClawTargetPosition(), velocityDir, maxAngleChange * 180.f / PI, 40.f, autoaimDir))
						{
							VC3 axis = velocityDir.GetCrossWith(autoaimDir);
							if(axis.GetSquareLength() > 0.0001f)
							{
								axis.Normalize();
								float dot = velocityDir.GetDotWith(autoaimDir);
								if(dot > 0)
								{
									float angle = acosf(dot);
									if(angle > maxAngleChange)
										angle = maxAngleChange;
									else if(angle < -maxAngleChange)
										angle = -maxAngleChange;

									QUAT q;
									q.MakeFromAxisRotation(axis, -angle);
									objectVelocity = q.GetRotated(originalObjectVelocity);
								}
							}
						}
					}
				}


				if(releaseExtraSpeed)
				{
					const MassEntry &massEntry = massSettings.getMassEntry(grabInfo);
					/*
					NxVec3 force = grabInfo.actor->getLinearVelocity();
					force *= massEntry.releaseExtraSpeedFactor;

					grabInfo.actor->addForce(force, NX_VELOCITY_CHANGE);
					*/
					if(turboCounter <= 0)
						objectVelocity *= 1.f + massEntry.releaseExtraSpeedFactor;
					releaseExtraSpeed = false;
				}

				grabInfo.actor->setLinearVelocity(fromVC3(objectVelocity));
				//grabInfo.actor->setAngularVelocity(NxVec3(0, 0, 0));
			}

			grabInfo.actor->setMass(grabInfo.actorMass);
			actors[clawSettings.boxAmount - 1].actor->setMass(clawSettings.clawMass);

			clawSettings.clawMaxDistance -= grabInfo.extraRange;

			// Revert object offset from target point
			{
				QUAT clawQuat;
				actors[clawSettings.boxAmount - 1].actor->getRotation(clawQuat);
				VC3 diffVec = clawQuat.GetRotated(grabInfo.clawSpaceDifference);

				targetPosition -= diffVec;
			}

			NxScene *scene = physicsLib->getScene();
			if(grabInfo.joint)
				scene->releaseJoint(*grabInfo.joint);

			// Remove collision for a while
			{
				CollisionFadeInfo fadeInfo;
				fadeInfo.actor = grabInfo.actor;

				//for(int j = 1; j < clawSettings.boxAmount; ++j)
				//	scene->setActorPairFlags(*actors[j].actor->getActor(), *fadeInfo.actor, NX_IGNORE_PAIR);

				if(ignoreActor)
					scene->setActorPairFlags(*ignoreActor, *grabInfo.actor, NX_NOTIFY_ALL);

				collisionFades.push_back(fadeInfo);
			}

			grabInfo.clear();
		}
	}

	int getFrustumShapes(const NxVec3 &position, const NxVec3 &dir, float angle, float range, NxShape **shapes, int maxShapes) const
	{
		assert(fabsf(dir.magnitude() - 1.f < 0.01f));
		NxVec3 up(0, 1.f, 0);
		NxVec3 side = up.cross(dir);
		side.normalize();
		up = dir.cross(side);
		up.normalize();

		NxScene *scene = physicsLib->getScene();
		NxPlane plane[5];

		// Create frustum
		{
			NxQuat yQuat = getRotationTowards(NxVec3(0, 0, 1.f), dir);

			plane[0].normal = side;
			plane[1].normal = -side;
			plane[2].normal = NxVec3(0, 1.f, 0); // up
			plane[3].normal = NxVec3(0, -1.f, 0); // -up
			plane[4].normal = dir;

			//float angle = 30.f; /*PI / 6.f*/;
			//float angle = 45.f; /*PI / 6.f*/;
			//float coneRange = clawSettings.clawMaxDistance;

			NxQuat q;
			q.fromAngleAxis(angle, NxVec3(0, 1.f, 0));
			q.rotate(plane[0].normal);
			q.fromAngleAxis(-angle, NxVec3(0, 1.f, 0));
			q.rotate(plane[1].normal);
			q.fromAngleAxis(-angle, NxVec3(1.f, 0, 0));
			q.rotate(plane[2].normal);
			yQuat.rotate(plane[2].normal);
			q.fromAngleAxis(angle, NxVec3(1.f, 0, 0));
			q.rotate(plane[3].normal);
			yQuat.rotate(plane[3].normal);

			plane[0].set(position, plane[0].normal);
			plane[1].set(position, plane[1].normal);
			plane[2].set(position, plane[2].normal);
			plane[3].set(position, plane[3].normal);
			NxVec3 backPos = position + (dir * range);
			plane[4].set(backPos, plane[4].normal);

			/**/
			debugLines[0].point = toVC3(position);
			debugLines[1].point = toVC3(position);
			debugLines[2].point = toVC3(position);
			debugLines[3].point = toVC3(position);
			debugLines[4].point = toVC3(position) + toVC3(dir * range);
			debugLines[0].normal = toVC3(plane[0].normal);
			debugLines[1].normal = toVC3(plane[1].normal);
			debugLines[2].normal = toVC3(plane[2].normal);
			debugLines[3].normal = toVC3(plane[3].normal);
			debugLines[4].normal = toVC3(plane[4].normal);
			/**/
		}

		return scene->cullShapes(5, plane, NX_ALL_SHAPES, maxShapes, shapes, NULL);
	}

	bool getAutoAimDirection(const VC3 &position, const VC3 &originalDir, float angle, float range, VC3 &resultDir) const
	{
		const int MAX_ACTORS = 100;
		NxShape *shapes[MAX_ACTORS] = { 0 };

		float angleRad = angle / 180.f * PI;

		bool found = false;
		resultDir = originalDir;
		float closestDistance = range * 2.f;

		int activeAmount = getFrustumShapes(fromVC3(position), fromVC3(originalDir), angle, range, shapes, MAX_ACTORS);
		for(int i = 0; i < activeAmount; ++i)
		{
			NxShape *shape = shapes[i];
			if(!shape)
				continue;
			NxActor &actor = shape->getActor();
			if(!actor.isDynamic())
				continue;

			AbstractPhysicsObject *object = 0;
			frozenbyte::physics::ActorBase *actorBase = static_cast<frozenbyte::physics::ActorBase *> (actor.userData);
			if(!actorBase || !actorBase->getUserData() || !game->getGamePhysics())
				continue;

			object = static_cast<AbstractPhysicsObject *> (game->getGamePhysics()->getInterfaceObjectForHandle(reinterpret_cast<int> (actorBase->getUserData())));
			if(!object)
				continue;

			Unit *unit = 0;
			if (object != 0)
			{
				int foo1 = 0;
				int foo2 = 0;
				PhysicsContactUtils::mapPhysicsObjectToUnitOrTerrainObject(game, object, &unit, &foo1, &foo2);

				if(!unit)
					continue;
			}

			// Hax hax
			int unitOwner = unit->getOwner();
			if(!game->isHostile(0, unitOwner))
				continue;

			// We should check that there isn't any static obstacles blocking the way?

			VC3 dirVec = toVC3(getActorCMassGlobalPosition(&actor)) - position;
//		dirVec.y = 0.f;
			float dirDist = dirVec.GetLength();
			dirVec.Normalize();

			float dot = dirVec.GetDotWith(originalDir);
//	if(dot < 0)
//		dot = 0.f;
//		dot = dot * dot;
			float dotAngle = acosf(dot);
			if(dotAngle > angleRad)
				dotAngle = angleRad;
			float factor = dotAngle / angleRad;
			//dirDist += dirDist * factor;
/*
std::stringstream ss;
ss << "Factor:" << factor << std::endl;
Logger::getInstance()->error( ss.str().c_str() );
*/
		dirDist += dirDist * (factor * 10.f);

			if(dirDist < closestDistance)
			{
				closestDistance = dirDist;
				resultDir = dirVec;
				found = true;
			}
		}

		return found;
	}

	bool updateTargeting()
	{
		if(!targetInfo.hasTarget() || grabInfo.actor)
		{
			targetInfo.clear();
			return false;
		}

		if(--autoGrabCounter < 0)
		{
			autoGrabCounter = 0;
			targetInfo.clear();
			return false;
		}
		/*
		// Clear targeting if not trying to grab anything
		if(!grabFlag)
		{
			targetInfo.clear();
			return false;
		}
		*/

		/*
		// Move target point towards actor shape
		NxVec3 globalPhysPos = getActorCMassGlobalPosition(targetInfo.actor);
		VC3 globalPos(globalPhysPos.x, globalPhysPos.y, globalPhysPos.z);
		//targetPosition = globalPos;

		VC3 dir = globalPos - targetPosition;
		if(dir.GetSquareLength() < 0.0001f)
			return true;
		*/

		NxScene *scene = physicsLib->getScene();
		jointBreakDelay = 0;
		VC3 grabPos;

		//IStorm3D_Helper *helper = model->SearchHelper("HELPER_BONE_Claw");
		//if(helper)
		//	grabPos = helper->GetGlobalPosition();

		//grabPos = getClawTargetPosition();
		grabPos = targetPosition;

		// Ray towards object -> should really point to closest point of shape instead of mass center
		NxRay rayShapeDirection;
		//float rayShapeDirectionFudge = 2.5f;
		float rayShapeDirectionFudge = 5.0f;
		rayShapeDirection.orig = fromVC3(grabPos);
		rayShapeDirection.dir = getActorCMassGlobalPosition(targetInfo.actor);
		rayShapeDirection.dir -= fromVC3(grabPos);
		rayShapeDirection.dir.normalize();
		rayShapeDirection.orig -= rayShapeDirection.dir * rayShapeDirectionFudge;

		NxRaycastHit hitInfo;
		if(targetInfo.shape->raycast(rayShapeDirection, clawSettings.clawMaxDistance + rayShapeDirectionFudge + EXTRA_GRAB_DISTANCE, NX_RAYCAST_FACE_NORMAL | NX_RAYCAST_IMPACT | NX_RAYCAST_DISTANCE, hitInfo, false))
		{
			float distance = model->GetPosition().GetRangeTo(toVC3(hitInfo.worldImpact));
			if(distance > clawSettings.clawMaxDistance + EXTRA_GRAB_DISTANCE)
			{
				targetInfo.clear();
				return false;
			}
		}
		else
		{
			targetInfo.clear();
			return false;
		}

		/*
		QUAT clawQuat;
		actors[actors.size() - 1].actor->getRotation(clawQuat);
		VC3 clawDir(0, 0, 1.f);
		clawQuat.RotateVector(clawDir);
		*/

		VC3 clawTargetDir = targetPosition - model->GetPosition();
		clawTargetDir.y = 0.f;
		if(clawTargetDir.GetLength() > 0.0001f)
			clawTargetDir.Normalize();

		//hitInfo.worldImpact += hitInfo.worldNormal * 0.15f;
		targetInfo.setDir(-toVC3(hitInfo.worldNormal), clawTargetDir);

		VC3 globalPos = toVC3(hitInfo.worldImpact);

		if(targetInfo.streetLamp)
		{
			// street lamp middle aim hack
			globalPos = toVC3(targetInfo.actor->getGlobalPose() * NxVec3(0, 2.0f, 0));
			VC3 dirVec(model->GetPosition().x - globalPos.x, 0, model->GetPosition().z - globalPos.z);
			dirVec.Normalize();
			globalPos += dirVec * 0.5f;
		}
		//globalPos -= toVC3(rayShapeDirection.dir * 0.2f);

		targetHeight = globalPos.y;

		VC3 dirVec = globalPos - targetPosition;

		dirVec.y = 0.f;
		float step = clawSettings.autoGrabSpeed;
		float dirLength = dirVec.GetLength();
		if(dirLength < step)
			step = dirLength;
		if(dirLength > 0.0001f)
			dirVec /= dirLength;
		dirVec *= step;
		targetPosition.x += dirVec.x;
		targetPosition.z += dirVec.z;

		/*
		dir.Normalize();
		dir *= 0.7928321376f;
		inputDelta.x = dir.x;
		inputDelta.y = dir.z;
		*/

		return true;
	}

	void update()
	{
		if(!physicsLib)
			return;

		// always mark object as thrown by claw
		if(grabInfo.actor)
		{
			AbstractPhysicsObject *object = 0;
			frozenbyte::physics::ActorBase *actorBase = static_cast<frozenbyte::physics::ActorBase *> (grabInfo.actor->userData);
			if(actorBase && actorBase->getUserData() && game->getGamePhysics())
			{
				object = static_cast<AbstractPhysicsObject *> (game->getGamePhysics()->getInterfaceObjectForHandle(reinterpret_cast<int> (actorBase->getUserData())));
				if(object)
				{
					object->setThrownByClaw(true);
				}
			}
		}

		// update last positions
		if(actors.size() == clawSettings.boxAmount && joints.size() == clawSettings.boxAmount - 1)
		{
			// move each position down
			unsigned int latestLastPosition = lastPositions.size() - 1;
			for(unsigned int i = 0; i < latestLastPosition; i++)
			{
				lastPositions[i] = lastPositions[i + 1];
				lastMasses[i] = lastMasses[i + 1];
			}
			// get latest position
			actors[clawSettings.boxAmount - 1].actor->getMassCenterPosition(lastPositions[latestLastPosition]);
			// get latest mass
			float mass = grabInfo.actor ? grabInfo.actor->getMass() : 0.0f;
			// hack for static
			if(grabInfo.actor && !grabInfo.actor->isDynamic())
				mass = STATIC_OBJECT_MASS;
			lastMasses[latestLastPosition] = mass;
		}
		

		// Drop destroyed/unconscious unit
		if(grabInfo.actor && grabInfo.unit)
		{
			if(grabInfo.unit->isDestroyed() || grabInfo.unit->getMoveState() == Unit::UNIT_MOVE_STATE_UNCONSCIOUS)
				toggleGrab = true;
		}

		// Get rid of broken joints
		//if(grabInfo.joint && grabInfo.joint->getState() == NX_JS_BROKEN)
		//	toggleGrab = true;
		if(grabInfo.actor && grabInfo.joint)
		{
			//float energy = grabInfo.actor->computeKineticEnergy();

			const MassEntry &massEntry = massSettings.getMassEntry(grabInfo);
			//if(++jointBreakDelay > 80 && massEntry.dropForce > 1 && energy > massEntry.dropForce)
			if(++jointBreakDelay > 80 && massEntry.dropForce > 1)
			{
				if(grabInfo.actor->userData)
				{
					frozenbyte::physics::ActorBase *actorBase = static_cast<frozenbyte::physics::ActorBase *> (grabInfo.actor->userData);
					if(actorBase && actorBase->getUserData() && game->getGamePhysics())
					{
						int handle = reinterpret_cast<int> (actorBase->getUserData());
						IGamePhysicsObject *gamePhysicsObject = game->getGamePhysics()->getInterfaceObjectForHandle(handle);

						if(gamePhysicsObject)
						{
							AbstractPhysicsObject *abstractObject = static_cast<AbstractPhysicsObject *> (gamePhysicsObject);
							VC3 v1 = abstractObject->getFeedbackNormal();

							float len = v1.GetLength();

//std::string foo = boost::lexical_cast<std::string> (len);
//Logger::getInstance()->error(foo.c_str());

							if(len > massEntry.dropForce)
							{
								toggleGrab = true;
							}
						}
					}
				}
			}
		}

		//updateTargeting();

		// Update faded collisions
		{
			for(unsigned int i = 0; i < collisionFades.size(); )
			{
				CollisionFadeInfo &fade = collisionFades[i];
				if(fade.actor)
				{
					if(--fade.updates <= 0)
					{
						// Restore collision
						NxScene *scene = physicsLib->getScene();
						for(int j = 1; j < clawSettings.boxAmount; ++j)
							scene->setActorPairFlags(*actors[j].actor->getActor(), *fade.actor, NX_NOTIFY_ALL);

						collisionFades.erase(collisionFades.begin() + i);
					}
					else
						++i;
				}
				else
				{
					// Removed by destroyed actor
					collisionFades.erase(collisionFades.begin() + i);
				}
			}
		}

		if(model)
		{
			IStorm3D_Helper *helper = model->SearchHelper(clawSettings.tailHelper.c_str());
			if(helper)
			{
				rootPosition = helper->GetGlobalPosition();
				IStorm3D_Bone *bone = helper->GetParentBone();

				if(bone)
				{
					const MAT &tm = bone->GetMXG();
					rootRotation = tm.GetRotation();

					//VC3 dir(0, 0, 1.f);
					//rootRotation.RotateVector(dir);
					//rootPosition -= dir * clawSettings.boxSize.z * 4.f;
				}
			}
			else
			{
				rootRotation = QUAT();
				rootPosition = model->GetPosition();
				rootPosition.y += 0.5f;
			}

			VC3 dist = targetPosition - model->GetPosition();
			if(dist.GetLength() > clawSettings.clawMaxDistance)
			{
				//if(turboCounter > 0)
				//	turboCounter = 0;

				dist.Normalize();
				targetPosition = model->GetPosition() + (dist * clawSettings.clawMaxDistance);

				// Remove velocity part going out of range
				VC2 dir2(dist.x, dist.z);
				if(dir2.GetSquareLength() > 0.001f)
				{
					dir2.Normalize();
					float dot = dir2.GetDotWith(speedDelta);
					speedDelta -= dir2 * dot;
				}
			}
			/*
			VC3 dist = targetPosition - rootPosition;
			if(dist.GetLength() > clawSettings.clawMaxDistance)
			{
				dist.Normalize();
				targetPosition = rootPosition + (dist * clawSettings.clawMaxDistance);
			}
			*/

			//modelDelta = model->GetPosition() - modelPosition;
			//modelPosition = model->GetPosition();
		}

		if(recreatePhysics)
		{
			createPhysics();
			recreatePhysics = false;
		}

		if(actors.size() != clawSettings.boxAmount || joints.size() != clawSettings.boxAmount - 1)
			return;

		actors[0].actor->movePosition(rootPosition);
		actors[0].actor->moveRotation(rootRotation);

		moveClaw();
		modelDelta = VC3();

		updateGrab();
	}

	void parseStuff()
	{
		frozenbyte::editor::Parser parser;
		frozenbyte::filesystem::createInputFileStream("Data/claw_settings.txt") >> parser;

		const ParserGroup &globals = parser.getGlobals();
		int groups = globals.getSubGroupAmount();
		for(int i = 0; i < groups; ++i)
		{
			// Claw
			{
				const ParserGroup &group = globals.getSubGroup("Claw");

				clawSettings.clawLength = convertFromString<float> (group.getValue("length"), 8.0f);
				clawSettings.tailHelper = group.getValue("tail_helper", "HELPER_BONE_Tail");
				clawSettings.boxAmount = convertFromString<int> (group.getValue("box_amount"), 16);
				clawSettings.boxSize.x = convertFromString<float> (group.getValue("box_size_x"), 0.1f);
				clawSettings.boxSize.y = convertFromString<float> (group.getValue("box_size_x"), 0.1f);
				clawSettings.boxSize.z = clawSettings.clawLength / float(clawSettings.boxAmount) * 0.5f;
				clawSettings.boxDamping = convertFromString<float> (group.getValue("box_damping"), 3.0f);
				clawSettings.boxMass = convertFromString<float> (group.getValue("box_mass"), 0.3f);
				clawSettings.clawMass = convertFromString<float> (group.getValue("claw_mass"), 10.0f);
				clawSettings.clawCenterForceEnabled = convertFromString<bool> (group.getValue("center_force"), true);
				clawSettings.clawRotateAwayEnabled = convertFromString<bool> (group.getValue("rotate_away"), true);
				clawSettings.velocityScaleFactor = convertFromString<float> (group.getValue("velocity_scale_factor"), 1.0f);
				clawSettings.maxVelocityFactor = convertFromString<float> (group.getValue("max_velocity_factor"), 10.0f);
				//clawSettings.clawMaxDistance = (clawSettings.boxSize.z * 2.f * clawSettings.boxAmount) * /*0.95f*/ 1.1f;
				//clawSettings.clawMaxDistance = (clawSettings.boxSize.z * 2.f * clawSettings.boxAmount) * 1.1f;
				clawSettings.clawMaxDistance = convertFromString<float> (group.getValue("max_distance"), clawSettings.clawLength + 1.0f);
				clawSettings.clawMaxDistanceOriginal = clawSettings.clawMaxDistance;
				clawSettings.heightWithActor = convertFromString<float> (group.getValue("height_with_actor"), 1.75f);
				clawSettings.heightWithoutActor = convertFromString<float> (group.getValue("height_without_actor"), 1.0f);
				clawSettings.bodyForceFactor = convertFromString<float> (group.getValue("body_force_factor"), 1.0f);
				clawSettings.bodyUpForce = convertFromString<float> (group.getValue("body_up_force"), 0.3f);

				clawSettings.maxThrowAngleAdjust = convertFromString<float> (group.getValue("max_throw_angle_adjust"), 45.0f);
				clawSettings.maxAutoAimAdjust = convertFromString<float> (group.getValue("max_autoaim_angle_adjust"), 20.0f);
				clawSettings.maxThrowAngleAdjust *= PI / 180.f;
				clawSettings.maxAutoAimAdjust *= PI / 180.f;

				clawSettings.maxTurboAutoAimAdjust = convertFromString<float> (group.getValue("max_turbo_autoaim_angle_adjust"), 20.0f);
				clawSettings.maxAimedThrowAutoAimAdjust = convertFromString<float> (group.getValue("max_aimed_throw_autoaim_angle_adjust"), 15.0f);
				clawSettings.maxTurboAutoAimAdjust *= PI / 180.f;
				clawSettings.maxAimedThrowAutoAimAdjust *= PI / 180.f;

				clawSettings.minAimedThrowBetaAngle = convertFromString<float> (group.getValue("min_aimed_throw_beta_angle"), -5.0f);
				clawSettings.minAimedThrowBetaAngle *= PI / 180.f;
				clawSettings.maxAimedThrowBetaAngle = convertFromString<float> (group.getValue("max_aimed_throw_beta_angle"), 20.0f);
				clawSettings.maxAimedThrowBetaAngle *= PI / 180.f;
				clawSettings.aimedThrowBetaAngleForceScale = convertFromString<float> (group.getValue("aimed_throw_beta_angle_force_scale"), 1.0f);


				clawSettings.actionPrepareTicks = convertFromString<int> (group.getValue("aimed_throw_prepare_time"), 500) / 15;
				clawSettings.actionPrepareSpeed = convertFromString<float> (group.getValue("aimed_throw_prepare_speed"), 0.4f);
				clawSettings.actionTicks = convertFromString<int> (group.getValue("aimed_throw_time"), 0) / 15;
				clawSettings.actionHitSpeed = convertFromString<float> (group.getValue("aimed_throw_speed"), 0.6f);
				clawSettings.turboCooldownTicks = convertFromString<int> (group.getValue("turbo_cooldown"), 1000) / 15;
				clawSettings.turboTicks = convertFromString<int> (group.getValue("turbo_duration"), 1000) /15;
				clawSettings.turboControllerFactor = convertFromString<float> (group.getValue("turbo_acceleration_factor"), 4.0f);
				clawSettings.turboMaxspeedFactor = convertFromString<float> (group.getValue("turbo_maxvelocity_factor"), 2.0f);
				clawSettings.restPoseTicks = convertFromString<int> (group.getValue("rest_pose_time"), 2000) / 15;
				clawSettings.autoGrabTicks = convertFromString<int> (group.getValue("auto_grab_time"), 1000) / 15;
				clawSettings.autoGrabSpeed = convertFromString<float> (group.getValue("auto_grab_speed"), 0.3f);
				controlSettings.drawHelperCursor = convertFromString<bool> (group.getValue("draw_helper_cursor"), false);

				clawSettings.preferObjectsFromClawDirectionNonlinearSpread = convertFromString<float> (group.getValue("prefer_objects_from_claw_direction_nonlinear_spread"), 1.0f);
				clawSettings.preferObjectsFromClawDirectionDistanceFactor = convertFromString<float> (group.getValue("prefer_objects_from_claw_direction_distance_factor"), 10.0f);
				clawSettings.preferObjectsWithMassLimit = convertFromString<float> (group.getValue("prefer_objects_with_mass_limit"), 50.0f);
				clawSettings.preferObjectsWithMassFactor = convertFromString<float> (group.getValue("prefer_objects_with_mass_factor"), 0.5f);
			}

			// Controls
			{
				const ParserGroup &controlsGroup = globals.getSubGroup("Controls");

				clawSettings.restPoseReturnMethod = convertFromString<float> (controlsGroup.getValue("rest_pose_return_method"), 0.5f);
				clawSettings.restPoseScaleFactor = convertFromString<float> (controlsGroup.getValue("rest_pose_scale_factor"), 0.005f);
				clawSettings.restPoseLinearSpeed = convertFromString<float> (controlsGroup.getValue("rest_pose_linear_speed"), 0.015f);

				controlSettings.animationSwitchFarPoseRange = convertFromString<float> (controlsGroup.getValue("animation_switch_far_pose_range"), 5.5f);
				controlSettings.animationSwitchNearPoseRange = convertFromString<float> (controlsGroup.getValue("animation_switch_near_pose_range"), 5.0f);

				// Pad
				{
					const ParserGroup &group = controlsGroup.getSubGroup("Pad");
					controlSettings.padDeltaScale = convertFromString<float> (group.getValue("delta_scale"), 0.015f);
					controlSettings.padDeltaPower = convertFromString<float> (group.getValue("delta_power"), 1.82f);
					controlSettings.padDeltaTurboFactor = convertFromString<float> (group.getValue("delta_turbo_factor"), 1.5f);
					controlSettings.padTriggerMovementFactor = convertFromString<float> (group.getValue("trigger_movement_factor"), 1.5f);
				}

				// Mouse
				{
					const ParserGroup &group = controlsGroup.getSubGroup("Mouse");
					controlSettings.mouseDeltaScale = convertFromString<float> (group.getValue("delta_scale"), 0.025f);
					controlSettings.mouseDeltaLimit = convertFromString<int> (group.getValue("delta_limit"), 5);
				}
			}

			// Mass
			{
				const ParserGroup &group = globals.getSubGroup("MassResponse");
				massSettings.massEntries.clear();

				grabInfo.unitMass = convertFromString<float> (group.getValue("unit_mass"), 80.f);

				for(int j = 0; j < group.getValueAmount(); ++j)
				{
					const std::string key = group.getValueKey(j);
					if(key.empty())
						continue;
					int mass = convertFromString<int> (key, -1);
					if(mass < 0)
						continue;

					MassEntry entry;
					entry.key = group.getValue(key);
					entry.mass = float(mass);

					const ParserGroup &valueGroup = group.getSubGroup(group.getValue(key));

					entry.minimumSpeedTowardsController = convertFromString<float> (valueGroup.getValue("minimum_speed_towards_controller"), 0.f);
					entry.liftSpeed = convertFromString<float> (valueGroup.getValue("lift_speed"), 0.07f);
					// Decay
					entry.accelerationDecayLinear = convertFromString<float> (valueGroup.getValue("acceleration_decay_linear"), 0.f);
					entry.accelerationDecayFactor = convertFromString<float> (valueGroup.getValue("acceleration_decay_factor"), 0.f);
					entry.accelerationDecayNonlinearFactor = convertFromString<float> (valueGroup.getValue("acceleration_decay_nonlinear_factor"), 0.f);
					entry.inactiveAccelerationDecayLinear = convertFromString<float> (valueGroup.getValue("inactive_acceleration_decay_linear"), 0.f);
					entry.inactiveAccelerationDecayFactor = convertFromString<float> (valueGroup.getValue("inactive_acceleration_decay_factor"), 0.f);
					entry.inactiveAccelerationDecayNonlinearFactor = convertFromString<float> (valueGroup.getValue("inactive_acceleration_decay_nonlinear_factor"), 0.f);
					entry.controllerAlignedAccelerationDecayLinear = convertFromString<float> (valueGroup.getValue("controller_aligned_acceleration_decay_linear"), 0.f);
					entry.controllerAlignedAccelerationDecayFactor = convertFromString<float> (valueGroup.getValue("controller_aligned_acceleration_decay_factor"), 0.f);
					entry.controllerAlignedAccelerationDecayNonlinearFactor = convertFromString<float> (valueGroup.getValue("controller_aligned_acceleration_decay_nonlinear_factor"), 0.f);

					// Acceleration
					entry.accelerationIncreaseLinear = convertFromString<float> (valueGroup.getValue("acceleration_increase_linear"), 0.f);
					entry.accelerationIncreaseFactor = convertFromString<float> (valueGroup.getValue("acceleration_increase_factor"), 0.f);
					entry.accelerationIncreaseNonlinearFactor = convertFromString<float> (valueGroup.getValue("acceleration_increase_nonlinear_factor"), 0.f);
					entry.accelerationDisableLinearIncreaseSpeed = convertFromString<float> (valueGroup.getValue("acceleration_disable_linear_increase_speed"), 10000.f);

					entry.velocityRotationHelperFactor = convertFromString<float> (valueGroup.getValue("velocity_rotation_helper_factor"), 0.f);
					entry.maxVelocity = convertFromString<float> (valueGroup.getValue("max_velocity"), 10000.f);
					entry.dropForce = convertFromString<float> (valueGroup.getValue("drop_force"), 1.f);
					entry.releaseExtraSpeedFactor = convertFromString<float> (valueGroup.getValue("release_extra_speed_factor"), 0.f);

					entry.clawCenterForceEnabled = convertFromString<bool> (valueGroup.getValue("center_force"), true);
					entry.clawRotateAwayEnabled = convertFromString<bool> (valueGroup.getValue("rotate_away"), true);

					massSettings.massEntries.push_back(entry);
				}

				std::sort(massSettings.massEntries.begin(), massSettings.massEntries.end(), MassEntrySorter());
			}
		}
	}

	static NxVec3 getActorCMassGlobalPosition(NxActor *actor)
	{
		if(actor->isDynamic())
		{
			return actor->getCMassGlobalPosition();	
		}
		else
		{
			// static actor hack
			NxVec3 pos = actor->getShapes()[0]->getGlobalPosition();
			return pos;
		}
	}

	VC3 getClawTargetPosition() const
	{
		VC3 position = targetPosition;

		/**/
		if(grabInfo.actor && !newClawObject)
		{
			position = toVC3(getActorCMassGlobalPosition(grabInfo.actor));
		}
		else /**/ if(!actors.empty())
		{
			boost::shared_ptr<frozenbyte::physics::ActorBase> claw = actors[actors.size() - 1].actor;
			if(claw)
				claw->getMassCenterPosition(position);
		}

		return position;
	}

   bool onHit(const NxRaycastHit &hit)
	{
		const NxShape *shape = hit.shape;
		const NxActor &actor = shape->getActor();
		if(!shape)
			return false;
		if(grabInfo.actor && grabInfo.actor == &actor)
			return false;

		int collisionGroup = shape->getGroup();
		if(collisionGroup == PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_WO_UNIT_COLL_NO_SOUND)
			return false;
		if(collisionGroup == PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_NO_SOUND)
			return false;
		if(collisionGroup == PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_WO_UNIT_COLL)
			return false;
		if(collisionGroup == PHYSICS_COLLISIONGROUP_MODEL_PARTICLES)
			return false;
		if(&actor == ignoreActor)
			return false;

		bool found = false;
		for(unsigned int j = 0; j < actors.size(); ++j)
		{
			if(&actor == actors[j].actor.get()->getActor())
				found = true;
		}

		if(found)
			return false;

		VC3 clawPosition;
		actors[clawSettings.boxAmount - 1].actor->getMassCenterPosition(clawPosition);

		VC3 hitPos = toVC3(hit.worldImpact);
		if(clawPosition.GetSquareRangeTo(hitPos) < clawPosition.GetSquareRangeTo(targetPosition))
		{
			VC3 dir = hitPos - clawPosition;
			if(dir.GetSquareLength() > 0.001f)
				dir.Normalize();
			hitPos -= dir * 0.1f;

			targetPosition = hitPos;
		}

		return true;
    }
};

ClawController::ClawController()
:	data(new Data())
{
}

ClawController::~ClawController()
{
}

void ClawController::setGame(Game *game)
{
	data->game = game;
}

void ClawController::createPhysics(frozenbyte::physics::PhysicsLib *physicsLib)
{
	data->setPhysics(physicsLib);
}

void ClawController::setModel(IStorm3D_Model *model)
{
	data->setModel(model);
}

void ClawController::setSpawnPosition(const VC3 &pos)
{
	if(!data->model)
		data->rootPosition = pos;
}

static float		claw_delta_min = 1000000.0f;
static float		claw_delta_ave = 0.0f;
static unsigned int claw_delta_ave_count = 0;
static float		claw_delta_max = 0;

void ClawController::update()
{
	if(data->newClawObject && ++data->newClawObjectDelay > 1)
	{
		NxActor *newActor = 0;
		game::IGamePhysicsObject *obj = data->game->getGamePhysics()->getInterfaceObjectForHandle(data->newClawObject->getHandle());
		if(obj)
			newActor = data->game->getGamePhysics()->getImplementingObject(obj);

		if(newActor)
		{
						VC3 grabPos = data->getClawTargetPosition();
						QUAT clawQuat;
						data->actors[data->actors.size() - 1].actor->getRotation(clawQuat);
						VC3 clawDir(0, 0, 1.f);
						clawQuat.RotateVector(clawDir);

						// try not to destroy the whole thing
						if(data->grabInfo.streetLamp)
						{
							data->targetPosition = grabPos;
							data->grabInfo.streetLampNoMoveDelay = GAME_TICKS_PER_SECOND / 10;
						}

						if(data->grabInfo.joint)
						{
							data->physicsLib->getScene()->releaseJoint(*data->grabInfo.joint);
						}


						NxScene *scene = data->physicsLib->getScene();
						

						NxSphericalJointDesc jointDesc;
						jointDesc.setToDefault();
						jointDesc.actor[0] = data->actors[data->clawSettings.boxAmount - 1].actor.get()->getActor();
						jointDesc.actor[1] = newActor;

						jointDesc.setGlobalAnchor(fromVC3(grabPos));
						jointDesc.setGlobalAxis(fromVC3(clawDir));

						data->grabInfo.actor = newActor;
						if(data->game)
						{
							AbstractPhysicsObject *object = 0;
							frozenbyte::physics::ActorBase *actorBase = static_cast<frozenbyte::physics::ActorBase *> (newActor->userData);
							if(actorBase && actorBase->getUserData() && data->game->getGamePhysics())
								object = static_cast<AbstractPhysicsObject *> (data->game->getGamePhysics()->getInterfaceObjectForHandle(reinterpret_cast<int> (actorBase->getUserData())));

							if (object != 0)
							{
								object->setThrownByClaw(true);
								PhysicsContactUtils::mapPhysicsObjectToUnitOrTerrainObject(data->game, object, &data->grabInfo.unit, &data->grabInfo.terrObjModelId, &data->grabInfo.terrObjInstanceId);

								// determine if it's a street lamp
								data->grabInfo.streetLamp = isStreetLamp(data->game, data->grabInfo.terrObjModelId);
							}
						}

						jointDesc.swingLimit.value = /*PI/20.f*/ 0.f;
						jointDesc.swingLimit.restitution = /*1.f*/ 0.f;
						jointDesc.flags |= NX_SJF_SWING_LIMIT_ENABLED;
						jointDesc.twistLimit.low.value = /*-PI/16.f*/ 0.f;
						jointDesc.twistLimit.high.value = /*PI/16.f*/ 0.f;
						jointDesc.flags |= NX_SJF_TWIST_LIMIT_ENABLED;

						data->grabInfo.joint = scene->createJoint(jointDesc);
						data->grabInfo.actorMass = data->grabInfo.actor->getMass();
						// hack for static
						if(!data->grabInfo.actor->isDynamic())
							data->grabInfo.actorMass = STATIC_OBJECT_MASS;

						if(data->grabInfo.actor->userData)
						{
							// WARNING: int to void * cast!
							frozenbyte::physics::ActorBase *actorBase = static_cast<frozenbyte::physics::ActorBase *> (data->grabInfo.actor->userData);
							if(actorBase && actorBase->getUserData() && data->game->getGamePhysics())
							{
								int handle = reinterpret_cast<int> (actorBase->getUserData());
								IGamePhysicsObject *gamePhysicsObject = data->game->getGamePhysics()->getInterfaceObjectForHandle(handle);

								if(gamePhysicsObject)
								{
									AbstractPhysicsObject *abstractObject = static_cast<AbstractPhysicsObject *> (gamePhysicsObject);
									abstractObject->setFeedbackEnabled(true);
								}
							}
						}

						data->targetInfo.clear();

						// Remove collision from grabbed object
						if(data->grabInfo.actor)
						{
							for(int j = 1; j < data->clawSettings.boxAmount; ++j)
								scene->setActorPairFlags(*data->actors[j].actor->getActor(), *data->grabInfo.actor, NX_IGNORE_PAIR);

							if(data->ignoreActor)
								scene->setActorPairFlags(*data->ignoreActor, *data->grabInfo.actor, NX_IGNORE_PAIR);
						}
		}
		else
		{
			if(data->grabInfo.joint)
				data->physicsLib->getScene()->releaseJoint(*data->grabInfo.joint);

			data->actors[data->clawSettings.boxAmount - 1].actor->setMass(data->clawSettings.clawMass);
			data->clawSettings.clawMaxDistance -= data->grabInfo.extraRange;

			// Revert object offset from target point
			{
				QUAT clawQuat;
				data->actors[data->clawSettings.boxAmount - 1].actor->getRotation(clawQuat);
				VC3 diffVec = clawQuat.GetRotated(data->grabInfo.clawSpaceDifference);

				data->targetPosition -= diffVec;
			}

			data->grabInfo.clear();
		}

		data->newClawObject.reset();
	}

	if(data->newClawObject)
		return;

	// ---

	// HACK!!
	if(!data->grabInfo.actor)
		data->clawSettings.clawMaxDistance = data->clawSettings.clawMaxDistanceOriginal;

	if(--data->turboCounter < -data->clawSettings.turboCooldownTicks)
		data->turboCounter = -data->clawSettings.turboCooldownTicks;
	if(data->turboAction == TurboThrow && data->grabInfo.actor && data->model && data->turboCounter > 0)
	{
		VC3 distVec = data->targetPosition - data->model->GetPosition();
		VC3 dirVec(data->turboVec.x, 0, data->turboVec.y);
		if(dirVec.GetLength() > 0.0001f)
			dirVec.Normalize();
	
		dirVec.y = distVec.GetNormalized().y;
		float factor = sqrtf(1.f - (dirVec.y*dirVec.y));
		dirVec.x *= factor;
		dirVec.z *= factor;

		float dot = dirVec.GetDotWith(distVec);
		float dist = data->clawSettings.clawMaxDistance - dot;

		if(dist < 1.5f || data->turboCounter == 2)
			data->toggleGrab = true;
	}

	if(--data->restPoseCounter < 0)
		data->restPoseCounter = 0;

	/*

		//
		/*
		if(scene->raycastAnyShape(worldRay, NX_ALL_SHAPES, 0xffffffff, , 0, shapes))
		{
			NxRaycastHit hitInfo;
			if(activeShape && activeShape->raycast(worldRay, 1.0f, NX_RAYCAST_FACE_NORMAL, hitInfo, false))
			{
				dir = -hitInfo.worldNormal;
				dir.y -= 0.5f;
				dir.normalize();
			}
		}
		*/

		/*
		const int MAX_ACTORS = 10;
		NxSphere worldSphere(NxVec3(grabPos.x,grabPos.y,grabPos.z), 0.1f);
		NxShape *shapes[MAX_ACTORS] = { 0 };
		int activeAmount = scene->overlapSphereShapes(worldSphere, NX_DYNAMIC_SHAPES, MAX_ACTORS, shapes, NULL, 0xffffffff, 0, true);

		NxActor *activeActor = 0;
		NxShape *activeShape = 0;
		for(int i = 0; i < activeAmount; ++i)
		{
			if(!shapes[i])
				continue;
			NxActor &actor = shapes[i]->getActor();

			int collisionGroup = shapes[i]->getGroup();
			if(collisionGroup == PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_WO_UNIT_COLL_NO_SOUND)
				continue;
			if(collisionGroup == PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_NO_SOUND)
				continue;
			if(collisionGroup == PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_WO_UNIT_COLL)
				continue;
			if(collisionGroup == PHYSICS_COLLISIONGROUP_MODEL_PARTICLES)
				continue;
			if(&actor == ignoreActor)
				continue;

			bool found = false;
			for(unsigned int j = 0; j < actors.size(); ++j)
			{
				if(&actor == actors[j].actor.get()->getActor())
					found = true;
			}

			if(found)
				continue;

			activeActor = &actor;
			activeShape = shapes[i];
		}
	}
	*/

	bool hasAutoAimTarget = data->updateTargeting();

	// Clamp target point distance from claw
	if(data->clawEnabled)
	{
		data->clawDistanceClamp = false;
		// This rather needs a sphere around claw position to check where is collision free area
		// Anyway, for now it works surprisingly well
		if(!hasAutoAimTarget)
		{
			VC3 tickMovement;
			if(data->grabInfo.actor)
				tickMovement = toVC3(data->grabInfo.actor->getLinearVelocity()) / 66.667f;

			VC3 targetPoint = data->getClawTargetPosition();
			targetPoint -= tickMovement * 0.25f;
			//targetPoint -= tickMovement * 0.20f;

			VC3 distanceVec = data->targetPosition - targetPoint;
			distanceVec.y = 0.f;
			float distance = distanceVec.GetLength();
			const float limit = 0.15f;
			//const float limit = 0.05f;

			if(distance > limit)
			{
				distanceVec /= distance;
				//data->targetPosition.x = targetPoint.x;
				//data->targetPosition.z = targetPoint.z;
				data->targetPosition.x = targetPoint.x + (distanceVec.x * limit);
				data->targetPosition.z = targetPoint.z + (distanceVec.z * limit);

				// HAX HAX 
				// Remove speedDelta based on distanceVec so that there can be no acceleration 
				// towards that direction

				VC2 dir2(distanceVec.x, distanceVec.z);
				if(dir2.GetSquareLength() > 0.001f)
				{
					dir2.Normalize();
					float dot = dir2.GetDotWith(data->speedDelta);
					data->speedDelta -= dir2 * dot;
				}

				data->clawDistanceClamp = true;
			}
		}
		/**/
	}

	// Update movement (in game ticks)
	if(hasAutoAimTarget)
	{
		data->inputDelta = VC2();
		data->speedDelta = VC2();
	}
	else
	{
		const MassEntry &massEntry = data->massSettings.getMassEntry(data->grabInfo);
		float deltaAmount = data->speedDelta.GetLength();
		float inverseDelta = 1.f / (deltaAmount + 1.f);

		/*
		// Decay
		{
			float decayLinear = massEntry.accelerationDecayLinear;
			float decayFactor = massEntry.accelerationDecayFactor;
			float decayNonlinearFactor = massEntry.accelerationDecayNonlinearFactor;
			if(data->inputDelta.GetSquareLength() < 0.0001f)
			{
				decayLinear = massEntry.inactiveAccelerationDecayLinear;
				decayFactor = massEntry.inactiveAccelerationDecayFactor;
				decayNonlinearFactor = massEntry.inactiveAccelerationDecayNonlinearFactor;
			}

			if(decayLinear > 0.0001f)
			{
				if(deltaAmount < decayLinear)
					decayLinear = deltaAmount;

				if(deltaAmount > 0.0001f)
					data->speedDelta /= deltaAmount;

				data->speedDelta *= deltaAmount - decayLinear;

			}

			float decayValue = decayFactor - (inverseDelta * decayNonlinearFactor);
			if(decayValue < 0.f)
				decayValue = 0.f;
			data->speedDelta *= decayValue;
		}
		*/

		// Decay
		{
			float decayLinear1 = massEntry.accelerationDecayLinear;
			float decayFactor1 = massEntry.accelerationDecayFactor;
			float decayNonlinearFactor1 = massEntry.accelerationDecayNonlinearFactor;
			float decayLinear2 = massEntry.controllerAlignedAccelerationDecayLinear;
			float decayFactor2 = massEntry.controllerAlignedAccelerationDecayFactor;
			float decayNonlinearFactor2 = massEntry.controllerAlignedAccelerationDecayNonlinearFactor;
			float interpolateAmount = 1.f;

			if(data->inputDelta.GetSquareLength() < 0.0001f)
			{
				decayLinear1 = massEntry.inactiveAccelerationDecayLinear;
				decayFactor1 = massEntry.inactiveAccelerationDecayFactor;
				decayNonlinearFactor1 = massEntry.inactiveAccelerationDecayNonlinearFactor;
				decayLinear2 = decayLinear1;
				decayFactor2 = decayFactor1;
				decayNonlinearFactor2 = decayNonlinearFactor1;
				interpolateAmount = 0.f;
			}
			else
			{
				/*
				VC2 velocity = data->speedDelta;
				if(velocity.GetSquareLength() > 0.0001f)
					velocity.Normalize();
				VC2 controlDir = data->inputDelta;
				if(controlDir.GetSquareLength() > 0.0001f)
					controlDir.Normalize();

				interpolateAmount = velocity.GetDotWith(controlDir);
				if(interpolateAmount < 0)
					interpolateAmount = 0.f;

				// Fudge angle
				interpolateAmount = (interpolateAmount - 0.5f) * 2.f;
				if(interpolateAmount < 0)
					interpolateAmount = 0.f;
				*/

				VC2 velocity = data->speedDelta;
				VC2 controlDir = data->inputDelta;

				if(velocity.GetSquareLength() > 0.0001f && controlDir.GetSquareLength() > 0.0001f)
				{
					velocity.Normalize();
					controlDir.Normalize();

					interpolateAmount = velocity.GetDotWith(controlDir);
					if(interpolateAmount < 0)
						interpolateAmount = 0.f;

					// Fudge angle
					interpolateAmount = (interpolateAmount - 0.5f) * 2.f;
					if(interpolateAmount < 0)
						interpolateAmount = 0.f;
				}
				else if(controlDir.GetSquareLength() > 0.0001f)
					interpolateAmount = 1.f;
				else
					interpolateAmount = 0.f;
			}

			VC2 speedDelta1 = data->speedDelta;
			{
				if(decayLinear1 > 0.00001f)
				{
					if(deltaAmount < decayLinear1)
						decayLinear1 = deltaAmount;

					if(deltaAmount > 0.00001f)
						speedDelta1 /= deltaAmount;

					speedDelta1 *= deltaAmount - decayLinear1;
				}

				float decayValue = decayFactor1 - (inverseDelta * decayNonlinearFactor1);
				if(decayValue < 0.f)
					decayValue = 0.f;
				speedDelta1 *= decayValue;
			}

			VC2 speedDelta2 = data->speedDelta;
			{
				if(decayLinear2 > 0.00001f)
				{
					if(deltaAmount < decayLinear2)
						decayLinear2 = deltaAmount;

					if(deltaAmount > 0.00001f)
						speedDelta2 /= deltaAmount;

					speedDelta2 *= deltaAmount - decayLinear2;
				}

				float decayValue = decayFactor2 - (inverseDelta * decayNonlinearFactor2);
				if(decayValue < 0.f)
					decayValue = 0.f;
				speedDelta2 *= decayValue;
			}
/*
std::stringstream ss;
ss << "Interpolate value: " << interpolateAmount << std::endl;
Logger::getInstance()->debug( ss.str().c_str() );
*/
			data->speedDelta = speedDelta1 * (1.f - interpolateAmount);
			data->speedDelta += speedDelta2 * interpolateAmount;
		}

		// Acceleration
		{
			VC2 acceleration = data->inputDelta;
			if(deltaAmount > massEntry.accelerationDisableLinearIncreaseSpeed)
				acceleration *= deltaAmount * massEntry.accelerationIncreaseNonlinearFactor;
			else
			{
				acceleration *= (massEntry.accelerationIncreaseFactor + deltaAmount * massEntry.accelerationIncreaseNonlinearFactor);

				if(massEntry.accelerationIncreaseLinear > 0.0001f)
				{
					VC2 linearVec = data->inputDelta;
					float amag = linearVec.GetLength();
					if(amag > 0.0001f)
						linearVec /= amag;

					linearVec *= massEntry.accelerationIncreaseLinear;
					acceleration += linearVec;
				}
			}

			// Minimum movement
			{
				VC2 linearVec = data->inputDelta;
				float amag = linearVec.GetLength();
				if(amag > 0.0001f)
					linearVec /= amag;

				linearVec *= massEntry.minimumSpeedTowardsController;
				data->speedDelta += linearVec;
			}

			data->speedDelta += acceleration;
		}

		// Clamp
		float maxVelocity = massEntry.maxVelocity;
		if(data->turboCounter > 0)
			maxVelocity *= data->clawSettings.turboMaxspeedFactor;;
		if(data->speedDelta.GetLength() > maxVelocity)
		{
			data->speedDelta.Normalize();
			data->speedDelta *= maxVelocity;
		}

		// Add
		data->targetPosition.x += data->speedDelta.x;
		data->targetPosition.z += data->speedDelta.y;

		// hack: extra strength for pulling lamps
		if(data->grabInfo.actor && data->grabInfo.streetLamp && data->game->getGamePhysics())
		{
			frozenbyte::physics::ActorBase *actorBase = static_cast<frozenbyte::physics::ActorBase *> (data->grabInfo.actor->userData);
			if(actorBase && data->game->getGamePhysics()->hasJointAttachedToWorld(actorBase))
			{
				NxQuat rot;
				data->grabInfo.actor->getGlobalPose().M.toQuat(rot);
				NxVec3 upVector = NxVec3(0, 1.0f, 0);
				rot.rotate(upVector);

				if(upVector.y < 0.95f)
				{
					VC3 dir = data->getClawTargetPosition() - data->targetPosition;
					dir.y = 0;
					float len = dir.GetLength();
					dir *= 1.0f / len;

					data->targetPosition.x -= dir.x * 0.1f;
					data->targetPosition.z -= dir.z * 0.1f;
				}
			}
		}

	}

	// Try to make claw easier to control
	{
		float speedMag = data->speedDelta.GetLength();
		float inputMag = data->inputDelta.GetLength();

		//float factor = 0.95f;
		const MassEntry &massEntry = data->massSettings.getMassEntry(data->grabInfo);
		float factor = 1.f - massEntry.velocityRotationHelperFactor;

		if(speedMag > 0.0001f && inputMag > 0.0001f)
		{
			//float dirDot = data->speedDelta.GetNormalized().GetDotWith(data->inputDelta.GetNormalized());
			//if(dirDot < 0.f)
			//	dirDot = 0.f;

			//if(dirDot > 0.f)
			{
				VC2 vec = (data->speedDelta / speedMag) * factor;
				vec += (data->inputDelta / inputMag) * (1.f - factor);
				vec.Normalize();
				vec *= speedMag;

				//float dot = data->speedDelta.GetNormalized().GetDotWith(vec.GetNormalized());
				//if(dot < 0)
				//	dot = 0;
				//dot = pow(dot, 5.f);
				//vec *= dot;

				// ...
				//float newLen = (speedMag * dirDot) + (vec.GetLength() * (1.f - dirDot));
				//data->speedDelta = (data->speedDelta * (1.f - dirDot)) + (vec * (dirDot));
				data->speedDelta = vec;
			}
		}
	}

	if(data->model)
	{
		data->modelDelta = data->model->GetPosition() - data->modelPosition;
		data->modelPosition = data->model->GetPosition();

		data->targetPosition += data->modelDelta;
		data->restPosition += data->modelDelta;
		data->flexPosition += data->modelDelta;
	}

/*
	// Prevent target point from going through obstacles
	if(data->physicsLib && data->clawEnabled)
	{
		NxScene *scene = data->physicsLib->getScene();
		if(!scene)
			return;

		VC3 clawPos = getClawPosition();
		VC3 clawDir = data->targetPosition - clawPos;
		if(clawDir.GetSquareLength() > 0.0001f)
		{
			clawDir.Normalize();
			NxRay worldRay;
			worldRay.orig = NxVec3(clawPos.x, clawPos.y, clawPos.z);
			worldRay.dir = NxVec3(clawDir.x, clawDir.y, clawDir.z);
			//worldRay.orig += worldRay.dir * data->clawSettings.boxSize.z;
			worldRay.orig -= worldRay.dir * 0.3f;

worldRay.orig.x -= data->modelDelta.x;
worldRay.orig.y -= data->modelDelta.y;
worldRay.orig.z -= data->modelDelta.z;

			scene->raycastAllShapes(worldRay, *data, NX_ALL_SHAPES, -1, clawPos.GetRangeTo(data->targetPosition) + 1.1f, NX_RAYCAST_DISTANCE | NX_RAYCAST_IMPACT | NX_RAYCAST_SHAPE, 0);
		}
	}
*/
	// temp hax
	if( data->actors.empty() == false && data->actors[ data->actors.size() - 1 ].actor )
	{
		VC3 currentVelocity;
		data->actors[ data->actors.size() - 1 ].actor->getVelocity(currentVelocity);

		if( currentVelocity.GetLength() < claw_delta_min ) claw_delta_min = currentVelocity.GetLength();
		if( currentVelocity.GetLength() > claw_delta_max ) claw_delta_max = currentVelocity.GetLength();
		claw_delta_ave = ( ( claw_delta_ave * claw_delta_ave_count ) + currentVelocity.GetLength() );
		++claw_delta_ave_count;
		claw_delta_ave = claw_delta_ave / (float)claw_delta_ave_count;

		/*
		// debug logging
		if( currentVelocity.GetLength() > 37.0f )
		{
			std::stringstream ss;
			ss << "claw_delta max: " << claw_delta_max << "\tmin: " << claw_delta_min << "\taverage: " << claw_delta_ave << std::endl;
			ss << currentVelocity.GetLength() << std::endl;
			Logger::getInstance()->debug( ss.str().c_str() );
		}
		*/
	}

	// Get to rest pose if not moving anymore and no triggered action running
	/*
	if(data->restPoseCounter > 0 &&  data->activeAction.action == ActionNone && && data->turboCounter <= 0 && !hasAutoAimTarget)
	{
		if(data->restPoseDelay > 0)
			--data->restPoseDelay;

		if(data->restPoseDelay <= 0)
		{
			VC3 t1;
			{
				float ff = data->clawSettings.restPoseScaleFactor;
				t1 = (data->targetPosition * (1.f - ff)) + (data->restPosition * ff);
			}

			VC3 t2 = data->targetPosition;
			{
				VC3 dir = data->restPosition - data->targetPosition;
				if(dir.GetSquareLength() > 0.001f)
				{
					float dist = dir.GetLength();
					dir /= dist;

					float f = data->clawSettings.restPoseLinearSpeed;
					if(f > dist)
						f = dist;
					t2 = data->targetPosition + (dir * f);
				}
			}

			data->targetPosition = (t1 * data->clawSettings.restPoseReturnMethod) + (t2 * (1.f - data->clawSettings.restPoseReturnMethod));
		}
	}
	else
		data->restPoseDelay = 150;
	*/

	// Actions
	if(data->model)
	{
		if(data->prepareAction != ActionNone)
		{
			++data->prepareActionCount;

			VC3 dirVec = data->flexPosition - data->targetPosition;
			dirVec.y = 0;
			float dirMag = dirVec.GetLength();

			if(dirMag > 0.001f)
			{
				dirVec.Normalize();

				if(data->clawSettings.actionPrepareSpeed < dirMag)
					data->targetPosition += dirVec * data->clawSettings.actionPrepareSpeed;
				else
					data->targetPosition += dirVec * dirMag;
			}

			data->targetHeight = data->flexPosition.y;
		}

		if(data->activeAction.action != ActionNone)
		{
			/*
			VC3 dir = data->restPosition - data->model->GetPosition();
			dir.y = 0;
			float distance = dir.GetLength();
			if(distance > 0.001f)
				dir.Normalize();
			*/
			VC3 dir = data->activeAction.dir;

			if(data->activeAction.time++ > data->clawSettings.actionTicks)
			{
				if(data->activeAction.action == ActionThrow)
				{
					if(data->grabInfo.actor && data->grabInfo.joint)
					{
						NxScene *scene = data->physicsLib->getScene();
						if(data->grabInfo.joint)
							scene->releaseJoint(*data->grabInfo.joint);

						VC3 force = dir;
						//force *= float(data->activeAction.strength) * 0.5f;
						//data->grabInfo.actor->addForce(NxVec3(force.x, force.y, force.z), NX_VELOCITY_CHANGE);

						force *= data->clawSettings.actionHitSpeed * 66.0f * data->activeAction.strength / 100.0f;
						//force.y += 0.2f;
						data->grabInfo.actor->setLinearVelocity(fromVC3(force));

						if(!data->grabInfo.unit)
						{
							data->grabInfo.actor->clearBodyFlag(NX_BF_FROZEN_ROT_X);
							data->grabInfo.actor->clearBodyFlag(NX_BF_FROZEN_ROT_Z);
						}
						else
						{	
							if(data->game)
								data->game->gameScripting->runOtherScript("claw", "ungrab_unit", data->grabInfo.unit, VC3(0,0,0));
						}
					

						data->grabInfo.actor->setMass(data->grabInfo.actorMass);
						data->actors[data->clawSettings.boxAmount - 1].actor->setMass(data->clawSettings.clawMass);

						// Remove collision for a while
						{
							CollisionFadeInfo fadeInfo;
							fadeInfo.actor = data->grabInfo.actor;

							if(data->ignoreActor)
								scene->setActorPairFlags(*data->ignoreActor, *data->grabInfo.actor, NX_NOTIFY_ALL);

							data->collisionFades.push_back(fadeInfo);
						}

						data->clawSettings.clawMaxDistance -= data->grabInfo.extraRange;

						// Revert object offset from target point
						{
							QUAT clawQuat;
							data->actors[data->clawSettings.boxAmount - 1].actor->getRotation(clawQuat);
							VC3 diffVec = clawQuat.GetRotated(data->grabInfo.clawSpaceDifference);

							data->targetPosition -= diffVec;
						}

						data->grabInfo.clear();
					}
				}
				else if(data->activeAction.action == ActionSwing)
				{
				}

				data->activeAction.action = ActionNone;
				// reset strength
				data->activeAction.strength = 100;
			}
			else
			{
				// Move claw ...
				data->targetPosition += dir * data->clawSettings.actionHitSpeed;
				data->targetHeight = data->restPosition.y;
			}
		}
	}

	// Height (not for static)
	if(!(data->grabInfo.actor && !data->grabInfo.actor->isDynamic()))
	{
		const MassEntry &massEntry = data->massSettings.getMassEntry(data->grabInfo);
		float step = massEntry.liftSpeed;

		if(data->targetPosition.y > data->targetHeight)
		{
			data->targetPosition.y -= step;
			if(data->targetPosition.y < data->targetHeight)
				data->targetPosition.y = data->targetHeight;
		}
		else
		{
			data->targetPosition.y += step;
			if(data->targetPosition.y > data->targetHeight)
				data->targetPosition.y = data->targetHeight;
		}
	}

	// Hax hax test
	if(data->ignoreActor && data->physicsLib)
	{
		NxScene *scene = data->physicsLib->getScene();

		if(scene && !data->actors.empty())
		{
			for(int j = 1; j < data->clawSettings.boxAmount; ++j)
				scene->setActorPairFlags(*data->actors[j].actor->getActor(), *data->ignoreActor, NX_IGNORE_PAIR);
		}
	}

	data->update();
}

void ClawController::syncClaw()
{
	data->syncClaw();
}

void ClawController::reloadConfig()
{
	data->parseStuff();
}

void ClawController::removeActor(NxActor *actor)
{
	if(data->grabInfo.actor == actor && !data->newClawObject)
	{
		if(data->grabInfo.joint)
			data->physicsLib->getScene()->releaseJoint(*data->grabInfo.joint);

		data->actors[data->clawSettings.boxAmount - 1].actor->setMass(data->clawSettings.clawMass);
		data->clawSettings.clawMaxDistance -= data->grabInfo.extraRange;

		// Revert object offset from target point
		{
			QUAT clawQuat;
			data->actors[data->clawSettings.boxAmount - 1].actor->getRotation(clawQuat);
			VC3 diffVec = clawQuat.GetRotated(data->grabInfo.clawSpaceDifference);

			data->targetPosition -= diffVec;
		}

		data->grabInfo.clear();
	}

	if(data->targetInfo.actor == actor)
	{
		data->targetInfo.clear();
	}

	for(unsigned int i = 0; i < data->collisionFades.size(); ++i)
	{
		if(actor == data->collisionFades[i].actor)
			data->collisionFades[i].actor = 0;
	}
}

bool ClawController::hasActor() const
{
	if(data->grabInfo.actor)
		return true;

	return false;
}

void ClawController::pickActor()
{
	if(!data->clawEnabled)
		return;

	if(!data->grabInfo.actor)
	{
		data->toggleGrab = true;
		data->autoGrabCounter = data->clawSettings.autoGrabTicks;
	}
}

void ClawController::dropActor()
{
	if(!data->clawEnabled)
		return;

	if(data->grabInfo.unit)
		data->grabInfo.unit->setOnGround(true);

	if(data->grabInfo.actor)
	{
		data->toggleGrab = true;
		data->releaseExtraSpeed = true;
	}
}

void ClawController::setGrabFlag(bool value)
{
	if(!data->clawEnabled)
		return;

	data->grabFlag = value;
}

void ClawController::setClawEnabled(bool value)
{
	data->clawEnabled = value;

	if(data->actors.size() >= (unsigned int)data->clawSettings.boxAmount)
	{
		for(int i = 1; i < data->clawSettings.boxAmount; ++i)
		{
			if(value)
			{
				// FIXME: Claw itself doesnt have collision for now
				if(i != data->clawSettings.boxAmount - 1)
					data->actors[i].actor->setCollisionGroup(PHYSICS_COLLISIONGROUP_CLAW);
				else
					//data->actors[i].actor->setCollisionGroup(PHYSICS_COLLISIONGROUP_CLAW);
					data->actors[i].actor->setCollisionGroup(PHYSICS_COLLISIONGROUP_NOCOLLISION);
			}
			else
				data->actors[i].actor->setCollisionGroup(PHYSICS_COLLISIONGROUP_NOCOLLISION);
		}
	}
}

void ClawController::setIgnoreActor(NxActor *actor)
{
	data->ignoreActor = actor;
}

void ClawController::setSprinting(bool enable)
{
	data->sprinting = enable;
}

/*
void ClawController::setTurbo(bool enable)
{
	if(enable && data->turboCounter <= -TURBO_COOLDOWN_TICKS)
	{
		data->turboCounter = TURBO_TICKS;

		data->turboVec = data->inputDelta;
		if(data->turboVec.GetLength() < 0.0001f)
		{
			VC3 dir = data->getClawTargetPosition() - data->model->GetPosition();
			dir.y = 0.f;
			if(dir.GetLength() > 0.0001f)
				dir.Normalize();

			data->turboVec.x = dir.x;
			data->turboVec.y = dir.z;
		}
		else
		{
			data->turboVec.Normalize();
		}

		data->turboVec *= 0.7928321376f;
		data->turboVec *= TURBO_CONTROLLER_FACTOR;

	}
}
*/
void ClawController::prepareAction(Action action)
{
	if(data->activeAction.action != ActionNone)
		return;
	if(!hasActor() && action == ActionThrow)
		return;

	if(action == ActionNone)
	{
		// Around 0.5 - 1.5 secs scale
		if(data->prepareAction != ActionNone && data->prepareActionCount > data->clawSettings.actionPrepareTicks)
		{
			data->activeAction.reset();
			data->activeAction.action = data->prepareAction;
			//data->activeAction.strength = data->prepareActionCount;
			//if(data->activeAction.strength > 100)
			//	data->activeAction.strength = 100;
			data->activeAction.strength = 100;

			// Get dir where to aim
			{
				VC3 dir = data->restPosition - data->model->GetPosition();
				dir.y = 0;
				float distance = dir.GetLength();
				if(distance > 0.001f)
					dir.Normalize();

				// Autoaim
				{
					const float maxAngleChange = data->clawSettings.maxAimedThrowAutoAimAdjust;
					VC3 autoaimDir;
					VC3 clawPosition = data->getClawTargetPosition();

					if(data->getAutoAimDirection(clawPosition, dir, maxAngleChange * 180.f / PI, 40.f, autoaimDir))
					{
						VC3 axis = dir.GetCrossWith(autoaimDir);
						if(axis.GetSquareLength() > 0.0001f)
						{
							axis.Normalize();
							float dot = dir.GetDotWith(autoaimDir);
							if(dot > 0)
							{
								float angle = acosf(dot);
								if(angle > maxAngleChange)
									angle = maxAngleChange;
								else if(angle < -maxAngleChange)
									angle = -maxAngleChange;

								QUAT q;
								q.MakeFromAxisRotation(axis, -angle);
								q.RotateVector(dir);
							}
						}
					}

					dir.y = sinf(data->throwBetaAngle);

					float strength_scale = powf(cosf(data->throwBetaAngle), data->clawSettings.aimedThrowBetaAngleForceScale);
					data->activeAction.strength = (int)(data->activeAction.strength * strength_scale);

					if(dir.GetLength() > 0.0001f)
						dir.Normalize();
				}

				data->activeAction.dir = dir;
			}
		}

		data->prepareActionCount = 0;
	}

	data->prepareAction = action;
}

void ClawController::setAction(TurboAction action)
{
	if(action == TurboThrow && !data->grabInfo.actor)
		return;

	data->turboAction = action;
	if(data->turboCounter <= -data->clawSettings.turboCooldownTicks)
	{
		data->turboCounter = data->clawSettings.turboTicks;

		data->turboVec = data->inputDelta;
		if(data->turboVec.GetLength() < 0.0001f)
		{
			VC3 clawPosition = data->getClawTargetPosition();
			VC3 dir = clawPosition - data->model->GetPosition();
			dir.y = 0.f;
			if(dir.GetLength() > 0.0001f)
				dir.Normalize();

			data->turboVec.x = dir.x;
			data->turboVec.y = dir.z;
		}
		else
		{
			data->turboVec.Normalize();
		}

		// Autoaim
		{
			const float maxAngleChange = data->clawSettings.maxTurboAutoAimAdjust;
			VC3 autoaimDir;
			VC3 dir(data->turboVec.x, 0, data->turboVec.y);
			VC3 clawPosition = data->getClawTargetPosition();

			if(data->getAutoAimDirection(clawPosition, dir, maxAngleChange * 180.f / PI, 40.f, autoaimDir))
			{
				VC3 axis = dir.GetCrossWith(autoaimDir);
				if(axis.GetSquareLength() > 0.0001f)
				{
					axis.Normalize();
					float dot = dir.GetDotWith(autoaimDir);
					if(dot > 0)
					{
						float angle = acosf(dot);
						if(angle > maxAngleChange)
							angle = maxAngleChange;
						else if(angle < -maxAngleChange)
							angle = -maxAngleChange;

						QUAT q;
						q.MakeFromAxisRotation(axis, -angle);
						q.RotateVector(dir);
					}
				}
			}

			data->turboVec.x = dir.x;
			data->turboVec.y = dir.z;
			if(data->turboVec.GetLength() > 0.0001f)
				data->turboVec.Normalize();
		}

		data->turboVec *= 0.7928321376f;
		data->turboVec *= data->clawSettings.turboControllerFactor;
	}
}

ClawController::Action ClawController::getPrepareAction() const
{
	return data->prepareAction;
}

void ClawController::setFlexPosition(const VC3 &point)
{
	if(!data->clawEnabled)
		return;

	data->flexPosition = point;

	if(hasActor())
		data->flexPosition.y += data->clawSettings.heightWithActor;
	else
		data->flexPosition.y += data->clawSettings.heightWithoutActor;

	//data->flexPosition.y += 2.f;
	data->flexPosition.y += 0.5f;
}

void ClawController::setRestPosition(const VC3 &point)
{
	if(!data->clawEnabled)
		return;

	data->restPosition = point;

	if(hasActor())
		data->restPosition.y += data->clawSettings.heightWithActor;
	else
		data->restPosition.y += data->clawSettings.heightWithoutActor;
}

void ClawController::moveTowards(const VC3 &point)
{
	if(!data->clawEnabled)
		return;
	//if(data->sprinting)
	//	return;

	data->mode = Data::TargetPoint;
	data->targetPosition.x = point.x;
	data->targetPosition.z = point.z;
	data->targetHeight = point.y;

	if(hasActor())
		data->targetHeight += data->clawSettings.heightWithActor;
	else
		data->targetHeight += data->clawSettings.heightWithoutActor;
}

void ClawController::moveByDelta(const VC2 &delta, float targetHeight)
{
	if(!data->clawEnabled)
		return;

	data->mode = Data::Force;

	if(!data->sprinting && data->prepareAction == ActionNone)
		data->inputDelta = delta;
	else
		data->inputDelta = VC2();

	if(data->turboCounter > 0 && data->model)
	{
		data->inputDelta = data->turboVec;
	}
	else if(data->restPoseCounter > 0)
	{
		VC3 distVec = data->restPosition - data->targetPosition;
		distVec.y = 0.f;
		float dist = distVec.GetLength();
		if(dist > 0.0001f)
			distVec.Normalize();

		distVec *= 0.7928321376f;

		data->inputDelta.x = distVec.x;
		data->inputDelta.y = distVec.z;

		// Hack: Remove some old movement going sideways
		{
			VC2 inputDelta = data->inputDelta;
			if(inputDelta.GetSquareLength() > 0.0001f)
				inputDelta.Normalize();
			VC2 speedDelta = data->speedDelta;
			if(speedDelta.GetSquareLength() > 0.0001f)
			{
				speedDelta.Normalize();

				float dot = speedDelta.GetDotWith(inputDelta);
				if(dot < 0.f)
					dot = 0.f;
				float factor = (dot * 0.2f) + 0.8f;
				data->speedDelta *= factor;
			}
		}

		{
			VC3 ref = data->model->GetPosition() - data->targetPosition;
			ref.y = 0.f;

			if(ref.GetDotWith(distVec) < 0)
			{
				data->speedDelta *= 0.f;
				data->restPoseCounter = 0;
			}
		}
		/*
		VC2 delta = data->speedDelta;
		delta *= 40;
		if(delta.GetLength() > dist)
		{
			data->inputDelta = VC2();
		}

		if(dist < 0.25f)
		{
			data->speedDelta *= 0.f;
			data->restPoseCounter = 0;
		}
		*/

		/*
		// Test
		VC2 delta = data->speedDelta;
		delta *= 20;
		if(delta.GetLength() < dist)
		{
			data->inputDelta.x = distVec.x;
			data->inputDelta.y = distVec.z;
		}
		else
		{
			data->inputDelta.x = 0.f;
			data->inputDelta.y = 0.f;
		}
		*/
	}


	data->targetHeight = targetHeight;
	if(hasActor())
		data->targetHeight += data->clawSettings.heightWithActor;
	else
		data->targetHeight += data->clawSettings.heightWithoutActor;
}

void ClawController::clampTargetClawDistance()
{
}

void ClawController::setTargetPosition(const VC3 &position)
{
	data->targetPosition = position;
	data->restPosition = position;

	if(hasActor())
		data->targetPosition.y += data->clawSettings.heightWithActor;
	else
		data->targetPosition.y += data->clawSettings.heightWithoutActor;
}

VC3 ClawController::getClawPosition() const
{
	VC3 position;

	if(data->actors.size() == data->clawSettings.boxAmount && data->joints.size() == data->clawSettings.boxAmount - 1)
	{
		data->actors[data->clawSettings.boxAmount - 1].actor->getMassCenterPosition(position);
	}

	return position;
}

VC3 ClawController::getClawVelocity() const
{
	VC3 vel;

	if(data->actors.size() == data->clawSettings.boxAmount && data->joints.size() == data->clawSettings.boxAmount - 1)
	{
		data->actors[data->clawSettings.boxAmount - 1].actor->getVelocity(vel);
	}

	return vel;
}

VC3 ClawController::getTargetPosition(int advanceTicks) const
{
	/*
	VC3 target = data->targetPosition;
	if(data->prepareAction != ClawController::ActionNone)
		target = data->flexPosition;
	return target;
	*/

	//VC3 pos = data->targetPosition;
	VC3 pos = data->getClawTargetPosition();

	pos.x += data->speedDelta.x * float(advanceTicks);
	pos.z += data->speedDelta.y * float(advanceTicks);

	if(data->restPoseCounter > 0)
	{
		pos = data->restPosition;
	}
	if(data->targetInfo.hasTarget())
	{
		pos = toVC3(Data::getActorCMassGlobalPosition(data->targetInfo.actor));
	}
	if(data->prepareAction != ClawController::ActionNone)
		pos = data->restPosition;
	if(data->activeAction.action != ClawController::ActionNone)
	{
		pos = data->restPosition;

		// Test for better animation response
		VC3 dir = data->restPosition - data->model->GetPosition();
		dir.y = 0.f;
		if(dir.GetLength() > 0.0001f)
			dir.Normalize();

		pos += dir * 3.f;
	}

	return pos;
}

void ClawController::moveToRestPosition()
{
	data->restPoseCounter = data->clawSettings.restPoseTicks;
}

const ControlSettings &ClawController::getControlSettings() const
{
	return data->controlSettings;
}

VC3 ClawController::getDirection() const
{
	VC3 dir = data->restPosition - data->model->GetPosition();
	dir.y = 0;
	float distance = dir.GetLength();
	if(distance > 0.001f)
		dir.Normalize();
	return dir;
}

VC3 ClawController::getRestPosition() const
{
	return data->restPosition;
}

VC3 ClawController::getModelPosition() const
{
	return data->model->GetPosition();
}

bool ClawController::isClawMoving() const
{
	if(data->activeAction.action != ActionNone || data->prepareAction != ActionNone)
		return true;

	if(data->speedDelta.GetSquareLength() < 0.01f)
		return false;

	return true;
}

int ClawController::getTerrainObjectInstanceId() const
{
	return data->grabInfo.terrObjInstanceId;
}

int ClawController::getTerrainObjectModelId() const
{
	return data->grabInfo.terrObjModelId;
}

float ClawController::getActorMass() const
{
	if(!data->grabInfo.actor)
		return 0.0f;
	return (float)data->grabInfo.actorMass;
}

void ClawController::setActor( NxActor * actor )
{
	data->grabInfo.actor = actor;
}

bool ClawController::isClawObject(int modelId, int objectId) const
{
	if(hasActor() && modelId == data->grabInfo.terrObjModelId && objectId == data->grabInfo.terrObjInstanceId)
		return true;

	return false;
}

void ClawController::setNewClawObject(boost::shared_ptr<game::AbstractPhysicsObject> physicsObject)
{
	data->newClawObject = physicsObject;
	data->newClawObjectDelay = 0;
}

float ClawController::getClawLength() const
{
	return data->clawSettings.clawLength;
}

VC3 ClawController::getRootPosition() const
{
	return data->rootPosition;
}

unsigned int ClawController::getNumClawPositions() const
{
	return data->lastPositions.size();
}

VC3 ClawController::getLastClawPosition(unsigned int i) const
{
	if(i < data->lastPositions.size())
	{
		return data->lastPositions[i];
	}
	return VC3(0,0,0);
}

float ClawController::getLastClawGrabMass(unsigned int i) const
{
	if(i < data->lastMasses.size())
	{
		return data->lastMasses[i];
	}
	return 0.0f;
}

void ClawController::setThrowBetaAngle(float angle)
{
	data->throwBetaAngle = angle;

	if(data->throwBetaAngle > data->clawSettings.maxAimedThrowBetaAngle)
		data->throwBetaAngle = data->clawSettings.maxAimedThrowBetaAngle;
	else if(data->throwBetaAngle < data->clawSettings.minAimedThrowBetaAngle)
		data->throwBetaAngle = data->clawSettings.minAimedThrowBetaAngle;
}

float ClawController::getThrowBetaAngle() const
{
	return data->throwBetaAngle;
}

void ClawController::getThrowBetaAngleLimits(float &minAngle, float &maxAngle) const
{
	maxAngle = data->clawSettings.maxAimedThrowBetaAngle;
	minAngle = data->clawSettings.minAimedThrowBetaAngle;
}


} // game
//} // frozenbyte
