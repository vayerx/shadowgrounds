
#ifndef DH_ANI_H
#define DH_ANI_H

#include <DatatypeDef.h>

// possible states for the unit animator:
//  - doing nothing
//  - recording
//  - playing
//
// all other methods than record/play require either record or play 
// state!
// like:
// startPlay()
// skipToMark("x")
// stopPlay()


// NOTE: this value is used to mark ani's "leapToEnd" state.
#define ANI_MAX_TICKS 999999999


namespace game
{
	class GameScripting;
  class Unit;
	class Game;
	class AniImpl;
	class Weapon;

	/**
	 * Unit animator class.
	 */
  class Ani
	{
		public:
			Ani(GameScripting *gameScripting, Unit *unit);

			~Ani();

			// set the pathname where the records will be saved
			static void setRecordPath(const char *recordPath);

			// set the pathname where the records will be saved
			static const char *getRecordPath();

			// set this ani's name (used by record and play)
			void setName(const char *aniScriptName);

			// set this ani's name (used by record and play)
			const char *getName() const;

			Unit *getUnit();

			// start recording ani to memory
			void startRecord(bool appendToOld = false, int appendAfterTick = -1);

			// saves the recorded ani sequence to disk (with the set name)
			void stopRecord();

			// cancels the recording of the ani sequence (no save)
			void cancelRecord();

			// adds the recorded movement, etc. commands automagically.
			// or, plays the aniscript
			void run();

			// play the recorded ani sequence (with the set name)
			// (aniscript should have previously been load from disk to memory)
			void startPlay();

			// stop the playing of the ani sequence
			// leaving to the current position
			void stopPlay();

			// true/false if the play has ended
			bool hasPlayEnded() const;

			// skip beginning of the ani until given ticks reached
			// (current unit state will not altered)
			void skipToPosition(int ticksFromStart);

			// immediate play beginning of the ani until given ticks reached
			// (unit state will be set to properly to that moment)
			void leapToPosition(int ticksFromStart);

			// leap to end (same as leapToPosition(999999...))
			void leapToEnd();

			// skip beginning of the ani until given mark reached
			// (current unit state will not altered)
			void skipToMark(const char *mark);

			// immediate play beginning of the ani until given mark reached
			// (unit state will be set to properly to that moment)
			void leapToMark(const char *mark);

			// add a named marker
			void addMark(const char *markName);

			// for scripting system's use (others may find these a bit
			// useless ;)
			// return true if the system should pause
			bool reachedMark(const char *markName);
			bool reachedTick();

			void aniStart();
			void aniEnd();
			void aniWarp(float x, float z);
			void aniHeight(float height);
			void aniRots(float angleX, float angleY, float angleZ);
			void aniAnim(int anim);
			void aniEndAnim(int anim);
			void aniMoveX(float offsetX);
			void aniMoveY(float offsetY);
			void aniMoveZ(float offsetZ);
			void aniOnGround(float height);
			void aniRotX(float rotateX);
			void aniRotY(float rotateY);
			void aniRotZ(float rotateZ);
			void aniAim(float aimY);
			void aniEndAim();
			void aniAxis(float yAxisRotation);
			void aniIgnore(const char *ignoreOperation);
			void aniMuzzleflash();

			void aniBlend(int anim);
			void aniEndBlend();

			void aniFireProjectile(Game *game, Weapon *weap, const VC3 &projpos, const VC3 &projtarg);

			bool aniWaitUntilAnisEnded();

			static void setGlobalOffsetSource(const VC3 &sourcePosition);
			static void setGlobalOffsetTarget(const VC3 &targetPosition);
			static void setGlobalRotation(float rotationY);
			static void resetGlobalOffsetAndRotation();

			// Duh, game camera also needs to use these values...
			static VC3 getGlobalOffsetSource();
			static VC3 getGlobalOffsetTarget();
			static float getGlobalRotation();

		private:
			AniImpl *impl;
	};
}

#endif

