
#ifndef ANIRECORDER_H
#define ANIRECORDER_H

#include <string>

class LinkedList;

namespace game
{
	class Ani;
	class Unit;
	class Game;
	class AniRecorderImpl;

	class AniRecorder
	{
		public:
			AniRecorder(Game *game);

			~AniRecorder();

			void reload();

			// advance one tick, remember to call after every animanager run
			void run();

			void setRecords(const char *recordDir);

			// plays all anis
			void play();

			// records to one ani, plays the others
			void record(Unit *unit);

			void addAniRecord(Unit *unit);

			void removeAniRecord(Unit *unit);

			void seekToTime(int ticks);
			void seekEndToTime(int ticks);

			void seekToMark(const char *mark);

			// note: takes some time, may improperly set current ani position!
			// (should rewind after calling this)
			void calculateLongestAniLength();
			int getLongestAniLength();

			int getCurrentPosition();
			int getCurrentEndPosition();

			void stop();

			void rewind();

			bool isRecording();

			bool isPlaying();

			// adds undo history
			void addScriptCommand(Unit *unit, const char *scriptCommand, const char *description);
			// does NOT add undo history (call only after addScriptCommand for consecutive script commands)
			void addScriptCommandContinued(Unit *unit, const char *scriptCommand);

			const char *getCurrentAniId();

			int dumpCameraPosition();
			void deleteCameraDump(int cameraDumpNumber);
			void testCameraPosition(int cameraDumpNumber);
			void interpolateCameraPosition(int cameraDumpNumber);

			// returns a new list, delete once done with it!
			LinkedList *getCameraDumpList();

			// returns a new list, delete once done with it!
			LinkedList *getUnitList();

			void addUndoHistory(const char *description);
			void undo();
			void redo();
			bool canUndo();
			bool canRedo();
			const char *getUndoDesc();
			const char *getRedoDesc();

			void deletePosition(Unit *unit);
			void dropOnGround(Unit *unit);
			void smoothPosition(Unit *unit, int smoothAmount);
			void smoothRotation(Unit *unit, int smoothAmount);
			void smoothAim(Unit *unit, int smoothAmount);

			std::string getSliderPosOrRangeText();

		private:
			AniRecorderImpl *impl;

			void applyUndoHistory();
	};

}


#endif

