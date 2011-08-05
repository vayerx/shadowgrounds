
#ifndef SELECTIONBOX_H
#define SELECTIONBOX_H

#include <c2_vectors.h>

// dragging the mouse at least this much during mouse button down
// will enable the selection box, if mouse has not moved outside these
// limits, the selection box will not be active.
#define SELECTION_BOX_MIN_SIZE 16


namespace game
{
  class Game;
}

namespace ui
{
  class SelectionBox
  {
  public:
	  typedef enum {
		  BOX_SELECT,         // Resets current selection, and select only new ones
		  BOX_ADD_SELECTION,  // Add to already selected
		  BOX_SUB_SELECTION   // Remove from already selected
	  } BoxSelectionType;

    SelectionBox(game::Game *game, int player, int renderScaleX, int renderScaleY);

    ~SelectionBox();

    // called at selection start (mouse button pressed down)
    void selectionStarted(int startX, int startY);

    // called every while button is pressed down
    // (every frame or possible every time the cursor moves)
    // NOTE: currently called all the time!
    void selectionPositionUpdate(int posX, int posY);

    // called at selection end (mouse button released)
    void selectionEnded(int endX, int endY);

    // called every frame while mouse button down
    // NOTE: currently called all the time!
    void render();

    // do stuff... the actual selection implementation
    //void doBoxSelection(BoxSelectionType type, VC2& a, VC2& b, VC2& c, VC2& d);
    void doBoxSelection(BoxSelectionType type);

    // return true if selectionbox is active (mouse is being dragged)
    // (that is, mouse position has moved >16px during mouse button down)
    bool selectionActive();

  private:
    game::Game *game;
    int player;

    int boxStartX;
    int boxStartY;
    int currentX;
    int currentY;
		bool active;

		int renderScaleX;
		int renderScaleY;

    // something to do with the selection box. corners maybe?
		VC3 ad,bd,cd,dd;

  };
}

#endif


