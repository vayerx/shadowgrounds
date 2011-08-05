#ifndef INCLUDED_TACTICALUNITWINDOW_H
#define INCLUDED_TACTICALUNITWINDOW_H

#include "../ogui/Ogui.h"
#include <c2_vectors.h>
#include <vector>

namespace game {
	class Game;
	class Unit;
}


namespace ui {

struct TacticalUnitWindowData;

class TacticalUnitWindow: public IOguiButtonListener
{
	TacticalUnitWindowData *data;

public:
	TacticalUnitWindow(Ogui *ogui, game::Game *game);
	~TacticalUnitWindow();

	// Execute the selected command (button)
	void executeCommand();

	// Track window evens
	void CursorEvent(OguiButtonEvent *event);
	// Call for each frame
	void update();

	bool isVisible();

	// this tells us if the window close has been acknowledged. 
	// need this so we can skip the next button click
	// ack automatically set by the tacticalunitwindow itself, if some
	// of the options are chosen (not closed by the "autoclose popup code").
	// --jpk
	bool isClosedAck();

	// acknowledge closing
	void setClosedAck();

	// Shows window at given point. Set's visible & reset's settings.
	// Pauses game if needed

	// Clicked over unit
	void showAt(int xPosition, int yPosition, std::vector<game::Unit *> *unitsSelected, game::Unit *targetUnit);
	// Clicked over terrain
	void showAt(int xPosition, int yPosition, std::vector<game::Unit *> *unitsSelected, const VC2 &targetPosition);

	// Clicked over existing waypoint. Do we need this?
	//void showAt(int xPosition, int yPosition, game::Unit *unit, int waypointIndex);

	// Force hide. Unpauses game if neede
	void hide();

	// Called to signal that the tactical stuff has been done and
	// it is time to continue. (unpause)
	void tacticalDone();

	// Called when the 3rd click has been done (if the tactical
	// menu requested one more click on the scene before executing
	// the order)
	void doTacticalClick(VC3 &position, game::Unit *targetUnit);

	// To make the menu visible again
	// (if the nasty autoclose has closed it againts our will ;)
	void keepVisible();
};

} // end of namespace ui

#endif
