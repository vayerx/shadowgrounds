
#ifndef LOADINGMESSAGE_H
#define LOADINGMESSAGE_H

class IStorm3D;
class IStorm3D_Scene;
class Ogui;


#ifdef PROJECT_SURVIVOR
	#define SHOW_LOADING_BAR(i) ui::LoadingMessage::showLoadingBar(i);
	#define SET_LOADING_BAR_TEXT(s) ui::LoadingMessage::setLoadingBarText(s);
#else
#ifdef PROJECT_AOV
	#define SHOW_LOADING_BAR(i) ui::LoadingMessage::showLoadingBar(i);
	#define SET_LOADING_BAR_TEXT(s) ui::LoadingMessage::setLoadingBarText(s);
#else
	#define SHOW_LOADING_BAR(i) 
	#define SET_LOADING_BAR_TEXT(s) 
#endif
#endif

namespace ui
{	
	class LoadingMessage
	{
		public:
			static void setManagers(IStorm3D *s3d, 
				IStorm3D_Scene *scene, Ogui *ogui);

			static void showLoadingMessage(const char *loadingMessage);
			static void clearLoadingMessage();

			static void showLoadingBar(int value);
			static void setLoadingBarText(const char *text);

		private:
			static void renderLoadingMessage();
			static IStorm3D *storm3d;
			static IStorm3D_Scene *scene;
			static Ogui *ogui;
	};
}

#endif



