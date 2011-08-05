
#ifndef UNITVISIBILITY_H
#define UNITVISIBILITY_H

#include "gamedefs.h"

namespace game
{
	class UnitVisibility
	{
		public:
			UnitVisibility();

			UnitVisibility(int player);

			~UnitVisibility();

			void setDestroyed(bool destroyed);

			bool isSeenByPlayer(int player) const;
			//void setSeenByPlayer(int player, bool seen);
			//bool isToBeSeenByPlayer(int player);
			void setToBeSeenByPlayer(int player, bool seen);
			void useToBeSeenByPlayer();

			bool isInRadarByPlayer(int player);
			//void setInRadarByPlayer(int player, bool inRadar);
			//bool isToBeInRadarByPlayer(int player);
			void setToBeInRadarByPlayer(int player, bool inRadar);
			void useToBeInRadarByPlayer();

			bool isSeenByFirstPerson();
			void setSeenByFirstPerson(bool seen);

		private:
			int owner;

			int seenByPlayerBits;
			int toBeSeenByPlayerBits;

			int inRadarByPlayerBits;
			int toBeInRadarByPlayerBits;

			// note: improper implementation for netgame...
			bool seenByFirstPerson;

			bool destroyed;

			// to get a small delay for the disappearing of the unit
			int seenDelay[ABS_MAX_PLAYERS];
	};
}

#endif

