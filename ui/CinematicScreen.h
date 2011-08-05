#ifndef INC_CINEMATICSCREEN_H
#define INC_CINEMATICSCREEN_H

class Ogui;
class IStorm3D;
#include <string>

namespace sfx
{
	class SoundMixer;
}

namespace game
{
	class Game;
}

namespace ui {

///////////////////////////////////////////////////////////////////////////////

class CinematicScreen
{
public:
	class CinematicScreenImpl;

	CinematicScreen( Ogui* ogui, game::Game *game, const std::string& name, sfx::SoundMixer *mixer, IStorm3D* storm3d );
	~CinematicScreen();

	void update();
	void close();

	bool isOpen() const;
	bool shouldBeDeleted() const;
	bool hasVideo() const;

	void raise();

private:
	CinematicScreenImpl* impl;

	
};

///////////////////////////////////////////////////////////////////////////////
} // end of namespace ui

#endif
