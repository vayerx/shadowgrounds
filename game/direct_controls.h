
#ifndef DIRECT_CONTROLS_H
#define DIRECT_CONTROLS_H

#define DIRECT_CTRL_INVALID 0

#define DIRECT_CTRL_FORWARD 1
#define DIRECT_CTRL_BACKWARD 2
#define DIRECT_CTRL_LEFT 3
#define DIRECT_CTRL_RIGHT 4
#define DIRECT_CTRL_FIRE 5
#define DIRECT_CTRL_TURN_LEFT 6
#define DIRECT_CTRL_TURN_RIGHT 7
#define DIRECT_CTRL_FIRE_SECONDARY 8
#define DIRECT_CTRL_SPECIAL_MOVE 9
#define DIRECT_CTRL_FIRE_GRENADE 10

#define DIRECT_CTRL_AMOUNT 11

// returns direct control id or direct_ctrl_invalid if no such control 

extern int getDirectControlIdForName(const char *name);

#endif

