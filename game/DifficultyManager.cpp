
#include "precompiled.h"

#include "DifficultyManager.h"

#include "Game.h"
#include "scripting/GameScripting.h"

#define DIFFICULTY_UPDATE_INTERVAL 100

namespace game
{

	DifficultyManager::DifficultyManager(Game *game)
	{
		this->game = game;
		this->ticksUntilUpdate = 0;
		this->damageAmountLevel = 0;
	}

	DifficultyManager::~DifficultyManager()
	{
		// nop?
	}

	void DifficultyManager::run()
	{
		if (ticksUntilUpdate > 0)
		{
			ticksUntilUpdate--;
		} else {
			ticksUntilUpdate = DIFFICULTY_UPDATE_INTERVAL;

			damageAmountLevel = game->gameScripting->getGlobalIntVariableValue("damage_amount_level");
		}
	}

	float DifficultyManager::getPlayerDamageRatio()
	{
		float ret = (float)damageAmountLevel / 100.0f;

		// TODO: this is totally over sensible ranges..
		// damage amount level should be much less significant,
		// the difficulty should be gained with other things as well
		// (such as the amount of items and hostiles)

		// damage ratio is between 25% (very easy) - 125% (very hard)
		ret = 0.25f + ret;
		
		return ret;
	}

}

