
#include "precompiled.h"

#include "GameConsole.h"
#include "../convert/str2int.h"
#include "../system/Logger.h"
#include "../game/scripting/GameScripting.h"
#include "../game/scripting/GameScriptData.h"
#include "../util/ScriptManager.h"
#include "../util/SimpleParser.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_debug.h"

#include "../filesystem/input_stream_wrapper.h"
#include "../util/fb_assert.h"
#include "../util/Debug_MemoryManager.h"

#include "../game/userdata.h"

extern bool apply_options_request;
using namespace frozenbyte;

namespace ui
{

	GameConsole::GameConsole(ErrorWindow *errorWin, game::GameScripting *gs)
	{
		this->errorWindow = errorWin;
		this->gameScripting = gs;

		inputBufUsed = 0;
		inputBuf[0] = '\0';
		inputBufRightUsed = 0;
		inputBufRight[0] = '\0';

		for (int i = 0; i < GAMECONSOLE_HISTORY; i++)
		{
			historyBufs[i] = new char[(GAMECONSOLE_MAX_INPUT_LEN * 2) + 1];
			historyBufs[i][0] = '\0';
		}
		historyNumber = 0;
		atHistory = 0;
		miniQueryMode = false;

		if (game::SimpleOptions::getBool(DH_OPT_B_CONSOLE_HISTORY_SAVE))
		{
			loadHistory(igios_mapUserDataPrefix("Config/console_history.txt").c_str());
		}
	}

	GameConsole::~GameConsole()
	{
		if (game::SimpleOptions::getBool(DH_OPT_B_CONSOLE_HISTORY_SAVE))
		{
			saveHistory(igios_mapUserDataPrefix("Config/console_history.txt").c_str());
		}

		for (int i = 0; i < GAMECONSOLE_HISTORY; i++)
		{
			delete [] historyBufs[i];
		}
	}

	void GameConsole::add(char ascii)
	{
		if (inputBufUsed < GAMECONSOLE_MAX_INPUT_LEN)
		{
			inputBuf[inputBufUsed] = ascii;
			inputBuf[inputBufUsed + 1] = '\0';
			inputBufUsed++;
		}
		errorWindow->setInputLine(inputBuf, inputBufRight);
	}

	void GameConsole::erasePrev()
	{
		if (inputBufUsed > 0)
		{
			inputBufUsed--;
			inputBuf[inputBufUsed] = '\0';
		}
		errorWindow->setInputLine(inputBuf, inputBufRight);
	}

	void GameConsole::eraseNext()
	{
		if (inputBufRightUsed > 0)
		{
			for (int i = 0; i < inputBufRightUsed-1; i++)
			{
				inputBufRight[i] = inputBufRight[i+1];
			}
			inputBufRightUsed--;
			inputBufRight[inputBufRightUsed] = '\0';
		}
		errorWindow->setInputLine(inputBuf, inputBufRight);
	}

	void GameConsole::nextChar()
	{
		if (inputBufRightUsed > 0 && inputBufUsed < GAMECONSOLE_MAX_INPUT_LEN - 1)
		{
			char tmp = inputBufRight[0];
			eraseNext();
			add(tmp);
		}
		errorWindow->setInputLine(inputBuf, inputBufRight);
	}

	void GameConsole::prevChar()
	{
		if (inputBufUsed > 0 && inputBufRightUsed < GAMECONSOLE_MAX_INPUT_LEN - 1)
		{
			char tmp = inputBuf[inputBufUsed - 1];
			erasePrev();
			for (int i = inputBufRightUsed; i > 0; i--)
			{
				inputBufRight[i] = inputBufRight[i-1];
			}
			inputBufRightUsed++;
			inputBufRight[inputBufRightUsed] = '\0';
			inputBufRight[0] = tmp;
		}
		errorWindow->setInputLine(inputBuf, inputBufRight);
	}

	void GameConsole::enter()
	{
		if (inputBufUsed + inputBufRightUsed != 0)
		{ 
			strcpy(historyBufs[historyNumber % GAMECONSOLE_HISTORY], inputBuf);
			strcat(historyBufs[historyNumber % GAMECONSOLE_HISTORY], inputBufRight);

			char *inp = historyBufs[historyNumber % GAMECONSOLE_HISTORY];
			int inpLen = strlen(inp);

			if (historyBufs[historyNumber % GAMECONSOLE_HISTORY] != NULL
				&& historyBufs[(historyNumber + GAMECONSOLE_HISTORY-1) % GAMECONSOLE_HISTORY] != NULL
				&& strcmp(historyBufs[historyNumber % GAMECONSOLE_HISTORY], historyBufs[(historyNumber + GAMECONSOLE_HISTORY-1) % GAMECONSOLE_HISTORY]) == 0)
			{
				// don't advance history if exactly same as previous history line.
			} else {
				historyNumber++;
			}

			atHistory = historyNumber;

			if (inp[0] == '/')
			{
				Logger::getInstance()->debug("GameConsole::enter - Executing console option command...");

				int valueSep = -1;
				for (int i = 0; i < inpLen; i++)
				{
					if (inp[i] == ' ')
					{
						// note: modifies history buffer, space char restored later on...
						inp[i] = '\0';
						valueSep = i;
						break;
					}
				}

				game::GameOptionManager *oman = game::GameOptionManager::getInstance();
				game::GameOption *opt = oman->getOptionByName(&inp[1]);

				if (opt != NULL)
				{
					if (valueSep != -1
						&& (inp[valueSep + 1] != '\0'
						|| opt->getVariableType() == game::IScriptVariable::VARTYPE_STRING))
					{
						if (opt->getVariableType() == game::IScriptVariable::VARTYPE_STRING)
						{
							opt->setStringValue(&inp[valueSep + 1]);
						} 
						else if (opt->getVariableType() == game::IScriptVariable::VARTYPE_INT)
						{
							opt->setIntValue(str2int(&inp[valueSep + 1]));
						} 
						else if (opt->getVariableType() == game::IScriptVariable::VARTYPE_FLOAT)
						{
							opt->setFloatValue((float)atof(&inp[valueSep + 1]));
						} 
						else if (opt->getVariableType() == game::IScriptVariable::VARTYPE_BOOLEAN)
						{
							if (str2int(&inp[valueSep + 1]) != 0)
								opt->setBooleanValue(true);
							else
								opt->setBooleanValue(false);
						} 
						if (opt->doesNeedApply())
							::apply_options_request = true;
						Logger::getInstance()->info("GameConsole::enter - Option value changed.");
					} else {
						Logger::getInstance()->debug("GameConsole::enter - No new value for option given, so nothing done.");
					}
				} else {
					Logger::getInstance()->warning("GameConsole::enter - Option with given name does not exist.");
				}

				if (valueSep != -1)
				{
					inp[valueSep] = ' ';
				}
			} else {
				Logger::getInstance()->debug("GameConsole::enter - Executing console command...");

				int oldLevel = Logger::getInstance()->getListenerLogLevel();
				if (game::SimpleOptions::getBool(DH_OPT_B_DEBUG_CONSOLE_COMMANDS))
				{
					Logger::getInstance()->setListenerLogLevel(LOGGER_LEVEL_DEBUG);
				}

				const char *fileName = "memory/console";

				char *databuf = new char[inpLen + 256];

				sprintf(databuf, "script _console_command;sub _exec;%s;endSub;endScript\r\n", inp);

				gameScripting->loadMemoryScripts(fileName, databuf, strlen(databuf));

				delete [] databuf;

				gameScripting->runMissionScript("_console_command", "_exec");

				Logger::getInstance()->setListenerLogLevel(oldLevel);
			}
		}
		inputBufUsed = 0;
		inputBuf[0] = '\0';
		inputBufRightUsed = 0;
		inputBufRight[0] = '\0';
		errorWindow->setInputLine("", "");

		if (miniQueryMode)
		{
			errorWindow->setMiniWindowMode(false);
			errorWindow->clearMessages();
			miniQueryMode = false;
			hide();
		}
	}

	void GameConsole::cancel()
	{
		inputBufUsed = 0;
		inputBuf[0] = '\0';
		inputBufRightUsed = 0;
		inputBufRight[0] = '\0';
		errorWindow->setInputLine(NULL, NULL);

		if (miniQueryMode)
		{
			errorWindow->setMiniWindowMode(false);
			miniQueryMode = false;
		}
	}

	void GameConsole::prevHistory()
	{
		if (atHistory <= historyNumber - GAMECONSOLE_HISTORY
			|| atHistory <= 0)
			return;

		atHistory--;

		strcpy(inputBuf, historyBufs[atHistory % GAMECONSOLE_HISTORY]);
		inputBufUsed = strlen(inputBuf);
		inputBufRight[0] = '\0';
		inputBufRightUsed = 0;
		errorWindow->setInputLine(inputBuf, inputBufRight);
	}

	void GameConsole::nextHistory()
	{
		if (atHistory >= historyNumber)
			return;

		atHistory++;

		if (atHistory == historyNumber)
		{
			inputBufUsed = 0;
			inputBuf[0] = '\0';
			inputBufRightUsed = 0;
			inputBufRight[0] = '\0';
			errorWindow->setInputLine("", "");
		} else {
			strcpy(inputBuf, historyBufs[atHistory % GAMECONSOLE_HISTORY]);
			inputBufUsed = strlen(inputBuf);
			inputBufRightUsed = 0;
			inputBufRight[0] = '\0';
			errorWindow->setInputLine(inputBuf, inputBufRight);
		}
	}


	bool GameConsole::autocompleteOption()
	{
		int optStart = 0;
		if (inputBuf[0] == '/')
		{
			optStart = 1;
		}

		// NO OPTION AUTOCOMPLETE WITHOUT STARTING SLASH
		// (could autocomplete in such cases too, but then there would
		// be no command autocomplete in most cases)
		// well, maybe if more than 2 letters?
		if (optStart != 1 && inputBufUsed < 2)
			return false;

		char buf[256+1];
		buf[0] = '\0';

		int matches = 0;
		int smallestMatchLength = 0;
		game::GameOptionManager *oman = game::GameOptionManager::getInstance();
		const LinkedList *optlist = oman->getOptionsList();
		LinkedListIterator iter(optlist);
		while (iter.iterateAvailable())
		{
			game::GameOption *opt = (game::GameOption *)iter.iterateNext();
			const char *optname = oman->getOptionNameForId(opt->getId());
			if (strncmp(&inputBuf[optStart], optname, inputBufUsed - optStart) == 0)
			{
				if (matches == 0)
				{
					if(strlen(optname) < 256)
					{
						strcpy(buf, optname);
						smallestMatchLength = strlen(optname);
					} else {
						assert(0);
					}
				} else {
					int slen = strlen(optname);
					int buflen = strlen(buf);
					if (buflen < slen)
						slen = buflen;
					for (int i = 0; i < slen; i++)
					{
						if (buf[i] != optname[i])
						{
							if (smallestMatchLength > i)
								smallestMatchLength = i;
							break;
						} 						 
					}
				}
				matches++;
			}
		}

		//if (matches >= 2)
		if (matches >= 1)
		{
			errorWindow->logMessage("", LOGGER_LEVEL_ERROR);
			LinkedListIterator iter2(optlist);
			while (iter2.iterateAvailable())
			{
				game::GameOption *opt = (game::GameOption *)iter2.iterateNext();
				const char *optname = oman->getOptionNameForId(opt->getId());
				if (strncmp(&inputBuf[optStart], optname, inputBufUsed - optStart) == 0)
				{
					char logbuf[512+4+1];
					logbuf[0] = '\0';
					int onlen = strlen(optname);
					if(onlen < 256)
					{
						strcpy(logbuf, optname);
					}
					if (opt->getVariableType() == game::IScriptVariable::VARTYPE_STRING)
					{
						char *val = opt->getStringValue();
						if (onlen + strlen(val) < 512)
						{
							strcat(logbuf, " (");
							strcat(logbuf, val);
							strcat(logbuf, ")");
						}
					}
					else if (opt->getVariableType() == game::IScriptVariable::VARTYPE_INT)
					{
						char *val = int2str(opt->getIntValue());
						strcat(logbuf, " (");
						strcat(logbuf, val);
						strcat(logbuf, ")");
					}
					else if (opt->getVariableType() == game::IScriptVariable::VARTYPE_FLOAT)
					{
						sprintf(&logbuf[strlen(logbuf)], " (%f)", opt->getFloatValue());
					}
					else if (opt->getVariableType() == game::IScriptVariable::VARTYPE_BOOLEAN)
					{
						const char *val;
						if (opt->getBooleanValue())
							val = "1";
						else
							val = "0";
						strcat(logbuf, " (");
						strcat(logbuf, val);
						strcat(logbuf, ")");
					}
					errorWindow->logMessage(logbuf, LOGGER_LEVEL_ERROR);
				}
			}
		}

		if (matches >= 1)
		{
			if (1 + smallestMatchLength + 1 < GAMECONSOLE_MAX_INPUT_LEN)
			{
				assert((int)strlen(buf) >= smallestMatchLength);
				if (optStart == 0)
				{
					strcpy(inputBuf, "/");
					optStart = 1;
				}
				strncpy(&inputBuf[optStart], buf, smallestMatchLength);
				inputBuf[optStart + smallestMatchLength] = '\0';
				if (matches == 1)
					strcat(inputBuf, " ");
				inputBufUsed = strlen(inputBuf); 
				errorWindow->setInputLine(inputBuf, inputBufRight);
			}
			return true;
		} else {
			return false;
		}
	}


	bool GameConsole::autocompleteCommand()
	{
		// HACK!!!
		const char *fileName = "memory/console";
		if (util::ScriptManager::getInstance()->getScript(fileName) == NULL)
		{
			char *databuf = new char[256];
			sprintf(databuf, "script _console_command;sub _exec;endSub;endScript\r\n");
			gameScripting->loadMemoryScripts(fileName, databuf, strlen(databuf));
			delete [] databuf;
		}

		util::ScriptProcess *sp = gameScripting->startNonUnitScript("_console_command", "_exec");
		if (sp == NULL)
			return false;

		int matchAmount = 0;
		char matchFor[256+1];
		matchFor[0] = '\0';
		int matchCut = 0;
		for (int i = inputBufUsed - 1; i >= 0; i--)
		{
			if (inputBuf[i] == ';')
			{
				while (i <= inputBufUsed - 1 && inputBuf[i + 1] == ' ')
				{
					i++;
				}
				if ((inputBufUsed - i) < 256)
				{
					strcpy(matchFor, &inputBuf[i + 1]);
					matchCut = i + 1;
					break;
				}
			}
		}
		if (matchCut == 0)
		{
			if (inputBufUsed < 256)
			{
				strcpy(matchFor, inputBuf);
			}
		}
		int smallestMatchLength = 0;
		char *buf = gameScripting->matchSuitableCommands(sp, &matchAmount, matchFor, &smallestMatchLength);
		if (matchAmount > 0)
		{
			assert(buf != NULL);
			if (matchAmount > 1)
			{
				errorWindow->logMessage("", LOGGER_LEVEL_ERROR);
				errorWindow->logMessage(buf, LOGGER_LEVEL_ERROR);
			}
			if (matchCut + smallestMatchLength + 1 < GAMECONSOLE_MAX_INPUT_LEN)
			{
				assert((int)strlen(buf) >= smallestMatchLength);
				strncpy(&inputBuf[matchCut], buf, smallestMatchLength);
				inputBuf[matchCut + smallestMatchLength] = '\0';
				inputBufUsed = strlen(inputBuf); 
				errorWindow->setInputLine(inputBuf, inputBufRight);
			}
			delete [] buf;
		}
		if (sp != NULL)
		{
			// WARNING: unsafe cast!
			game::GameScriptData *gsd = (game::GameScriptData *)sp->getData();
			delete gsd;
		}
		delete sp;

		// FIXME: should return true if was autocompleted
		return false;
	}

	void GameConsole::tab()
	{
		if (inputBufUsed == 0)
			return;

		autocompleteOption();

		autocompleteCommand();
	}

	void GameConsole::hide()
	{
		bool wasMini = miniQueryMode;
		cancel();
		if (wasMini)
		{
			errorWindow->hide();
		} else {
			errorWindow->hideWithEffect();
		}
	}

	void GameConsole::setMiniQueryMode()
	{
		errorWindow->clearMessages();
		errorWindow->setMiniWindowMode(true);
		miniQueryMode = true;
	}

	void GameConsole::show()
	{
		inputBufUsed = 0;
		inputBuf[0] = '\0';
		inputBufRightUsed = 0;
		inputBufRight[0] = '\0';
		errorWindow->setInputLine("", "");

		errorWindow->raise();
		errorWindow->show();
	}

	bool GameConsole::isVisible()
	{
		return errorWindow->isVisible();
	}

	void GameConsole::setLine(const char *line)
	{
		//inputBufUsed = 0;
		//inputBuf[0] = '\0';
		//errorWindow->setInputLine("");

		if (strlen(line) < GAMECONSOLE_MAX_INPUT_LEN-1)
		{
			strcpy(inputBuf, line);
			inputBufUsed = strlen(inputBuf);
			errorWindow->setInputLine(inputBuf, inputBufRight);
		}
	}

	void GameConsole::loadHistory(const char *filename)
	{
		filesystem::FB_FILE *f = filesystem::fb_fopen(filename, "rb");
		if (f != NULL)
		{
			filesystem::fb_fclose(f);
		} else {
			Logger::getInstance()->debug("GameConsole::loadHistory - Console history file open failed (assuming no previous history saved).");
			return;
		}

		util::SimpleParser sp;
		if (sp.loadFile(filename))
		{
			int h = 0;
			while (sp.next())
			{
				char *l = sp.getLine();
				if (l != NULL)
				{
					if (strlen(l) < GAMECONSOLE_MAX_INPUT_LEN - 2)
					{
						strcpy(historyBufs[h], l);
					}
				} else {
					break;
				}
				if (l[0] == '\0')
					break;
				h++;
				if (h >= GAMECONSOLE_HISTORY)
				{
					break;
				}
			}
			historyNumber = h;			
			atHistory = historyNumber;
		} else {
			Logger::getInstance()->warning("GameConsole::loadHistory - Error loading history file even though it existed.");
		}
	}

	void GameConsole::saveHistory(const char *filename)
	{
		FILE *f = fopen(filename, "wb");
		fb_assert(historyNumber >= 0);
		if (f != NULL)
		{
			int h = historyNumber;
			while (true)
			{
				if (historyBufs[h % GAMECONSOLE_HISTORY][0] != '\0')
				{
					fprintf(f, "%s\r\n", historyBufs[h % GAMECONSOLE_HISTORY]);
				}
				h++;
				if ((h % GAMECONSOLE_HISTORY) == (historyNumber % GAMECONSOLE_HISTORY))
					break;
			}
			fclose(f);
		} else {
			Logger::getInstance()->warning("GameConsole::saveHistory - Failed to open console history file for writing.");
		}
	}

}

