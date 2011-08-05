
#ifndef GAMECONFIGS_H
#define GAMECONFIGS_H

#define GAMECONFIGS_NO_ID -1

// max amount of IDable game config entries 
// (also defines the limit for the max id value)
#define GAMECONFIGS_MAX_IDS 1024

class LinkedList;

namespace game
{
	class GameConfNodeImpl;

	/**
	 * A class that can hold game configuration key and value pairs.
	 * Note, you most probably want to see GameOptionManager instead.
	 * This is an old deprecated implementation! Still used by
	 * GameOptionManager, but may be removed in the future.
	 * 
   * @version 1.0, 13.3.2003
   * @author Jukka Kokkonen <jukka@frozenbyte.com>
	 * @see GameOptionManager
	 */
	class GameConfigs
	{
		public:
			GameConfigs();

			~GameConfigs();

			static GameConfigs *getInstance();

			static void cleanInstance();

			bool getBoolean(const char *confname);
			int getInt(const char *confname);
			float getFloat(const char *confname);
			char *getString(const char *confname);

			bool getBoolean(int id);
			int getInt(int id);
			float getFloat(int id);
			char *getString(int id);

			const char *getNameForId(int id);

			void addBoolean(const char *confname, bool value, 
				int id = GAMECONFIGS_NO_ID);

			void addInt(const char *confname, int value, 
				int id = GAMECONFIGS_NO_ID);

			void addFloat(const char *confname, float value, 
				int id = GAMECONFIGS_NO_ID);

			void addString(const char *confname, const char *value, 
				int id = GAMECONFIGS_NO_ID);

			void setBoolean(const char *confname, bool value);

			void setInt(const char *confname, int value);

			void setFloat(const char *confname, float value);

			void setString(const char *confname, const char *value);

			void setBoolean(int id, bool value);

			void setInt(int id, int value);

			void setFloat(int id, float value);

			void setString(int id, const char *value);

		private:

			GameConfNodeImpl *idTable[GAMECONFIGS_MAX_IDS];

			GameConfNodeImpl *getNode(const char *confname);

			static GameConfigs *instance;

			// a rather unefficient solution, but so what
			// this should be used only on non-time-critical code.
			LinkedList *confList;
	};
}

#endif

