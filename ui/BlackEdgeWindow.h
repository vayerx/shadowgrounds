
#ifndef BLACKEDGEWINDOW_H
#define BLACKEDGEWINDOW_H

class Ogui;
class OguiWindow;

namespace ui
{
  /**
   * A window used just for creating a black area at the top or bottom 
	 * of the screen (approx. 16:9 movie effect - 2:1 to be precise).
   * @version 1.0, 30.12.2002
   * @author Jukka Kokkonen <jukka@frozenbyte.com>
   * @see MovieAspectWindow
   */
	
	class BlackEdgeWindow
	{
	public:
		BlackEdgeWindow(Ogui *ogui);
		
		~BlackEdgeWindow();
		
		void moveTo(int x, int y);

		void update();

	private:
		Ogui *ogui;
		OguiWindow *win;
		
	};
	
}

#endif


