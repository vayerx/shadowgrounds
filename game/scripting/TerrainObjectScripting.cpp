#include "precompiled.h"

#include "TerrainObjectScripting.h"

#include "scripting_macros_start.h"
#include "terrainobject_script_commands.h"
#include "scripting_macros_end.h"

#include <DatatypeDef.h>
#include <IStorm3D.h>
#include <istorm3d_mesh.h>

#include "GameScriptData.h"
#include "GameScriptingUtils.h"
#include "../Game.h"
#include "../GameUI.h"
#include "../GameMap.h"
#include "../GameScene.h"
#include "../scaledefs.h"
#include "../savegamevars.h"
#include "../../util/StringUtil.h"

#include "../../system/Logger.h"
#include "../../system/Timer.h"
#include <stdio.h>

#include "../../convert/str2int.h"
#include "../../convert/int64_to_hex.h"
#include "../../util/ScriptProcess.h"
#include "../../util/ScriptManager.h"
#include "../unified_handle.h"
#include "../UnifiedHandleManager.h"
#include "../../editor/UniqueEditorObjectHandle.h"
#include "../../editor/ueoh_to_id_string.h"

#include "../../util/Debug_MemoryManager.h"

using namespace ui;

namespace game
{
    void TerrainObjectScripting::process(util::ScriptProcess *sp,
                                         int command, int intData, char *stringData, ScriptLastValueType *lastValue,
                                         GameScriptData *gsd, Game *game)
    {
        switch (command) {
        case GS_CMD_getTerrainObjectMetaValueString:
            if (stringData != NULL) {
                if ( VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle) ) {
                    if ( IS_UNIFIED_HANDLE_TERRAIN_OBJECT(gsd->unifiedHandle) ) {
                        if ( game->unifiedHandleManager->doesObjectExist(gsd->unifiedHandle) ) {
                            if ( game->gameUI->getTerrain() != NULL
                                 && game->gameUI->getTerrain()->hasObjectTypeMetaValue(gsd->unifiedHandle, stringData) )
                            {
                                gsd->setStringValue( game->gameUI->getTerrain()->getObjectTypeMetaValue(
                                                         gsd->
                                                         unifiedHandle,
                                                         stringData).
                                                     c_str() );
                            } else {
                                sp->warning(
                                    "MiscScripting::process - getTerrainObjectMetaValueString, object type does not have requested meta key.");
                                Logger::getInstance()->debug(stringData);
                                gsd->setStringValue(NULL);
                            }
                        } else {
                            sp->error(
                                "MiscScripting::process - getTerrainObjectMetaValueString, object does not exist with given unified handle.");
                            gsd->setStringValue(NULL);
                        }
                    } else {
                        sp->error(
                            "MiscScripting::process - getTerrainObjectMetaValueString, object with given unified handle is not a terrain object.");
                        gsd->setStringValue(NULL);
                    }
                } else {
                    sp->error("MiscScripting::process - getTerrainObjectMetaValueString, invalid unified handle.");
                    gsd->setStringValue(NULL);
                }
            } else {
                sp->error(
                    "MiscScripting::process - getTerrainObjectMetaValueString parameter missing (terrain object meta key name expected).");
                gsd->setStringValue(NULL);
            }
            break;

        case GS_CMD_hasTerrainObjectMetaValueString:
            if (stringData != NULL) {
                if ( VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle) ) {
                    if ( IS_UNIFIED_HANDLE_TERRAIN_OBJECT(gsd->unifiedHandle) ) {
                        if ( game->unifiedHandleManager->doesObjectExist(gsd->unifiedHandle) ) {
                            if ( game->gameUI->getTerrain() != NULL
                                 && game->gameUI->getTerrain()->hasObjectTypeMetaValue(gsd->unifiedHandle, stringData) )
                                *lastValue = 1;
                            else
                                *lastValue = 0;
                        } else {
                            sp->error(
                                "MiscScripting::process - hasTerrainObjectMetaValueString, object does not exist with given unified handle.");
                            *lastValue = 0;
                        }
                    } else {
                        sp->error(
                            "MiscScripting::process - hasTerrainObjectMetaValueString, object with given unified handle is not a terrain object.");
                        *lastValue = 0;
                    }
                } else {
                    sp->error("MiscScripting::process - hasTerrainObjectMetaValueString, invalid unified handle.");
                    *lastValue = 0;
                }
            } else {
                sp->error(
                    "MiscScripting::process - hasTerrainObjectMetaValueString parameter missing (terrain object meta key name expected).");
                *lastValue = 0;
            }
            break;

        case GS_CMD_getTerrainObjectVariable:
            if (stringData != NULL) {
                if ( VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle) ) {
                    if ( IS_UNIFIED_HANDLE_TERRAIN_OBJECT(gsd->unifiedHandle) ) {
                        if ( game->gameUI->getTerrain() != NULL
                             && game->unifiedHandleManager->doesObjectExist(gsd->unifiedHandle) )
                        {
                            int varNum = game->gameUI->getTerrain()->getObjectVariableNumberByName(stringData);
                            if (varNum != -1) {
                                *lastValue = game->gameUI->getTerrain()->getObjectVariableValue(gsd->unifiedHandle,
                                                                                                varNum);
                            } else {
                                sp->warning(
                                    "MiscScripting::process - getTerrainObjectVariable, terrain object variable of given name does not exist.");
                                Logger::getInstance()->debug(stringData);
                                *lastValue = 0;
                            }
                        } else {
                            sp->error(
                                "MiscScripting::process - getTerrainObjectVariable, object does not exist with given unified handle.");
                            *lastValue = 0;
                        }
                    } else {
                        sp->error(
                            "MiscScripting::process - getTerrainObjectVariable, object with given unified handle is not a terrain object.");
                        *lastValue = 0;
                    }
                } else {
                    sp->error("MiscScripting::process - getTerrainObjectVariable, invalid unified handle.");
                    *lastValue = 0;
                }
            } else {
                sp->error(
                    "MiscScripting::process - getTerrainObjectVariable parameter missing (terrain object variable name expected).");
                *lastValue = 0;
            }
            break;

        case GS_CMD_setTerrainObjectVariable:
            if (stringData != NULL) {
                if ( VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle) ) {
                    if ( IS_UNIFIED_HANDLE_TERRAIN_OBJECT(gsd->unifiedHandle) ) {
                        if ( game->gameUI->getTerrain() != NULL
                             && game->unifiedHandleManager->doesObjectExist(gsd->unifiedHandle) )
                        {
                            int varNum = game->gameUI->getTerrain()->getObjectVariableNumberByName(stringData);
                            if (varNum != -1) {
                                game->gameUI->getTerrain()->setObjectVariableValue(gsd->unifiedHandle,
                                                                                   varNum,
                                                                                   *lastValue);
                            } else {
                                sp->warning(
                                    "MiscScripting::process - setTerrainObjectVariable, terrain object variable of given name does not exist.");
                                Logger::getInstance()->debug(stringData);
                            }
                        } else {
                            sp->error(
                                "MiscScripting::process - setTerrainObjectVariable, object does not exist with given unified handle.");
                        }
                    } else {
                        sp->error(
                            "MiscScripting::process - setTerrainObjectVariable, object with given unified handle is not a terrain object.");
                    }
                } else {
                    sp->error("MiscScripting::process - setTerrainObjectVariable, invalid unified handle.");
                }
            } else {
                sp->error(
                    "MiscScripting::process - setTerrainObjectVariable parameter missing (terrain object variable name expected).");
            }
            break;

        case GS_CMD_changeTerrainObjectTo:
            if (stringData != NULL) {
                if ( VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle) ) {
                    if ( IS_UNIFIED_HANDLE_TERRAIN_OBJECT(gsd->unifiedHandle) ) {
                        if ( game->gameUI->getTerrain() != NULL
                             && game->unifiedHandleManager->doesObjectExist(gsd->unifiedHandle) )
                        {
                            int modelId = game->gameUI->getTerrain()->getModelIdForFilename(stringData);
                            if (modelId != -1) {
                                gsd->unifiedHandle = game->gameUI->getTerrain()->changeObjectTo(gsd->unifiedHandle,
                                                                                                modelId);
                                if (gsd->unifiedHandle == UNIFIED_HANDLE_NONE)
                                    sp->error(
                                        "MiscScripting::process - changeTerrainObjectTo, Terrain::changeObjectTo failed (internal error?).");
                            } else {
                                sp->error(
                                    "MiscScripting::process - changeTerrainObjectTo, no terrain object model loaded with given filename.");
                            }
                        } else {
                            sp->error(
                                "MiscScripting::process - changeTerrainObjectTo, object does not exist with given unified handle.");
                        }
                    } else {
                        sp->error(
                            "MiscScripting::process - changeTerrainObjectTo, object with given unified handle is not a terrain object.");
                    }
                } else {
                    sp->error("MiscScripting::process - changeTerrainObjectTo, invalid unified handle.");
                }
            } else {
                sp->error(
                    "MiscScripting::process - changeTerrainObjectTo, parameter missing (terrain object variable name expected).");
            }
            break;

        case GS_CMD_deleteTerrainObject:
            if ( VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle) ) {
                if ( IS_UNIFIED_HANDLE_TERRAIN_OBJECT(gsd->unifiedHandle) ) {
                    if ( game->gameUI->getTerrain() != NULL
                         && game->unifiedHandleManager->doesObjectExist(gsd->unifiedHandle) )
                        game->gameUI->getTerrain()->deleteTerrainObject(gsd->unifiedHandle);
                    else
                        sp->error(
                            "MiscScripting::process - deleteTerrainObject, object does not exist with given unified handle.");
                } else {
                    sp->error(
                        "MiscScripting::process - deleteTerrainObject, object with given unified handle is not a terrain object.");
                }
            } else {
                sp->error("MiscScripting::process - deleteTerrainObject, invalid unified handle.");
            }
            break;

        case GS_CMD_setTerrainObjectDamageTextureFadeFactorToFloatValue:
            if ( VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle) ) {
                if ( IS_UNIFIED_HANDLE_TERRAIN_OBJECT(gsd->unifiedHandle) ) {
                    if ( game->gameUI->getTerrain() != NULL
                         && game->unifiedHandleManager->doesObjectExist(gsd->unifiedHandle) )
                        game->gameUI->getTerrain()->setInstanceDamageTexture(gsd->unifiedHandle, gsd->floatValue);
                    else
                        sp->error(
                            "MiscScripting::process - setTerrainObjectDamageTextureFadeFactor, object does not exist with given unified handle.");
                } else {
                    sp->error(
                        "MiscScripting::process - setTerrainObjectDamageTextureFadeFactor, object with given unified handle is not a terrain object.");
                }
            } else {
                sp->error("MiscScripting::process - setTerrainObjectDamageTextureFadeFactor, invalid unified handle.");
            }
            break;

        case GS_CMD_doesReplacementForTerrainObjectExist:
            if ( VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle) ) {
                if ( IS_UNIFIED_HANDLE_TERRAIN_OBJECT(gsd->unifiedHandle) ) {
                    if (game->gameUI->getTerrain() != NULL) {
                        if ( !game->gameUI->getTerrain()->doesTrackableUnifiedHandleObjectExist(gsd->unifiedHandle) ) {
                            UnifiedHandle uh = game->gameUI->getTerrain()->getReplacementForUnifiedHandleObject(
                                gsd->unifiedHandle);
                            if (uh != UNIFIED_HANDLE_NONE)
                                *lastValue = 1;
                            else
                                *lastValue = 0;
                        } else {
                            // sp->warning("... terrain object was not even destroyed? ...");
                            *lastValue = 0;
                        }
                    }
                } else {
                    sp->error(
                        "MiscScripting::process - getReplacementForTerrainObject, object with given unified handle is not a terrain object.");
                    *lastValue = 0;
                }
            } else {
                sp->error("MiscScripting::process - getReplacementForTerrainObject, invalid unified handle.");
                *lastValue = 0;
            }
            break;

        case GS_CMD_getReplacementForTerrainObject:
            if ( VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle) ) {
                if ( IS_UNIFIED_HANDLE_TERRAIN_OBJECT(gsd->unifiedHandle) ) {
                    if (game->gameUI->getTerrain() != NULL) {
                        if ( !game->gameUI->getTerrain()->doesTrackableUnifiedHandleObjectExist(gsd->unifiedHandle) )
                            gsd->unifiedHandle = game->gameUI->getTerrain()->getReplacementForUnifiedHandleObject(
                                gsd->unifiedHandle);
                        else
                            // sp->warning("... terrain object was not even destroyed? ...");
                            gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
                    }
                } else {
                    sp->error(
                        "MiscScripting::process - getReplacementForTerrainObject, object with given unified handle is not a terrain object.");
                    gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
                }
            } else {
                sp->error("MiscScripting::process - getReplacementForTerrainObject, invalid unified handle.");
                gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
            }
            break;

        case GS_CMD_findClosestTerrainObjectOfMaterial:
            if (stringData != NULL) {
                if (game->gameUI->getTerrain() != NULL) {
                    // NOTE: hard coded max radius value here.
                    float maxRadius = 100.0f;
                    gsd->unifiedHandle = game->gameUI->getTerrain()->findClosestTerrainObjectOfMaterial(gsd->position,
                                                                                                        stringData,
                                                                                                        maxRadius);
                    if ( VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle) )
                        *lastValue = 1;
                    else
                        *lastValue = 0;
                } else {
                    *lastValue = 0;
                }
            } else {
                sp->error("MiscScripting::process - findClosestTerrainObjectOfMaterial parameter missing.");
            }
            break;

        case GS_CMD_setTerrainObjectByIdString:
            if (stringData != NULL) {
                if (game->gameUI->getTerrain() != NULL) {
                    gsd->unifiedHandle = game->gameUI->getTerrain()->findTerrainObjectByIdString(stringData);
                    if ( VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle) )
                        *lastValue = 1;
                    else
                        *lastValue = 0;
                } else {
                    *lastValue = 0;
                }
            } else {
                sp->error("MiscScripting::process - setTerrainObjectByIdString parameter missing.");
            }
            break;

        case GS_CMD_getTerrainObjectPosition:
            // NOTE: this is the same as getUnifiedHandleObjectPosition, but for terrain object only.
            if ( VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle) ) {
                if ( IS_UNIFIED_HANDLE_TERRAIN_OBJECT(gsd->unifiedHandle) ) {
                    if (game->gameUI->getTerrain() != NULL) {
                        if ( game->gameUI->getTerrain()->doesTrackableUnifiedHandleObjectExist(gsd->unifiedHandle) )
                            gsd->position = game->gameUI->getTerrain()->getTrackableUnifiedHandlePosition(
                                gsd->unifiedHandle);
                        else
                            sp->error(
                                "MiscScripting::process - getTerrainObjectPosition, terrain object with given unified handle does not exist.");
                    }
                } else {
                    sp->error(
                        "MiscScripting::process - getTerrainObjectPosition, object with given unified handle is not a terrain object.");
                }
            } else {
                sp->error("MiscScripting::process - getTerrainObjectPosition, invalid unified handle.");
            }
            break;

        case GS_CMD_getTerrainObjectIdString:
            if ( VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle) ) {
                if ( IS_UNIFIED_HANDLE_TERRAIN_OBJECT(gsd->unifiedHandle) ) {
                    if (game->gameUI->getTerrain() != NULL) {
                        if ( game->gameUI->getTerrain()->doesTrackableUnifiedHandleObjectExist(gsd->unifiedHandle) )
                            gsd->setStringValue( game->gameUI->getTerrain()->getTerrainObjectIdString(gsd->
                                                                                                      unifiedHandle) );
                        else
                            sp->error(
                                "MiscScripting::process - getTerrainObjectIdString, terrain object with given unified handle does not exist.");
                    }
                } else {
                    sp->error(
                        "MiscScripting::process - getTerrainObjectIdString, object with given unified handle is not a terrain object.");
                }
            } else {
                sp->error("MiscScripting::process - getTerrainObjectIdString, invalid unified handle.");
            }
            break;

        case GS_CMD_findClosestTerrainObjectWithFilenamePart:
            if (stringData != NULL) {
                if (game->gameUI->getTerrain() != NULL) {
                    // NOTE: hard coded max radius value here.
                    float maxRadius = 100.0f;
                    gsd->unifiedHandle = game->gameUI->getTerrain()->findClosestTerrainObjectWithFilenamePart(
                        gsd->position,
                        stringData,
                        maxRadius);
                    if ( VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle) )
                        *lastValue = 1;
                    else
                        *lastValue = 0;
                } else {
                    *lastValue = 0;
                }
            } else {
                sp->error("MiscScripting::process - findClosestTerrainObjectWithFilenamePart parameter missing.");
            }
            break;

        default:
            sp->error("MiscScripting::process - Unknown command.");
            assert(0);
        }
    }
}
