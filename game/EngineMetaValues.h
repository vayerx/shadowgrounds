
#ifndef ENGINEMETAVALUES_H
#define ENGINEMETAVALUES_H

namespace game
{
  /**
   * A simple replacement for permanent global script variables for holding values that
	 * are kept during the lifetime of the game engine instance, but are not saved to 
	 * savegames as permanent global script variables are.
	 *
   * @version 1.0, 27.7.2007
   * @author Jukka Kokkonen <jukka@frozenbyte.com>
   */

	class EngineMetaValues
	{
	public:
		static bool doesMetaValueExist(const char *key);
		static bool isMetaValueTypeInt(const char *key);
		static int getMetaValueInt(const char *key);
		static void setMetaValueInt(const char *key, int value);
	};
}

#endif
