#ifndef INC_OGUITYPEEFFECTLISTENER_H
#define INC_OGUITYPEEFFECTLISTENER_H

#include <string>

#include "IOguiEffectListener.h"

namespace game
{
	class Game;
}

// will launch the type sounds
class OguiTypeEffectListener : public IOguiEffectListener
{
public:
	OguiTypeEffectListener( game::Game* game, const std::string& effectFile = "" );
	~OguiTypeEffectListener();
	
	void EffectEvent( OguiEffectEvent *eve );
	
private:
	std::string effectFile;
	game::Game* game;
};

#endif
