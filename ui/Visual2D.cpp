
#include "precompiled.h"

#include <stdlib.h> // for abort
#include "Visual2D.h"
#include "../ogui/Ogui.h"

#include "../util/Debug_MemoryManager.h"

namespace ui
{
  
  Ogui *Visual2D::visualOgui = NULL;

  void Visual2D::setVisualOgui(Ogui *ogui)
  {
    visualOgui = ogui;
  }

  Visual2D::Visual2D(char *filename)
  {
    if (filename != NULL)
    {
      // must set the ogui for visualization first...
      if (visualOgui == NULL) abort();

      // want to catch exceptions here if not debugging
// TODO: put these back!
//#ifndef _DEBUG
      try {
//#endif
        image = visualOgui->LoadOguiImage(filename);
//#ifndef _DEBUG
      } catch(OguiException *e) {
        image = NULL;
        delete e;
      }
//#endif
    }
  }

  Visual2D::Visual2D(int renderTargetIndex)
	{
		image = visualOgui->GetOguiRenderTarget(renderTargetIndex);
	}

  Visual2D::~Visual2D()
  {
    if (image != NULL)
    {
      delete image;
      image = NULL;
    }
  }

  IOguiImage *Visual2D::getImage() 
  {
    return image;
  }

}

