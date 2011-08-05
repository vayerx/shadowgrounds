
#ifndef GAMEPOINTERS_H
#define GAMEPOINTERS_H

#include <c2_vectors.h>

#define GPOINTER_DUMMY 0
#define GPOINTER_WAYPOINT 1
#define GPOINTER_FINALPOINT 2
#define GPOINTER_TARGET 3
#define GPOINTER_GROUNDTARGET 4
#define GPOINTER_SELECTED 5
#define GPOINTER_UNSELECTED 6
#define GPOINTER_ENEMY 7
#define GPOINTER_FRIENDLY 8
#define GPOINTER_UNSEL_TARGET 9
#define GPOINTER_UNSEL_GROUNDTARGET 10
#define GPOINTER_UNSEL_FINALPOINT 11
#define GPOINTER_UNSEL_SNEAKPOINT 12
#define GPOINTER_SNEAKPOINT 13
#define GPOINTER_HOSTILE_SIGHT 14
#define GPOINTER_UNSEL_SPRINTPOINT 15
#define GPOINTER_SPRINTPOINT 16

#define GPOINTER_AMOUNT 17


class LinkedList;
class IStorm3D_Material;

namespace game
{
  class GameScene;
}

namespace ui
{

  class VisualObjectModel;
  class VisualObject;
  class IPointableObject;

  /**
   * Class for showing 3d game pointers (selections, waypoints and stuff).
   */

  class GamePointers
  {
  public:
    GamePointers(game::GameScene *gameScene);
    ~GamePointers();

    void addPointer(const VC3 &position, int pointerType, 
      const IPointableObject *lockedTo, const IPointableObject *lineFrom,
			float maxDistance = -1.0f);

    void clearPointers();

    // update pointer positions to match the objects they are locked to
    // call this on regular basis
    void updatePositions();

    // call before rendering
    void prepareForRender();

  private:
		game::GameScene *gameScene;

    LinkedList *pointers;
    VisualObjectModel **models;
    VisualObject **objects;
    IStorm3D_Material **materials;
  };

}

#endif
