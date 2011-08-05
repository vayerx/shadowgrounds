
#ifndef GAMEPROFILESENUMERATION_H
#define GAMEPROFILESENUMERATION_H

#include <vector>
#include <string>

namespace game
{
	class GameProfilesEnumeration
	{
		public:
			std::string getNextProfile();
			bool isNextProfileAvailable();

		private:
			std::vector<std::string> profileList;
			int currentPos;

		friend class GameProfiles;
	};

}



#endif

