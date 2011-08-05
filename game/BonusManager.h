
#ifndef BONUSMANAGER_H
#define BONUSMANAGER_H

namespace game
{
	class Game;

	class BonusManager
	{
	public:
		BonusManager(Game *game);
		~BonusManager();

		// list unlocked options
		int getNumOptions();
		const std::string &getOptionName(int i);

		// simply mark options active/inactive (applied later)
		void activateOption(int i);
		bool isActive(int i);
		void deactivateAll();

		// actually applies the options (note: called twice with savegames)
		void applyOptions(bool apply, int flags = 0);

		// add new unlocked option
		void unlockOption(const std::string &name);
		void unlockAllOptions();
		bool isOptionUnlocked(const std::string &name);

		// options are only available after game has been completed
		bool areOptionsAvailable(void);
		void makeOptionsAvailable(bool available);

		enum ApplyFlags
		{
			APPLYING_BEFORE_LOAD = 1,
			APPLYING_AFTER_LOAD = 2
		};

		// hack for options with only button
		bool shouldCreateAsButton(int i);
		void buttonPressed(int i);

	private:	
		class BonusManagerImpl;
		BonusManagerImpl *impl;
	};
}

#endif
