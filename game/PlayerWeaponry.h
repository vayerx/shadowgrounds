
#ifndef PLAYERWEAPONRY_H
#define PLAYERWEAPONRY_H

namespace game
{
	class Game;
  class Unit;

	class PlayerWeaponry
	{
		public:

			static void initWeaponry();

			static void uninitWeaponry();

			static int getWeaponIdByUINumber(Unit *unit, int weaponUINumber);

			static int getUINumberByWeaponId(Unit *unit, int weaponId);

			static void selectWeapon(Game *game, Unit *unit, int weaponId);

			// TODO: move next/prev weapon logic here (from combatwindow)

			static void setPlayerWeaponry( Unit* unit, int ui_number, int weapon_id );
		private:

			static int getPlayerWeaponry( Unit* unit, int i );
		
	};

}

#endif

