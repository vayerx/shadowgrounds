
#ifndef GUIEFFECTWINDOW_H
#define GUIEFFECTWINDOW_H


#include <boost/utility.hpp>

#include <DatatypeDef.h>

class Ogui;
class OguiWindow;

namespace ui
{
	class GUIEffectWindow : public boost::noncopyable
	{
		public:

			GUIEffectWindow(Ogui *ogui, const char *layer1Image, const char *layer2Image,
				const char *layer3Image, const VC2I &position = VC2I(0,0), const VC2I &size = VC2I(1024,768));
				
			~GUIEffectWindow();

			void update(int msecTimeDelta);
			void raise();
			void show();
			void hide();

			void setTransparency(int value, int value2);
			void fadeIn(int time);
			void fadeOut(int time);

		private:

			Ogui *ogui;

			OguiWindow *win1;
			OguiWindow *win2;
			OguiWindow *win3;

			int effectTime;
			bool charWindow;

			float lastNoiseScrollX;
			float lastNoiseScrollY;
	};
}


#endif



