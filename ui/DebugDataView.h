
#ifndef DEBUGDATAVIEW_H
#define DEBUGDATAVIEW_H

namespace game
{
	class Game;
}

namespace ui
{
	class DebugDataViewImpl;

	class DebugDataView
	{
		public:
			DebugDataView(game::Game *game);

			~DebugDataView();

			void run();

			void cleanup();

			//void setSpecialImageARGB(unsigned char *argbBuf);
			//void setSpecialImageValue(unsigned char *valueBuf);

			static DebugDataView *getInstance(game::Game *game);

			static void cleanInstance();

		private:
			DebugDataViewImpl *impl;
			static DebugDataView *instance;
	};
}

#endif

