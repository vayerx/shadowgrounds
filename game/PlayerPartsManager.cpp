
#include "precompiled.h"

#include "PlayerPartsManager.h"

#include <assert.h>
#include "Game.h"
#include "Part.h"
#include "PartList.h"
#include "PartType.h"
#include "PartTypeAvailabilityList.h"
#include "../system/Logger.h"

#include "../util/Debug_MemoryManager.h"

namespace game
{

	PlayerPartsManager::PlayerPartsManager(Game *game)
	{
		this->game = game;
	}


	PlayerPartsManager::~PlayerPartsManager()
	{
		// nop
	}


	bool PlayerPartsManager::allowPartType(int player, char *partIdString)
	{
		assert(partIdString != NULL);
		if (player < 0 || player >= ABS_MAX_PLAYERS)
		{
			assert(!"allowPartType - player number out of range.");
			return false;
		}
    if (!PARTTYPE_ID_STRING_VALID(partIdString))
    {
      if (partIdString == NULL)
        Logger::getInstance()->error("PlayerPartsManager::allowPartType - Expected part type id.");
      else
        Logger::getInstance()->error("PlayerPartsManager::allowPartType - Illegal part type id.");
			return false;
    } else {
      PartType *pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT(partIdString));
      if (pt == NULL) 
      { 
        Logger::getInstance()->error("PlayerPartsManager::allowPartType - Reference to unloaded part type.");
				return false;
      } else {
        if (game->partTypesAvailable->isPartTypeAvailable(player, pt))
        {
					// already have that part type available
					// possibly not an error.(?) 
					// but better give some debug info...
          Logger::getInstance()->debug("PlayerPartsManager::allowPartType - Part type is already available to player.");
        } else {
          game->partTypesAvailable->addPartType(player, pt);
        }
      }
    }
		return true;
	}
	

	Part *PlayerPartsManager::addStoragePart(int player, char *partIdString)
	{
		Part *part = NULL;
    if (!PARTTYPE_ID_STRING_VALID(partIdString))
    {
      if (partIdString == NULL)
        Logger::getInstance()->error("PlayerPartsManager::addStoragePart - Expected part type id.");
      else
        Logger::getInstance()->error("PlayerPartsManager::addStoragePart - Illegal part type id.");
    } else {
      PartType *pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT(partIdString));
      if (pt == NULL) 
      { 
        Logger::getInstance()->error("PlayerPartsManager::addStoragePart - Reference to unloaded part type.");
      } else {
        part = pt->getNewPartInstance();
        part->setOwner(player);
        game->parts->addPart(part);
        //partInUnit = false;
      }
    }
		return part;
	}
						
}
