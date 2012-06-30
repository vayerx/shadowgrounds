#include "precompiled.h"

#include "PhysicsScripting.h"

#include "scripting_macros_start.h"
#include "physics_script_commands.h"
#include "scripting_macros_end.h"

#include "../Game.h"
#include "../GameUI.h"
#include "../UnifiedHandleManager.h"
#include "../unified_handle.h"
#include "GameScriptingUtils.h"
#include "GameScriptData.h"
#include "../../util/ScriptProcess.h"
#include "../../util/Script.h"
#include "../../util/ScriptManager.h"
#include "../../system/Logger.h"
#include "../../convert/str2int.h"
#include "../../ui/Terrain.h"

#include "../../convert/int64_to_hex.h"
#include "../physics/gamephysics.h"

#ifdef PHYSICS_PHYSX
#  include "../../physics/physics_lib.h"
#  include "../../util/StringUtil.h"
#endif

#include "../../util/Debug_MemoryManager.h"

using namespace ui;

namespace game
{
    // adding joints? what type?
    static bool physicsscripting_adding_joint = false;
    static std::string physicsscripting_adding_joint_type = "";

    static bool physicsscripting_joint_param_name_given = false;
    static std::string physicsscripting_setting_param_name = "";

    // for "physics" type...
    //static frozenbyte::physics::JointProperties physicsscripting_joint_physics_props;

    // for "rope" type...
    //static ...

    namespace {
        struct JointParameters {
            std::string   objectA;
            std::string   objectB;
            UnifiedHandle objectAUH;
            UnifiedHandle objectBUH;

            std::string   id;

            VC3   position;
            VC3   axis;
            VC3   normal;

            VC3   lowAngle;
            VC3   highAngle;
            VC3   lowSpring;
            VC3   highSpring;
            VC3   lowDamping;
            VC3   highDamping;

            float breakForce;
            float normalAngle;

            JointParameters()
                :   breakForce(0),
                normalAngle(0),
                objectAUH(UNIFIED_HANDLE_NONE),
                objectBUH(UNIFIED_HANDLE_NONE)
            {
            }
        };

        JointParameters jointParameters;
    } // unnamed

    void PhysicsScripting::process(util::ScriptProcess *sp,
                                   int command, int intData, char *stringData, ScriptLastValueType *lastValue,
                                   GameScriptData *gsd, Game *game)
    {
        float floatData = *( (float *)&intData );

        switch (command) {
        case GS_CMD_physSetGroupColl:
        case GS_CMD_physSetGroupCont:
#ifdef PHYSICS_PHYSX
            if (stringData != NULL) {
                std::string str = stringData;
                std::vector<std::string> splitted = util::StringSplit(",", str);

                if (splitted.size() == PHYSICS_MAX_COLLISIONGROUPS + 1) {
                    std::string rowstr = util::StringRemoveWhitespace(splitted[0]);
                    int row = str2int( rowstr.c_str() );

                    if (row >= 0 && row < PHYSICS_MAX_COLLISIONGROUPS)
                        for (int col = 0; col < PHYSICS_MAX_COLLISIONGROUPS; col++) {
                            std::string colstr = util::StringRemoveWhitespace(splitted[1 + col]);
                            int value = 0;
                            if (colstr != "-")
                                value = str2int( colstr.c_str() );
                            if (command == GS_CMD_physSetGroupCont)
                                frozenbyte::physics::physicslib_group_cont[row][col] = value;
                            else
                                frozenbyte::physics::physicslib_group_coll[row][col] = (value != 0 ? true : false);
                        }
                    else
                        sp->error(
                            "PhysicsScripting::process - physSetGroupColl/Cont parameter invalid, parameter does not start with a valid collision group value.");
                } else {
                    sp->error(
                        "PhysicsScripting::process - physSetGroupColl/Cont parameter invalid, wrong number of entries.");
                }

            } else {
                if (command == GS_CMD_physSetGroupCont)
                    sp->warning("PhysicsScripting::process - physSetGroupCont parameter missing.");
                else
                    sp->warning("PhysicsScripting::process - physSetGroupColl parameter missing.");
            }
#else
            sp->warning(
                "PhysicsScripting::process - physSetGroupColl/Cont not supported by used physics implementation.");
#endif
            break;

        case GS_CMD_jointAttachToUnifiedHandleObject:
            if (physicsscripting_adding_joint) {
                if ( VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle) ) {
                    if ( game->unifiedHandleManager->doesObjectExist(gsd->unifiedHandle) ) {
                        // NOTE: for now, only terrain objects will be supported. ok...?
                        if ( IS_UNIFIED_HANDLE_TERRAIN_OBJECT(gsd->unifiedHandle) ) {
                            if (physicsscripting_adding_joint_type == "physics") {
                                // TODO: ...
                            } else if (physicsscripting_adding_joint_type == "rope") {
                                // TODO: ...
                                // adding_rope->attachTo(...)
                            } else {
                                sp->error(
                                    "PhysicsScripting::process - jointAttachToUnifiedHandleObject, joint type unsupported.");
                            }
                        } else {
                            sp->error(
                                "PhysicsScripting::process - jointAttachToUnifiedHandleObject, object with given unified handle is not a terrain object.");
                        }
                    } else {
                        sp->error(
                            "PhysicsScripting::process - jointAttachToUnifiedHandleObject, unified handle object does not exist.");
                    }
                } else {
                    sp->error("PhysicsScripting::process - jointAttachToUnifiedHandleObject, invalid unified handle.");
                }
            } else {
                sp->error(
                    "PhysicsScripting::process - jointAttachToUnifiedHandleObject, was not adding a joint (jointAddStart expected before jointAttachToUnifiedHandleObject).");
            }
            break;

        case GS_CMD_jointSetParameterName:
            if (stringData != NULL) {
                if (physicsscripting_adding_joint) {
                    if (physicsscripting_joint_param_name_given)
                        sp->error(
                            "PhysicsScripting::process - jointSetParameterName, name set command without setting value for previous one first.");
                    physicsscripting_joint_param_name_given = true;

                    bool paramOk = false;
                    if (physicsscripting_adding_joint_type == "physics") {
                        // TODO: check for appropriate parameter names
                        //physicsscripting_setting_param_name = stringData;
                        //paramOk = true;
                    } else if (physicsscripting_adding_joint_type == "rope") {
                        // TODO: check for appropriate parameter names
                        //physicsscripting_setting_param_name = stringData;
                        //paramOk = true;
                    } else {
                        sp->error("PhysicsScripting::process - jointSetParameterName, joint type unsupported.");
                        paramOk = true; // (don't give the below parameter error, we've already given an error)
                    }
                    if (!paramOk)
                        sp->error(
                            "PhysicsScripting::process - jointSetParameterName parameter bad (parameter name was unknown or invalid for joint type).");
                } else {
                    sp->error(
                        "PhysicsScripting::process - jointSetParameterName, was not adding a joint (jointAddStart expected before jointSetParameterName).");
                }
            } else {
                sp->error(
                    "PhysicsScripting::process - jointSetParameterName parameter missing (joint parameter name expected).");
            }
            break;

        case GS_CMD_jointSetParameterValue:
            if (physicsscripting_joint_param_name_given) {
                physicsscripting_joint_param_name_given = false;

                if (physicsscripting_adding_joint_type == "physics") {
                    // TODO: set param
                    // something like...
                    // if (physicsscripting_setting_param_name == "restitution")
                    //        physicsscripting_joint_physics_props.restitution = floatValue;
                } else if (physicsscripting_adding_joint_type == "rope") {
                    // TODO: set param
                } else {
                    sp->error("PhysicsScripting::process - jointSetParameterName, joint type unsupported.");
                }
            } else {
                sp->error(
                    "PhysicsScripting::process - jointSetParameterValue, name was not set (jointSetParameterName expected before value).");
            }
            break;

        case GS_CMD_jointAddStart:
            if (stringData != NULL) {
                if (strcmp(stringData, "physics") == 0)
                    physicsscripting_adding_joint = true;
                    //physicsscripting_joint_physics_props = ...
                else if (strcmp(stringData, "rope") == 0)
                    physicsscripting_adding_joint = true;
                    // ...
                else
                    sp->error("PhysicsScripting::process - jointAddStart parameter bad (joint type name expected).");
            } else {
                sp->error("PhysicsScripting::process - jointAddStart parameter missing (joint type name expected).");
            }
            break;

        case GS_CMD_jointAddDone:
            if (physicsscripting_adding_joint) {
                if (physicsscripting_adding_joint_type == "physics") {
                    //game->jointManager->addJoint(new PhysicsJoint(physicsscripting_joint_physics_props));
                } else if (physicsscripting_adding_joint_type == "rope") {
                    //game->jointManager->addJoint(new RopeJoint(...));
                }
                physicsscripting_adding_joint = false;
            } else {
                sp->error(
                    "PhysicsScripting::process - jointAddDone, was not adding a joint (jointAddStart expected before done).");
            }
            break;

        // --- Joints

        case GS_CMD_setJointObjectA: {
            if (stringData)
                jointParameters.objectA = stringData;
            else
                sp->error("PhysicsScripting::process - stringData missing from setJointObjectA.");
        }
        break;

        case GS_CMD_setJointObjectB: {
            if (stringData)
                jointParameters.objectB = stringData;
            else
                sp->error("PhysicsScripting::process - stringData missing from setJointObjectB.");
        }
        break;

        case GS_CMD_setJointTwistLimits: {
            if (stringData) {
                if (sscanf(stringData, "%f,%f", &jointParameters.lowAngle.x, &jointParameters.highAngle.x) != 2)
                    sp->error("PhysicsScripting::process - invalid parameters for setJointTwistLimits");
            } else {
                sp->error("PhysicsScripting::process - stringData missing from setJointTwistLimits");
            }
        }
        break;

        case GS_CMD_setJointTwistLowSpring: {
            if (stringData) {
                if (sscanf(stringData, "%f,%f", &jointParameters.lowSpring.x, &jointParameters.lowDamping.x) != 2)
                    sp->error("PhysicsScripting::process - invalid parameters for setJointTwistLowSpring");
            } else {
                sp->error("PhysicsScripting::process - stringData missing from setJointTwistLowSpring");
            }
        }
        break;

        case GS_CMD_setJointTwistHighSpring: {
            if (stringData) {
                if (sscanf(stringData, "%f,%f", &jointParameters.highSpring.x, &jointParameters.highDamping.x) != 2)
                    sp->error("PhysicsScripting::process - invalid parameters for setJointTwistHighSpring");
            } else {
                sp->error("PhysicsScripting::process - stringData missing from setJointTwistHighSpring");
            }
        }
        break;

        case GS_CMD_setJointSwing1: {
            if (stringData) {
                if (sscanf(stringData, "%f,%f,%f", &jointParameters.highAngle.y, &jointParameters.highSpring.y,
                           &jointParameters.highDamping.y) != 3)
                    sp->error("PhysicsScripting::process - invalid parameters for setJointSwing1");
            } else {
                sp->error("PhysicsScripting::process - stringData missing from setJointSwing1");
            }
        }
        break;

        case GS_CMD_setJointSwing2: {
            if (stringData) {
                if (sscanf(stringData, "%f,%f,%f", &jointParameters.highAngle.z, &jointParameters.highSpring.z,
                           &jointParameters.highDamping.z) != 3)
                    sp->error("PhysicsScripting::process - invalid parameters for setJointSwing2");
            } else {
                sp->error("PhysicsScripting::process - stringData missing from setJointSwing2");
            }
        }
        break;

        case GS_CMD_setJointBreakForce: {
            float floatValue = *(float *)&intData;
            if (floatValue >= 0.f)
                jointParameters.breakForce = floatValue;
            else
                sp->error("PhysicsScripting::process - invalid force for setJointBreakForce");
        }
        break;

        case GS_CMD_setJointAxis: {
            if (stringData) {
                if (sscanf(stringData, "%f,%f,%f", &jointParameters.axis.x, &jointParameters.axis.y,
                           &jointParameters.axis.z) != 3)
                    sp->error("PhysicsScripting::process - invalid parameters for setJointAxis");
            } else {
                sp->error("PhysicsScripting::process - stringData missing from setJointAxis");
            }
        }
        break;

        case GS_CMD_setJointNormal: {
            if (stringData) {
                if (sscanf(stringData, "%f,%f,%f", &jointParameters.normal.x, &jointParameters.normal.y,
                           &jointParameters.normal.z) != 3)
                    sp->error("PhysicsScripting::process - invalid parameters for setJointNormal");
            } else {
                sp->error("PhysicsScripting::process - stringData missing from setJointNormal");
            }
        }
        break;

        case GS_CMD_setJointPosition: {
            if (stringData) {
                if (sscanf(stringData, "%f,%f,%f", &jointParameters.position.x, &jointParameters.position.y,
                           &jointParameters.position.z) != 3)
                    sp->error("PhysicsScripting::process - invalid parameters for setJointPosition");
            } else {
                sp->error("PhysicsScripting::process - stringData missing from setJointPosition");
            }
        }
        break;

        case GS_CMD_setJointNormalAngle: {
            float floatValue = *(float *)&intData;
            if (floatValue >= -180.f && floatValue <= 180.f)
                jointParameters.normalAngle = floatValue;
            else
                sp->error("PhysicsScripting::process - invalid angle for setJointNormalAngle");
        }
        break;

        /*
           case GS_CMD_setJointId:
            {
                if(stringData)
                    jointParameters.id = stringData;
                else
                    sp->error("PhysicsScripting::process - no string data for setJointId");
            }
           break;
         */

        case GS_CMD_createJoint: {
            if ( (jointParameters.objectA.empty() && jointParameters.objectAUH == UNIFIED_HANDLE_NONE)
                 || (jointParameters.objectB.empty() && jointParameters.objectBUH == UNIFIED_HANDLE_NONE) )
            {
                sp->error("PhysicsScripting::process - invalid parameters for createJoint");
                jointParameters = JointParameters();
                break;
            }
            if ( (!jointParameters.objectA.empty() && jointParameters.objectAUH != UNIFIED_HANDLE_NONE)
                 || (!jointParameters.objectB.empty() && jointParameters.objectBUH != UNIFIED_HANDLE_NONE) )
            {
                // NOTE: this may be caused by leaking properties from one joint creation to another...
                // (if some joint creation is cancelled in the middle of setting properties, use cancelJoint to prevent
                // this from happening)
                sp->error("PhysicsScripting::process - supplied both UH and UEOH for joint object (only one expected).");
                jointParameters = JointParameters();
                break;
            }

            bool nullObjectA = jointParameters.objectA == "null";
            bool nullObjectB = jointParameters.objectB == "null";

            boost::shared_ptr<game::AbstractPhysicsObject> tempA;
            boost::shared_ptr<game::AbstractPhysicsObject> tempB;

            Terrain *terrain = game->gameUI->getTerrain();
            if (terrain) {
                if (!nullObjectA) {
                    __int64 hexA = 0;
                    if ( !jointParameters.objectA.empty() )
                        hexA = hex_to_int64( jointParameters.objectA.c_str() );

                    UnifiedHandle handleA = jointParameters.objectAUH;
                    if (hexA != 0)
                        handleA = terrain->findTerrainObjectByHex(hexA);
                    tempA = terrain->getPhysicsActor(handleA);
                }

                if (!nullObjectB) {
                    __int64 hexB = 0;
                    if ( !jointParameters.objectB.empty() )
                        hexB = hex_to_int64( jointParameters.objectB.c_str() );

                    UnifiedHandle handleB = jointParameters.objectBUH;
                    if (hexB != 0)
                        handleB = terrain->findTerrainObjectByHex(hexB);
                    tempB = terrain->getPhysicsActor(handleB);
                }
            }

            if ( (!tempA && !nullObjectA) || (!tempB && !nullObjectB) ) {
                if (!tempA && !tempB)
                    sp->error("PhysicsScripting::process - invalid objects for createJoint (objects have no physics?)");
                else
                    sp->error("PhysicsScripting::process - invalid object for createJoint (object has no physics?)");
                jointParameters = JointParameters();
                break;
            }

            frozenbyte::physics::PhysicsJoint jointInfo;
            jointInfo.globalAnchor = jointParameters.position;

            jointInfo.globalAxis = jointParameters.axis;
            jointInfo.globalNormal = jointParameters.normal;
            QUAT q;
            q.MakeFromAngles(0, jointParameters.normalAngle * PI / 180.f);
            q.RotateVector(jointInfo.globalAxis);
            q.RotateVector(jointInfo.globalNormal);

            jointInfo.low.angle.x = jointParameters.lowAngle.x * PI / 180.f;
            jointInfo.low.angle.y = jointParameters.lowAngle.y * PI / 180.f;
            jointInfo.low.angle.z = jointParameters.lowAngle.z * PI / 180.f;
            jointInfo.high.angle.x = jointParameters.highAngle.x * PI / 180.f;
            jointInfo.high.angle.y = jointParameters.highAngle.y * PI / 180.f;
            jointInfo.high.angle.z = jointParameters.highAngle.z * PI / 180.f;
            jointInfo.low.spring.x = jointParameters.lowSpring.x;
            jointInfo.low.spring.y = jointParameters.lowSpring.y;
            jointInfo.low.spring.z = jointParameters.lowSpring.z;
            jointInfo.high.spring.x = jointParameters.highSpring.x;
            jointInfo.high.spring.y = jointParameters.highSpring.y;
            jointInfo.high.spring.z = jointParameters.highSpring.z;
            jointInfo.low.damping.x = jointParameters.lowDamping.x;
            jointInfo.low.damping.y = jointParameters.lowDamping.y;
            jointInfo.low.damping.z = jointParameters.lowDamping.z;
            jointInfo.high.damping.x = jointParameters.highDamping.x;
            jointInfo.high.damping.y = jointParameters.highDamping.y;
            jointInfo.high.damping.z = jointParameters.highDamping.z;

            jointInfo.breakForce = jointParameters.breakForce;
            if ( game->getGamePhysics() )
                game->getGamePhysics()->addJoint(tempA, tempB, jointInfo, jointParameters.id);

            jointParameters = JointParameters();
        }
        break;

        case GS_CMD_cancelJoint: {
            jointParameters = JointParameters();
        }
        break;

        case GS_CMD_setJointObjectAToUnifiedHandle:
            if ( VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle) ) {
                // for now, only terrainobjects are ok.
                if ( IS_UNIFIED_HANDLE_TERRAIN_OBJECT(gsd->unifiedHandle) )
                    jointParameters.objectAUH = gsd->unifiedHandle;
                else
                    sp->error(
                        "PhysicsScripting::process - setJointObjectAToUnifiedHandle, only terrain objects supported.");
            } else {
                sp->error("PhysicsScripting::process - setJointObjectAToUnifiedHandle, invalid unified handle.");
            }
            break;

        case GS_CMD_setJointObjectBToUnifiedHandle:
            if ( VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle) ) {
                // for now, only terrainobjects are ok.
                if ( IS_UNIFIED_HANDLE_TERRAIN_OBJECT(gsd->unifiedHandle) )
                    jointParameters.objectBUH = gsd->unifiedHandle;
                else
                    sp->error(
                        "PhysicsScripting::process - setJointObjectBToUnifiedHandle, only terrain objects supported.");
            } else {
                sp->error("PhysicsScripting::process - setJointObjectBToUnifiedHandle, invalid unified handle.");
            }
            break;

        case GS_CMD_setJointPositionToPosition:
            jointParameters.position = gsd->position;
            break;

        case GS_CMD_setJointParam: {
            if (stringData) {
                float x, y, z;
                char str[64];
                if (sscanf(stringData, "%s %f,%f,%f", str, &x, &y, &z) != 4) {
                    sp->error("PhysicsScripting::process - invalid parameters for setJointParam");
                    break;
                }

                if (strcmp(str, "lowAngle") == 0) {
                    jointParameters.lowAngle = VC3(x, y, z);
                } else if (strcmp(str, "highAngle") == 0) {
                    jointParameters.highAngle = VC3(x, y, z);
                } else if (strcmp(str, "lowSpring") == 0) {
                    jointParameters.lowSpring = VC3(x, y, z);
                } else if (strcmp(str, "highSpring") == 0) {
                    jointParameters.highSpring = VC3(x, y, z);
                } else if (strcmp(str, "lowDamping") == 0) {
                    jointParameters.lowDamping = VC3(x, y, z);
                } else if (strcmp(str, "highDamping") == 0) {
                    jointParameters.highDamping = VC3(x, y, z);
                } else {
                    sp->error("PhysicsScripting::process - unknown param name for setJointParam");
                    break;
                }
            } else {
                sp->error("PhysicsScripting::process - stringData missing from setJointParam");
            }
        }
        break;

        case GS_CMD_setJointDeforming: {
            if (stringData) {
                if ( game->getGamePhysics() ) {
                    frozenbyte::physics::JointDeformingInfo info;

                    if (sscanf(stringData, "%f,%f,%f", &info.bendAngle, &info.breakAngle, &info.durability) != 3)
                        sp->error(
                            "PhysicsScripting::process - invalid parameters for setJointDeforming (expecting \"bendAngle,breakAngle,durability\")");

                    info.bendAngle *= PI / 180.0f;
                    info.breakAngle *= PI / 180.0f;
                    game->getGamePhysics()->addDeformingToPreviousJoint(&info);
                }
            } else {
                sp->error("PhysicsScripting::process - stringData missing from setJointDeforming");
            }
        }
        break;

        default:
            sp->error("PhysicsScripting::process - Unknown command.");
            assert(0);
        }
    }
}
