
#ifndef GAMEDEFS_H
#define GAMEDEFS_H


// absolute value for maximum players
#define ABS_MAX_PLAYERS 4

// absolute value for maximum players per client 
// reserved for splitsceen! ;)
#define MAX_PLAYERS_PER_CLIENT 4

// default sync send interval in game ticks (artificial lag)
// this smooths "jerkiness" that may be caused by network
#define DEFAULT_MULTIPLAYER_SYNC_INTERVAL 8

// single player game sync (and the future split screen sync ;)
#define DEFAULT_SINGLEPLAYER_SYNC_INTERVAL 1

// Game timer tick rate (Hz)
#ifdef PROJECT_AOV
#define GAME_TICKS_PER_SECOND 100
#else
#define GAME_TICKS_PER_SECOND 67
#endif

// Game timer tick length (milliseconds)
#ifdef PROJECT_AOV
#define GAME_TICK_MSEC 10
#else
#define GAME_TICK_MSEC 15
#endif

// the "invalid player" / "no unit owner"
#define NO_UNIT_OWNER -15233


#endif

