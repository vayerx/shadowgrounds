// from 2350

#define GS_CMD_BASE 2350

GS_CMD_SIMPLE(0, doesReplacementForTerrainObjectExist,                NONE)
GS_CMD_SIMPLE(1, getReplacementForTerrainObject,                      NONE)

GS_CMD_SIMPLE(2, findClosestTerrainObjectOfMaterial,                  STRING)
GS_CMD_SIMPLE(3, getTerrainObjectPosition,                            NONE)

GS_CMD_SIMPLE(4, hasTerrainObjectMetaValueString,                     STRING)
GS_CMD_SIMPLE(5, getTerrainObjectMetaValueString,                     STRING)

GS_CMD_SIMPLE(6, getTerrainObjectVariable,                            STRING)
GS_CMD_SIMPLE(7, setTerrainObjectVariable,                            STRING)

GS_CMD_SIMPLE(8, changeTerrainObjectTo,                               STRING)
GS_CMD_SIMPLE(9, deleteTerrainObject,                                 NONE)

GS_CMD_SIMPLE(10, setTerrainObjectDamageTextureFadeFactorToFloatValue, NONE)
GS_CMD_SIMPLE(11, setTerrainObjectByIdString,                          STRING)
GS_CMD_SIMPLE(12, getTerrainObjectIdString,                            NONE)

GS_CMD_SIMPLE(13, findClosestTerrainObjectWithFilenamePart,            STRING)

#undef GS_CMD_BASE

// up to 2399
