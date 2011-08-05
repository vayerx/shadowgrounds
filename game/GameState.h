
#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "GameObject.h"

namespace game
{
	class GameStateVariable;

	class GameState : public GameObject
	{
		public:
	    virtual SaveData *getSaveData() = 0;	

	};
}

#endif

