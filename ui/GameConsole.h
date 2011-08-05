
#ifndef GAMECONSOLE_H
#define GAMECONSOLE_H

#include "ErrorWindow.h"

#define GAMECONSOLE_MAX_INPUT_LEN 256

#ifdef LEGACY_FILES
#define GAMECONSOLE_HISTORY 32
#else
#define GAMECONSOLE_HISTORY 64
#endif

namespace game
{
	class GameScripting;
}

namespace ui
{
	class GameConsole
	{
		public:
			GameConsole(ErrorWindow *errorWin, game::GameScripting *gs);

			~GameConsole();

			void add(char ascii);

			void enter();

			void cancel();

			void tab();

			void erasePrev();
			void eraseNext();

			void prevChar();
			void nextChar();

			void show();
			void hide();
			bool isVisible();

			void prevHistory();
			void nextHistory();

			void setLine(const char *line);

			void setMiniQueryMode();

			void loadHistory(const char *filename);
			void saveHistory(const char *filename);

		private:
			ErrorWindow *errorWindow;
			game::GameScripting *gameScripting;

			char inputBuf[GAMECONSOLE_MAX_INPUT_LEN + 1];
			int inputBufUsed;
			char inputBufRight[GAMECONSOLE_MAX_INPUT_LEN + 1];
			int inputBufRightUsed;

			char *historyBufs[GAMECONSOLE_HISTORY];
			int historyNumber;
			int atHistory;

			bool miniQueryMode;

      bool autocompleteOption();

      bool autocompleteCommand();

	};
}

#endif

