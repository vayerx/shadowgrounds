
#ifndef VISUAL2D_H
#define VISUAL2D_H

#include "../ogui/Ogui.h"  

//
// The 2D visualization of an armor part, character face, etc.
//

//
// You should usually think of this as an interface and not presume that
// it has any real methods... unless you are dealing with the game ui 
// implementation and need to draw the visualization on screen for real.
//

// NOTICE: uses a static variable to store instance of ogui for convinience.
// thus may not behave properly if you have multiple instances of ogui.

namespace ui
{

  class Visual2D
  {
  public:
    static void setVisualOgui(Ogui *ogui);

    Visual2D(char *filename);
    Visual2D(int renderTargetIndex);
    ~Visual2D();

    IOguiImage *getImage();

  private:
    static Ogui *visualOgui;

    IOguiImage *image;
  };

}

#endif
