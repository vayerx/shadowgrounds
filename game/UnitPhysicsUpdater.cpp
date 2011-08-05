
#include "precompiled.h"

#include "UnitPhysicsUpdater.h"
#include "Unit.h"
#include "UnitType.h"
#include "unittypes.h"
#include "physics/GamePhysics.h"
#include "physics/BoxPhysicsObject.h"
#include "physics/CapsulePhysicsObject.h"
#include "physics/PhysicsContactUtils.h"
#include "options/options_physics.h"
#include "SimpleOptions.h"
#include "scaledefs.h"
#include "physics/physics_collisiongroups.h"
#include "../util/ObjectDurabilityParser.h"
#include <assert.h>

#ifdef PROJECT_CLAW_PROTO
#include "ClawController.h"
#include "Game.h"
#endif

// TEMP HACK
#ifdef PHYSICS_ODE
#include "options/options_physics.h"
#include "IStorm3D_Scene.h"
extern IStorm3D_Scene *disposable_scene;
#endif


namespace game
{
	static int unitphys_stat_amount = 0;
	static int unitphys_stat_setposition = 0;

	void UnitPhysicsUpdater::startStats()
	{
		unitphys_stat_amount = 0;
		unitphys_stat_setposition = 0;
	}

	void UnitPhysicsUpdater::endStats()
	{
		// nop
	}

#define UPU_PHYSOBJTYPE_GAME 0
#define UPU_PHYSOBJTYPE_FLUID_CONTAINMENT 1        

	void UnitPhysicsUpdater::createPhysics(Unit *unit, GamePhysics *physics, Game *game)
	{
		assert(unit->getGamePhysicsObject() == NULL);

		// normal physics object and fluid containment physics object...
		for (int physobjtype = 0; physobjtype < 2; physobjtype++)
		{
			if ((physobjtype == UPU_PHYSOBJTYPE_GAME && unit->getUnitType()->hasPhysicsObject())
				|| (physobjtype == UPU_PHYSOBJTYPE_FLUID_CONTAINMENT 
					&& unit->getUnitType()->hasFluidContainmentPhysicsObject() 
					&& SimpleOptions::getBool(DH_OPT_B_PHYSICS_FLUIDS_ENABLED)))
			{
				const VC3 &pos = unit->getPosition();

				UnitType *ut = unit->getUnitType();
				game::AbstractPhysicsObject *physobj = NULL;
				int collGroup = ut->getPhysicsObjectCollisionGroup();
				if (physobjtype == UPU_PHYSOBJTYPE_FLUID_CONTAINMENT)
				{
					collGroup = PHYSICS_COLLISIONGROUP_FLUID_CONTAINMENT_PLANES;
				}
				if (ut->getPhysicsObjectType() == UNITTYPE_PHYSICS_OBJECT_TYPE_CAPSULE)
				{
					float radius = ut->getPhysicsObjectSizeX();
					float height = ut->getPhysicsObjectSizeY();

					if (ut->getPhysicsObjectImplementationType() == UNITTYPE_PHYSICS_OBJECT_IMPLEMENTATION_TYPE_SIDEWAYS)
					{
						height = ut->getPhysicsObjectSizeZ();
					}

					// HACK: fluid containment is radius + 0.1 and height + 0.5
					if (physobjtype == UPU_PHYSOBJTYPE_FLUID_CONTAINMENT)
					{
						radius += 0.1f;
						height += 0.5f;
					}

	#ifdef PHYSICS_NONE
					physobj = new game::BoxPhysicsObject(physics, VC3(ut->getPhysicsObjectSizeX(),ut->getPhysicsObjectSizeY(),ut->getPhysicsObjectSizeZ()), ut->getPhysicsObjectMass(), collGroup, pos);
	#else
					physobj = new game::CapsulePhysicsObject(physics, height, radius, ut->getPhysicsObjectMass(), collGroup, pos);
	#endif
				}
				if (ut->getPhysicsObjectType() == UNITTYPE_PHYSICS_OBJECT_TYPE_BOX)
				{
					physobj = new game::BoxPhysicsObject(physics, VC3(ut->getPhysicsObjectSizeX(),ut->getPhysicsObjectSizeY(),ut->getPhysicsObjectSizeZ()), ut->getPhysicsObjectMass(), collGroup, pos);
					if (unit->getUnitType()->isPhysicsObjectDoor())
					{
						VC3 urot = unit->getRotation();
						physobj->setRotation(QUAT(UNIT_ANGLE_TO_RAD(urot.x),UNIT_ANGLE_TO_RAD(urot.y),UNIT_ANGLE_TO_RAD(urot.z)));
					}
				}

				if (physobj != NULL)
				{
					// WARNING: unsafe int to void * cast!
					physobj->setCustomData((void *)PhysicsContactUtils::calcCustomPhysicsObjectDataForUnit(game, unit));

					util::ObjectDurabilityParser durp;
					int durtype = durp.getDurabilityTypeIndexByName("unit");
					physobj->setDurabilityType(durtype);

					if (physobjtype == UPU_PHYSOBJTYPE_FLUID_CONTAINMENT)
					{
						unit->setFluidContainmentPhysicsObject(physobj);
					} else {
						unit->setGamePhysicsObject(physobj);
					}
					if (ut->hasPhysicsObjectDisabledAngularVelocity())
					{
						physobj->disableAngularVelocity();
					}
					if (ut->hasPhysicsObjectDisabledYMovement())
					{
						physobj->disableYMovement();
					}
#ifdef PHYSICS_FEEDBACK
					physobj->setFeedbackEnabled(true);
#endif
				}
			}
		}
	}

	void UnitPhysicsUpdater::deletePhysics(Unit *unit, GamePhysics *physics)
	{
		if (unit->getFluidContainmentPhysicsObject() != NULL)
		{
			delete unit->getFluidContainmentPhysicsObject();
			unit->setFluidContainmentPhysicsObject(NULL);
		}
		if (unit->getGamePhysicsObject() != NULL)
		{
			delete unit->getGamePhysicsObject();
			unit->setGamePhysicsObject(NULL);
			unit->setPhysicsObjectDifference(0.0f);
		}

	}

	/**
	 * This is the "sideways" type physics implementation.
	 * May be useful for some topdown objects too.
	 */
	static void updatePhysicsSidewaysImpl(Unit *unit, GamePhysics *physics, int gameTickTimer)
	{
		assert(unit->getUnitType()->getPhysicsObjectImplementationType() == UNITTYPE_PHYSICS_OBJECT_IMPLEMENTATION_TYPE_SIDEWAYS);
		assert(unit->getGamePhysicsObject() != NULL);

		UnitType *ut = unit->getUnitType();

		game::AbstractPhysicsObject *bp = unit->getGamePhysicsObject();

		VC3 physPos = bp->getMassCenterPosition();

		float offz = ut->getPhysicsObjectHeightOffset();

		if (ut->getPhysicsObjectType() == UNITTYPE_PHYSICS_OBJECT_TYPE_CAPSULE)
		{
			offz += ut->getPhysicsObjectSizeZ() * 0.5f + ut->getPhysicsObjectSizeX();
		} else {
			// height
			offz += ut->getPhysicsObjectSizeZ();
		}

		// HACK: raise up the physics object a bit if on slope
		if (unit->isOnSlope())
		{
			offz += 0.3f;
		}

		VC3 unitPosWHeight = unit->getPosition() + VC3(0, 2.0f, offz - 0.5f);

		unit->setOnPhysicsObject(false);

		VC3 diff = unitPosWHeight - physPos;
		unit->setPhysicsObjectDifference(diff.GetLength());
		if (diff.GetSquareLength() > ut->getPhysicsObjectWarpThreshold())
		{
			bp->setPosition(unitPosWHeight);
			bp->setVelocity(VC3(0,0,0));
			bp->setRotation(QUAT(0,0,0));

			unitphys_stat_setposition++;
		} else {
			float threshold = ut->getPhysicsObjectImpulseThreshold();
			VC3 vel = unit->getVelocity();
			bool setposx = false;
			bool setposz = false;
			if (fabs(diff.x) > threshold)
			{
				if (diff.x > 0) 
				{
#ifdef PHYSICS_FEEDBACK
					// -0.86 normal.x... is about 60 deg left...
					if (bp->getFeedbackNormalLeft().x < -0.86f)
#else
					if (true)
#endif
					{
//char buf[256];
//sprintf(buf, "L: %f", bp->getFeedbackNormalLeft().x);
//Logger::getInstance()->error(buf);
						setposx = true;
						diff.x -= threshold; 
						if (vel.x > 0) vel.x = -0.01f;
					}
				} 
				if (diff.x < 0)
				{
#ifdef PHYSICS_FEEDBACK
					// 0.86 normal.x... is about 60 deg right...
					if (bp->getFeedbackNormalRight().x > 0.86f)
#else
					if (true)
#endif
					{
//char buf[256];
//sprintf(buf, "R: %f", bp->getFeedbackNormalRight().x);
//Logger::getInstance()->error(buf);
						setposx = true;
						diff.x += threshold;
						if (vel.x < 0) vel.x = 0.01f;
					}
				}
			}
			if (diff.z > threshold)
			{
#ifdef PHYSICS_FEEDBACK
				// -0.707 normal.z... is about 45 deg downward...
				// NOTE: upward feedback is preferred, so ofter, this may not be true!
				if (bp->getFeedbackNormal().z < -0.7f)
#else
				if (true)
#endif
				{
					if (vel.z > 0)
					{
						vel.z = 0;
					}
				}
				diff.z -= threshold;
			}

			if (diff.z < -threshold)
			{
#ifdef PHYSICS_FEEDBACK

//Logger::getInstance()->error(int2str((100.0f * bp->getFeedbackNormal().z)));

				// 0.707 normal.z... is about 45 deg upward...
				if (bp->getFeedbackNormal().z > 0.7f)
#else
				if (true)
#endif
				{	
					setposz = true;
#ifdef PHYSICS_FEEDBACK
					if (bp->getFeedbackNormal().z > 0.7f)
#endif
					{
						unit->setOnPhysicsObject(true);
					}

					if (vel.z < 0)
					{
						vel.z = 0;
					}
				}
				diff.z += threshold;
			} 
			if (unit->isPhysicsObjectFeedbackEnabled())
			{				
				unit->setVelocity(vel);
				if (diff.GetSquareLength() > 0.05f * 0.05f && (setposx || setposz))
				{
					UnitActor *ua = getUnitActorForUnit(unit);
					ua->removeUnitObstacle(unit);
					VC3 upos = unit->getPosition();
					VC3 feedbackdiff = diff;
						feedbackdiff.x *= 0.2f;
					if (!setposx)
						feedbackdiff.x = 0;
					if (!setposz)
						feedbackdiff.z = 0;
//					if (!setposz)
//						feedbackdiff.z *= 0.2f;
					upos -= feedbackdiff * ut->getPhysicsObjectImpulseFeedbackFactor();
					unit->setPosition(upos);
					unit->setWaypoint(upos);
					unit->setFinalDestination(upos);
					ua->addUnitObstacle(unit);
				}
			}

			if (setposx || setposz)
			{
				bp->setVelocity(diff * ut->getPhysicsObjectImpulseFactor() * (1.0f - ut->getPhysicsObjectImpulseFeedbackFactor()));
			} else {
				if (fabs(unit->getVelocity().x) < 0.01f)
				{
					diff.x *= 0.5;
				}
				bp->setVelocity(diff * ut->getPhysicsObjectImpulseFactor());
			}
			bp->setRotation(QUAT(0,0,0));
		}

#ifdef PHYSICS_ODE
// TEMP HACK:
if (SimpleOptions::getBool(DH_OPT_B_PHYSICS_VISUALIZE_COLLISION_SHAPES))
{
	VC3 sizes = VC3(ut->getPhysicsObjectSizeX(),ut->getPhysicsObjectSizeY(),ut->getPhysicsObjectSizeZ());

	VC3 c1 = VC3(physPos.x - sizes.x, physPos.y + sizes.y, physPos.z - sizes.z);
	VC3 c2 = VC3(physPos.x + sizes.x, physPos.y + sizes.y, physPos.z - sizes.z);
	VC3 c3 = VC3(physPos.x + sizes.x, physPos.y + sizes.y, physPos.z + sizes.z);
	VC3 c4 = VC3(physPos.x - sizes.x, physPos.y + sizes.y, physPos.z + sizes.z);
	VC3 cb1 = VC3(physPos.x - sizes.x, physPos.y - sizes.y, physPos.z - sizes.z);
	VC3 cb2 = VC3(physPos.x + sizes.x, physPos.y - sizes.y, physPos.z - sizes.z);
	VC3 cb3 = VC3(physPos.x + sizes.x, physPos.y - sizes.y, physPos.z + sizes.z);
	VC3 cb4 = VC3(physPos.x - sizes.x, physPos.y - sizes.y, physPos.z + sizes.z);
	COL col = COL(1,0,0);
	/*
	disposable_scene->AddTriangle(c4, c2, c1, col);
	disposable_scene->AddTriangle(c4, c3, c2, col);
	disposable_scene->AddTriangle(cb1, cb2, cb4, col);
	disposable_scene->AddTriangle(cb2, cb3, cb4, col);
	*/
	disposable_scene->AddLine(c1, c2, col);
	disposable_scene->AddLine(c2, c3, col);
	disposable_scene->AddLine(c3, c4, col);
	disposable_scene->AddLine(c4, c1, col);
	disposable_scene->AddLine(cb1, cb2, col);
	disposable_scene->AddLine(cb2, cb3, col);
	disposable_scene->AddLine(cb3, cb4, col);
	disposable_scene->AddLine(cb4, cb1, col);
	disposable_scene->AddLine(c1, cb1, col);
	disposable_scene->AddLine(c2, cb2, col);
	disposable_scene->AddLine(c3, cb3, col);
	disposable_scene->AddLine(c4, cb4, col);
}
#endif
	}

	/**
	 * Runs the normal type physics implementation or calls to other implementation.
	 */
	void UnitPhysicsUpdater::updatePhysics(Unit *unit, GamePhysics *physics, int gameTickTimer)
	{
		// TODO: optimize: fluid containment has no need for update, if we're not near any fluid effect
		// (however, we don't have any easily accessible info on the whereabouts of fluid effects, thus
		// cannot easily optimize this)

		for (int physobjtype = 0; physobjtype < 2; physobjtype++)
		{
			if ((physobjtype == UPU_PHYSOBJTYPE_GAME && unit->getGamePhysicsObject() != NULL)
				|| (physobjtype == UPU_PHYSOBJTYPE_FLUID_CONTAINMENT && unit->getFluidContainmentPhysicsObject() != NULL))
			{
				// is destroyed, then maybe should delete the physics object?
				if (unit->isDestroyed())
				{
					if (!unit->getUnitType()->hasPhysicsObjectIfDestroyed())
					{
#ifdef PROJECT_CLAW_PROTO
// add a little hack delay here??
// also don't delete physics until the unit has stopped.
if (unit->variables.getVariable("proning") > 30 &&
	unit->getGamePhysicsObject()->getVelocity().GetSquareLength() < 0.1f*0.1f )
{
		deletePhysics(unit, physics);
		return;
} else {
	unit->variables.setVariable("proning", unit->variables.getVariable("proning") + 1);
}
#else
						deletePhysics(unit, physics);
						return;
#endif
					}
				}

				unitphys_stat_amount++;

				if (unit->getUnitType()->getPhysicsObjectImplementationType() == UNITTYPE_PHYSICS_OBJECT_IMPLEMENTATION_TYPE_SIDEWAYS)
				{
					updatePhysicsSidewaysImpl(unit, physics, gameTickTimer);
					return;
				}

				UnitType *ut = unit->getUnitType();

				game::AbstractPhysicsObject *bp = unit->getGamePhysicsObject();
				if (physobjtype == UPU_PHYSOBJTYPE_FLUID_CONTAINMENT)
				{
					bp = unit->getFluidContainmentPhysicsObject();
				}

				//VC3 physPos = bp->getMassCenterPosition();
				VC3 physPos = bp->getPosition();


				if (unit->isPhysicsObjectLock()
					&& physobjtype == UPU_PHYSOBJTYPE_GAME)
				{
					UnitActor *ua = getUnitActorForUnit(unit);
					ua->removeUnitObstacle(unit);
					//VC3 upos = unit->getPosition();
					VC3 upos = physPos;
					unit->setPosition(upos);
					unit->setWaypoint(upos);
					unit->setFinalDestination(upos);
					unit->setVelocity(VC3(0,0,0));
					
					// TODO: phys obj. rotation (quat) to unit rotation (eul angles)
					//unit->setRotation(...);
					ua->addUnitObstacle(unit);

					continue;
				}


				float offy = ut->getPhysicsObjectHeightOffset();
				if (ut->getPhysicsObjectType() == UNITTYPE_PHYSICS_OBJECT_TYPE_CAPSULE)
				{
	#ifdef PHYSICS_PHYSX
					// do nothing.
	#else
					// height/2 + radius
					offy += ut->getPhysicsObjectSizeY() * 0.5f + ut->getPhysicsObjectSizeX();
	#endif
				} else {
					// height
	#ifdef PHYSICS_ODE
					offy += ut->getPhysicsObjectSizeY();
	#endif
				}

				// HACK: fluid containment is at -0.4 height
				if (physobjtype == UPU_PHYSOBJTYPE_FLUID_CONTAINMENT)
				{
					offy -= 0.4f;
				}

				VC3 unitPosWHeight = unit->getPosition() + VC3(0,offy,0);

				float threshold = ut->getPhysicsObjectImpulseThreshold();
				if (unit->getVelocity().GetSquareLength() > (threshold*10.0f)*(threshold*10.0f))
				{
					unitPosWHeight += unit->getVelocity() * 2.0f;
				}

				if( Unit::getVisualizationOffsetInUse() )
				{
					unitPosWHeight.y += ( unit->getVisualizationOffsetInterpolation() * Unit::getVisualizationOffset() );
				}

				if (ut->isPhysicsObjectDoor())
				{
					if (!ut->hasBothLineSides())
					{
						VC3 urot = unit->getRotation();
						bool rightside = unit->isUnitMirrorSide();
						if (rightside)
						{
							urot.y += 180.0f;
							if (urot.y >= 360.0f)
								urot.y -= 360.0f;
						}
						float angRad = UNIT_ANGLE_TO_RAD(urot.y);
						float dx = -sinf(angRad - 3.141592f/2);
						float dz = -cosf(angRad - 3.141592f/2);
						float offsetLen = ut->getPhysicsObjectSizeX();
						unitPosWHeight.x += dx * offsetLen;
						unitPosWHeight.z += dz * offsetLen;
					}
				}

				VC3 diff = unitPosWHeight - physPos;

				// FIXME: other units won't get the difference because of this check
				// however, this is a nice little optimization, as other units don't really need the info
				// (should perhaps have some update diff flag, that would be set by the script command
				// to allow other units to have this info too, although, the very first call would still have incorrect zero value)
				if (ut->isPhysicsObjectDoor())
				{
					unit->setPhysicsObjectDifference(diff.GetLength());
				}

				if (diff.GetSquareLength() > ut->getPhysicsObjectWarpThreshold())
				{
					bp->setPosition(unitPosWHeight);
					bp->setVelocity(VC3(0,0,0));
					if (unit->getUnitType()->isPhysicsObjectDoor())
					{
						VC3 urot = unit->getRotation();
						bp->setRotation(QUAT(UNIT_ANGLE_TO_RAD(urot.x),UNIT_ANGLE_TO_RAD(urot.y),UNIT_ANGLE_TO_RAD(urot.z)));
					} else {
						bp->setRotation(QUAT(0,0,0));
					}

					unitphys_stat_setposition++;
				} else {
	// TODO: could this be done on every tick?
	// would be more stable? but causes wrenchers to sometimes warp inside boxes, etc?
					if ((gameTickTimer & 1) == 0)
					{
						bp->setVelocity(VC3(0,0,0));
					}

					// HACK: umm... cannot really disable y movement, otherwise cannot walk on slopes, etc.
					// so just trying to enable/disable it "every once a while"...
					if (ut->hasPhysicsObjectDisabledYMovement())
					{
						if ((gameTickTimer & 7) == 0)
						{
							if (fabs(physPos.y - unitPosWHeight.y) > 0.2f)
							{
								// NOTE: this actually assumes that the GamePhysics sync is handled in a specific way..
								// these calls will first enable Y, then move position, then disable Y
								// not because there are called in this order, but because AbstractPhysicsObject does them in that
								// specific order.
								bp->enableYMovement();
								bp->disableYMovement();
								VC3 hackpos = physPos;
								hackpos.y = unitPosWHeight.y;
								bp->setPosition(hackpos);
							}
						} 
					}

					if (diff.GetSquareLength() > threshold*threshold)
					{
						bp->addImpulse(bp->getMassCenterPosition(), diff * GAME_TICKS_PER_SECOND * ut->getPhysicsObjectImpulseFactor());
						if (unit->isPhysicsObjectFeedbackEnabled())
						{
							//VC3 newvel = -diff * ut->getPhysicsObjectImpulseFeedbackFactor();
							//newvel += unit->getVelocity();
							//unit->setVelocity(newvel);
	//if (unit->getVelocity().GetSquareLength() > 0.1f * 0.1f)
	//{
							if (diff.GetSquareLength() > (threshold*10.0f)*(threshold*10.0f))
							{
								unit->setVelocity(-diff * ut->getPhysicsObjectImpulseFeedbackFactor());
							}
	//}
						}
					}
				}
			}
		}
	}

	char *UnitPhysicsUpdater::getStatusInfo()
	{
		char *ret = new char[65536+1];
		sprintf(ret, "Unit physics info follows:\r\n%d acted units with physics object\r\n%d units forcibly set physics object position during last tick\r\n", unitphys_stat_amount, unitphys_stat_setposition);
		return ret;
	}

	void UnitPhysicsUpdater::deleteStatusInfo(char *buf)
	{
		delete [] buf;
	}

}

