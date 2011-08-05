
#include "precompiled.h"

#include "PlayerWeaponry.h"

#include "Game.h"
#include "Unit.h"
#include "PartType.h"
#include "Weapon.h"
#include "Flashlight.h"
#include "../ui/AnimationSet.h"
#include "../ui/Animator.h"
#include "../util/SimpleParser.h"
#include "../system/Logger.h"
#include "../convert/str2int.h"

// (for max number of ui weaps)
#include "../ui/GameController.h"

#include <map>
#include <vector>


#define PLAYERWEAPONRY_MAX_WEAPONS 16


// TODO: should read weapons from some conf file or something...

namespace game
{
#ifdef PROJECT_SHADOWGROUNDS
	int playerweaponry_weaplist[PLAYERWEAPONRY_MAX_WEAPONS] = { 0 };

	int PlayerWeaponry::getPlayerWeaponry( Unit* unit, int i )
	{
		if ( i >= 0 && i < PLAYERWEAPONRY_MAX_WEAPONS )
		{
			return playerweaponry_weaplist[ i ];
		}
		else
		{
			return 0;
		}
	}

	void PlayerWeaponry::setPlayerWeaponry( Unit* unit, int i, int weapon )
	{
		if ( i >= 0 && i < PLAYERWEAPONRY_MAX_WEAPONS )
		{
			playerweaponry_weaplist[ i ] = weapon;
		}
	}
#else
	std::map< int, std::vector< int > > playerweaponry_weaplist;

	int PlayerWeaponry::getPlayerWeaponry( Unit* unit, int i )
	{
		if( unit )
		{
			int unit_id = unit->getIdNumber();

			std::map< int, std::vector< int > >::iterator it;
			it = playerweaponry_weaplist.find( unit_id );
			if( it != playerweaponry_weaplist.end() )
			{
				if( i >= 0 && i < (signed)it->second.size() )
					return it->second[ i ];
			}
		}
		
		return 0;
	}

	void PlayerWeaponry::setPlayerWeaponry( Unit* unit, int i, int weapon )
	{
		if( unit )
		{
			int unit_id = unit->getIdNumber();
			std::map< int, std::vector< int > >::iterator it;
			it = playerweaponry_weaplist.find( unit_id );
			if( it == playerweaponry_weaplist.end() )
				playerweaponry_weaplist.insert( std::pair< int, std::vector< int > >( unit_id, std::vector< int >( PLAYERWEAPONRY_MAX_WEAPONS ) ) );
			
			if( i >= 0 && i < PLAYERWEAPONRY_MAX_WEAPONS )
			{
				playerweaponry_weaplist[ unit_id ][ i ] = weapon;
			}
		}
	}

#endif

	// NOTE: unit parameter not used in these 2 methods,
	// maybe needed in future, so keep it that way?

	void PlayerWeaponry::initWeaponry()
	{
		Logger::getInstance()->debug("PlayerWeaponry::initWeaponry - About to load player weaponry file.");

		util::SimpleParser sp;
#ifdef LEGACY_FILES
		if (sp.loadFile("Data/Parts/playerweaponry.txt"))
#else
		if (sp.loadFile("data/part/playerweaponry.txt"))
#endif
		{
			while (sp.next())
			{
				if (sp.getKey() != NULL && sp.getValue() != NULL)
				{
					int num = str2int(sp.getKey());
					if (num >= 0 && num < PLAYERWEAPONRY_MAX_WEAPONS
						&& str2int_errno() == 0)
					{
						char *partTypeIdString = sp.getValue();
						if (PARTTYPE_ID_STRING_VALID(partTypeIdString))
						{
							int idVal = PARTTYPE_ID_STRING_TO_INT(partTypeIdString);
							setPlayerWeaponry( NULL, num, idVal );
						} else {
							sp.error("PlayerWeaponry::initWeaponry - Value is not a valid part type id.");
						}
					} else {
						sp.error("PlayerWeaponry::initWeaponry - Invalid key string or key out of range.");
					}
				} else {
					sp.error("PlayerWeaponry::initWeaponry - Key or value missing.");
				}
			}
		} else {
			sp.error("PlayerWeaponry::initWeaponry - Failed to load player weaponry file, game will not function properly.");
		}

		Logger::getInstance()->debug("PlayerWeaponry::initWeaponry - Player weaponry file loaded.");
	}

	void PlayerWeaponry::uninitWeaponry()
	{
		// nop
	}

	int PlayerWeaponry::getWeaponIdByUINumber(Unit *unit, int weaponUINumber)
	{
		int weapId = 0;
		weapId = getPlayerWeaponry( unit, weaponUINumber );
		return weapId;
	}


	int PlayerWeaponry::getUINumberByWeaponId(Unit *unit, int weaponId)
	{
		// NOTE: this is not very efficient.
		for (int uinum = 0; uinum < WEAPON_CTRL_AMOUNT; uinum++)
		{
			if (getWeaponIdByUINumber(unit, uinum) == weaponId)
				return uinum;
		}
		return -1;
	}


	void PlayerWeaponry::selectWeapon(Game *game, Unit *unit, int weaponId)
	{
		int weapId = weaponId;

		int newweap = -1;
		if (weapId != 0)
		{
			newweap = unit->getWeaponByWeaponType(weapId);
		}

		if (newweap != -1)
		{
			unit->setSelectedWeapon(newweap);

			unit->setKeepReloading(false);

			if (unit->isActive())
			{
				for (int w = 0; w < UNIT_MAX_WEAPONS; w++)
				{
					if (w == unit->getSelectedWeapon())
					{
						unit->setWeaponActive(w, true);
						if (unit->getWeaponType(w) != NULL)
						{
							float firingSpread = unit->getFiringSpreadFactor();
							if (firingSpread > unit->getWeaponType(w)->getMaxSpread())
								firingSpread = unit->getWeaponType(w)->getMaxSpread();
							if (firingSpread < unit->getWeaponType(w)->getMinSpread())
								firingSpread = unit->getWeaponType(w)->getMinSpread();
							unit->setFiringSpreadFactor(firingSpread);
						}
					} else {
						unit->setWeaponActive(w, false);
					}
				}
			}

			// FIXME: oh yeah babe, make the right weapon visible..
			// by recreating the whole visual object!
			// _really_ not a very brilliant idea, not at all.
			// (maybe we should just change the visibilities of the
			// weapon objects - now that would be something nice)
			if (unit->isActive())
			{
				//char *stats = game->getGameScene()->getStorm3D()->GetPrintableStatusInfo();
				//Logger::getInstance()->error(stats);
				//game->getGameScene()->getStorm3D()->DeletePrintableStatusInfo(stats);

				if (unit->isMuzzleflashVisible())
					unit->setMuzzleflashVisualEffect(NULL, 0);

				game->deleteVisualOfParts(unit, unit->getRootPart(), true);
				game->createVisualForParts(unit, unit->getRootPart(), true);
				if (unit->getFlashlight() != NULL)
				{
					unit->getFlashlight()->resetOrigin(unit->getVisualObject());
				}

				//stats = game->getGameScene()->getStorm3D()->GetPrintableStatusInfo();
				//Logger::getInstance()->error(stats);
				//game->getGameScene()->getStorm3D()->DeletePrintableStatusInfo(stats);

				unit->setPosition(unit->getPosition());
				VC3 rot = unit->getRotation();
				unit->setRotation(rot.x, rot.y, rot.z);
				if (unit->getWalkDelay() < 1)
					unit->setWalkDelay(1);
				if (unit->getAnimationSet() != NULL)
				{
#ifdef PROJECT_CLAW_PROTO
					// nop
#else
					unit->setAnimation(0); // ANIM_NONE
					if (unit->getAnimationSet()->isAnimationInSet(ANIM_STAND))
						unit->getAnimationSet()->animate(unit, ANIM_STAND);

					// hopefully, these will clear weapon animations of incorrect weapon animation type...
					ui::Animator::endBlendAnimation(unit, 1, true);
					ui::Animator::endBlendAnimation(unit, 2, true);
#endif
				}
			}
		}
	}


}


