
#ifndef MOVIEASPECTWINDOW_H
#define MOVIEASPECTWINDOW_H

class Ogui;

namespace ui
{
  class BlackEdgeWindow;

  /**
   * A window used just for creating a black areas at both top and bottom 
	 * of the screen (approx. 16:9 movie effect - 2:1 to be precise).
   * @version 1.0, 30.12.2002
   * @author Jukka Kokkonen <jukka@frozenbyte.com>
   * @see BlackEdgeWindow
   * @see UIEffects
   */

	class MovieAspectWindow
	{
		public:
			MovieAspectWindow(Ogui *ogui);
			
			~MovieAspectWindow();

			void update();
			
		private:
			BlackEdgeWindow *top;
			BlackEdgeWindow *bottom;			
	};
	
}

#endif


