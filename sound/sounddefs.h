
#ifndef SOUNDDEFS_H
#define SOUNDDEFS_H


//#define DEFAULT_SOUND_VOLUME 70

// default volume for effect sounds
#define DEFAULT_SOUND_EFFECT_VOLUME 100

// default volume for speech sounds
#define DEFAULT_SPEECH_VOLUME 100

// default sound volume for "volumed sounds" (sounds that use the
// current script sound volume)
#define DEFAULT_SOUND_SCRIPT_VOLUME 100

#ifdef PROJECT_CLAW_PROTO
#define DEFAULT_SOUND_RANGE 25.f
#else
#define DEFAULT_SOUND_RANGE 55.f
#endif

#define DEFAULT_SOUND_PRIORITY_HIGH 20
#define DEFAULT_SOUND_PRIORITY_NORMAL 10
#ifdef PROJECT_SHADOWGROUNDS
#define DEFAULT_SOUND_PRIORITY_LOW 1
#else
#define DEFAULT_SOUND_PRIORITY_LOW 5
#endif

#define MIN_SOUND_PRIORITY 1
#define MAX_SOUND_PRIORITY 25

#endif

