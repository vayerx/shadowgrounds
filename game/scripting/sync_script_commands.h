
// Copyright(C) Jukka Kokkonen, 2007

// from 2100

#define GS_CMD_BASE 2100

GS_CMD_SIMPLE(0, editSendMoveUnifiedHandleObjectToPosition, NONE)
GS_CMD_SIMPLE(1, editSendDeleteUnifiedHandleObject, NONE)
GS_CMD_SIMPLE(2, editSendDuplicateUnifiedHandleObject, NONE)
GS_CMD_SIMPLE(3, editSendRotateUnifiedHandleObjectX, NONE)
GS_CMD_SIMPLE(4, editSendRotateUnifiedHandleObjectY, NONE)
GS_CMD_SIMPLE(5, editSendRotateUnifiedHandleObjectZ, NONE)

GS_CMD_SIMPLE(6, editRecvMoveUnifiedHandleObjectToPosition, NONE)
GS_CMD_SIMPLE(7, editRecvDeleteUnifiedHandleObject, NONE)
GS_CMD_SIMPLE(8, editRecvDuplicateUnifiedHandleObject, NONE)
GS_CMD_SIMPLE(9, editRecvRotateUnifiedHandleObjectX, NONE)
GS_CMD_SIMPLE(10, editRecvRotateUnifiedHandleObjectY, NONE)
GS_CMD_SIMPLE(11, editRecvRotateUnifiedHandleObjectZ, NONE)

#undef GS_CMD_BASE

// up to 2199
