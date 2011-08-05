
#ifndef GAMEPROFILES_H
#define GAMEPROFILES_H

#include <string>

namespace game
{
	class GameProfilesImpl;
	class GameProfilesEnumeration;

	class GameProfiles
	{
		public:
			GameProfiles();
			~GameProfiles();

			void createNewProfile(const char *profile);
			void deleteProfile(const char *profile);
			void setCurrentProfile(const char *profile, int player, bool safetyChecks = true);
			const char *getCurrentProfile(int player ) const;

			bool isProfileNameValid(const char* profile) const;
			bool doesProfileExist( int player ) const;

			const char *getProfileDirectory(int player ) const;
			std::string convertFilenameToProfileDirectory(const std::string &filename);

			// returns a _new_ enumeration (delete once done with it)
			GameProfilesEnumeration *getProfileList();

		private:
			GameProfilesImpl *impl;
	};
}

#endif


