
#ifndef DH_ANIMANAGER_H
#define DH_ANIMANAGER_H

namespace game
{
	class AniManagerImpl;
	class Ani;
	class Unit;
	class GameScripting;

	class AniManager
	{
		public:
			AniManager(GameScripting *gameScripting);

			~AniManager();

			static AniManager *getInstance();

			static void createInstance(GameScripting *gameScripting);

			static void cleanInstance();

			// run all anis
			void run();

			// creates a new (yet empty) ani
			Ani *createNewAniInstance(Unit *unit);

			// deletes a previously created ani
			void deleteAni(Ani *ani);

			// returns true if all ani plays have ended
			bool isAllAniComplete();

			// pauses ani executing or resumes it
			void setAniPause(bool aniPaused);

			// get the ani being currently run (for scripting system)
			Ani *getCurrentScriptAni();

			// get an existing ani with given name
			Ani *getAniByName(const char *aniName);

			// needed or not??
			void setCurrentScriptAni(Ani *ani);

			// stops ani for unit (call when destroying a unit while it is being animated)
			void stopAniForUnit(Unit *unit);

			// stop all playing anis
			void stopAllAniPlay();

			// leap ("fastforward") all anis to the end
			void leapAllAniPlayToEnd();

		private:
			AniManagerImpl *impl;
	};
}

#endif

