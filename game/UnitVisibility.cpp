
#include "precompiled.h"

#include "UnitVisibility.h"

#include "../ui/VisualObject.h"
#include "../system/Logger.h"

#include "../util/Debug_MemoryManager.h"

// 6 pass delay for visibility change to hidden

// (actual value in seconds depends on visibility checker's speed)
// if visibility check make check for one unit in 2 passed, then
// it will take 2 * 10 msec for one unit to pass = 2 seconds for 100 units.
// now, if this value is 3, it takes 2 * 3 = 6 seconds for the unit to
// become invisible...

// 1 * 6 = 6 secs

#define SEEN_DELAY 6


namespace game
{

  // NOTE: clone method or =operator needed if memory allocations or 
	// pointers are used, because = operator is used for objects of this
	// class.

	// NOTE: 2 constructors!
	UnitVisibility::UnitVisibility(int player)
	{
		this->owner = player;
    seenByPlayerBits = 0;
    toBeSeenByPlayerBits = 0;
    inRadarByPlayerBits = 0;
		toBeInRadarByPlayerBits = 0;
    seenByFirstPerson = true;
		destroyed = false;
		for (int i = 0; i < ABS_MAX_PLAYERS; i++)
		{
			seenDelay[i] = 0;
		}
	}



	// NOTE: 2 constructors!
	UnitVisibility::UnitVisibility()
	{
		this->owner = NO_UNIT_OWNER;
    seenByPlayerBits = 0;
    toBeSeenByPlayerBits = 0;
    inRadarByPlayerBits = 0;
		toBeInRadarByPlayerBits = 0;
    seenByFirstPerson = true;
		destroyed = false;
		for (int i = 0; i < ABS_MAX_PLAYERS; i++)
		{
			seenDelay[i] = 0;
		}
	}



	UnitVisibility::~UnitVisibility()
	{
		// TODO
		// nop?
	}


	
	void UnitVisibility::setDestroyed(bool destroyed)
	{
		this->destroyed = destroyed;
	}



  bool UnitVisibility::isSeenByPlayer(int player) const
  {
    #ifdef _DEBUG
      if (player < 0 || player >= ABS_MAX_PLAYERS)
      {
        Logger::getInstance()->error("Unit::isSeenByPlayer - Player number out of range.");
        return false;
      }
    #endif
    if ((seenByPlayerBits & (1<<player)) != 0)
      return true;
    else
      return false;
  }



  /*
  void Unit::setSeenByPlayer(int player, bool seen)
  {
    #ifdef _DEBUG
      if (player < 0 || player >= ABS_MAX_PLAYERS)
      {
        Logger::getInstance()->error("Unit::setSeenByPlayer - Player number out of range.");
        return;
      }
    #endif
    if (seen)
    {
      seenByPlayerBits |= (1<<player);

      // TODO: NETGAME, this won't work for netgame!
      if (player == game->singlePlayerNumber && visualObject != NULL)
        visualObject->setVisible(true);

    } else {
      seenByPlayerBits &= ((1<<ABS_MAX_PLAYERS)-1) ^ (1<<player);

      if (player == game->singlePlayerNumber && visualObject != NULL)
        visualObject->setVisible(false);
    }
  }
  */



  /*
  bool Unit::isToBeSeenByPlayer(int player)
  {
    #ifdef _DEBUG
      if (player < 0 || player >= ABS_MAX_PLAYERS)
      {
        Logger::getInstance()->error("Unit::isToBeSeenByPlayer - Player number out of range.");
        return false;
      }
    #endif
    if ((toBeSeenByPlayerBits & (1<<player)) != 0)
      return true;
    else
      return false;
  }
  */



  void UnitVisibility::useToBeSeenByPlayer()
  {
    seenByPlayerBits = toBeSeenByPlayerBits;
    if (!destroyed)
		{
      //toBeSeenByPlayerBits = 0;
			for (int i = 0; i < ABS_MAX_PLAYERS; i++)
			{
				if (seenDelay[i] > 0)
				{
					seenDelay[i]--;
				} else {
		      toBeSeenByPlayerBits &= (((1<<ABS_MAX_PLAYERS)-1) ^ (1<<i));
				}
			}
		}
    if (owner != NO_UNIT_OWNER)
    {
      toBeSeenByPlayerBits |= (1<<owner);
    }
  }



  void UnitVisibility::setToBeSeenByPlayer(int player, bool seen)
  {
    #ifdef _DEBUG
			if (player < 0 || player >= ABS_MAX_PLAYERS)
			{
				Logger::getInstance()->error("Unit::setToBeSeenByPlayer - Player number out of range.");
				return;
			}
		#endif
    if (seen)
    {
      toBeSeenByPlayerBits |= (1<<player);
			seenDelay[player] = SEEN_DELAY;
    } else {
      toBeSeenByPlayerBits &= (((1<<ABS_MAX_PLAYERS)-1) ^ (1<<player));
    }
  }



  bool UnitVisibility::isInRadarByPlayer(int player)
  {
    #ifdef _DEBUG
      if (player < 0 || player >= ABS_MAX_PLAYERS)
      {
        Logger::getInstance()->error("Unit::isInRadarByPlayer - Player number out of range.");
        return false;
      }
    #endif
    if ((inRadarByPlayerBits & (1<<player)) != 0)
      return true;
    else
      return false;
  }



  void UnitVisibility::setToBeInRadarByPlayer(int player, bool inRadar)
  {
    #ifdef _DEBUG
      if (player < 0 || player >= ABS_MAX_PLAYERS)
      {
        Logger::getInstance()->error("Unit::setToBeInRadarByPlayer - Player number out of range.");
        return;
      }
    #endif
    if (inRadar)
    {
      toBeInRadarByPlayerBits |= (1<<player);
    } else {
      toBeInRadarByPlayerBits &= (((1<<ABS_MAX_PLAYERS)-1) ^ (1<<player));
    }
  }



  bool UnitVisibility::isSeenByFirstPerson()
  {
    return seenByFirstPerson;
  }



  void UnitVisibility::setSeenByFirstPerson(bool seen)
  {
    this->seenByFirstPerson = seen;

		// nowadays set directly at unitvisiblitychecker, cos this class
		// has no access to unit's visualobject.
		/*
    // TODO: proper impl for netgame
    if (visualObject != NULL)
    {
      //if ((seenByPlayerBits & (1<<0)) != 0
      if (seenByFirstPerson) 
      {
        visualObject->setVisible(true);
      } else {
        visualObject->setVisible(false);
      }
    }
		*/
  }



  void UnitVisibility::useToBeInRadarByPlayer()
  {
    inRadarByPlayerBits = toBeInRadarByPlayerBits;
    toBeInRadarByPlayerBits = 0;
    if (owner != NO_UNIT_OWNER)
    {
      toBeInRadarByPlayerBits |= (1<<owner);
    }
  }


}

