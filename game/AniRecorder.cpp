
#include "precompiled.h"

#include "AniRecorder.h"

#include "Ani.h"
#include "AniManager.h"
#include "AniTool.h"
#include "Game.h"
#include "scripting/GameScripting.h"
#include "GameUI.h"
#include "UnitList.h"
#include "UnitLevelAI.h"
#include "../editor/file_wrapper.h"
#include "../ui/GameCamera.h"
#include "../util/SimpleParser.h"
#include "../util/ScriptManager.h"
#include "../util/TextFileModifier.h"
#include "../util/TextFinder.h"
#include "../util/hiddencommand.h"
#include "../container/LinkedList.h"
#include "../convert/str2int.h"
#include "../system/Logger.h"

#include "../util/Debug_MemoryManager.h"

#include <stdio.h>


#define ANIRECORDER_MAX_ANIS 32

// note: this means something like ANIRECORDER_MAX_UNDOS - 2 actual undo levels
#define ANIRECORDER_MAX_UNDOS 32


using namespace util;


namespace game
{
	static bool anirecorder_cleanupSandbox = false;

	class AniRecorderImpl
	{
		public:
			AniRecorderImpl(Game *game)
			{
				for (int i = 0; i < ANIRECORDER_MAX_ANIS; i++)
				{
					anis[i] = NULL;
				}
				recordingAni = NULL;
				this->game = game;

				this->currentPosition = 0;
				this->currentEndPosition = 0;
				this->recordDir = NULL;
				this->recordBatchFile = NULL;
				this->cameraFile = NULL;
				this->recordIdFile = NULL;
				this->recordIdString = NULL;

				this->playing = false;
				this->recording = false;

				this->needScriptReload = true;

				// 1 sec min slider length
				this->longestAniLength = GAME_TICKS_PER_SECOND;

				this->endOfHistory = 0;
				this->atHistory = 0;
				for (int j = 0; j < ANIRECORDER_MAX_UNDOS; j++)
				{
					this->undoHistory[j] = NULL;
					this->undoHistoryDesc[j] = "";
				}
				this->addedLastHistory = false;
			}

			~AniRecorderImpl()
			{
				for (int i = 0; i < ANIRECORDER_MAX_ANIS; i++)
				{
					if (anis[i] != NULL)
						AniManager::getInstance()->deleteAni(anis[i]);
				}
				// TODO: delete dir/file name buffers?
			}

			int getAniNumberForUnit(Unit *unit)
			{
				int aninum = -1;
				for (int i = 0; i < ANIRECORDER_MAX_ANIS; i++)
				{
					if (anis[i] != NULL)
					{
						if (anis[i]->getUnit() == unit)
						{
							aninum = i;
							break;
						}
					}
				}
				assert(aninum != -1);
				return aninum;
			}

			Game *game;
			Ani *anis[ANIRECORDER_MAX_ANIS];
			Ani *recordingAni;
			int currentPosition;
			int currentEndPosition;
			char *recordDir;
			char *recordBatchFile;
			char *cameraFile;
			char *recordIdFile;
			char *recordIdString;
			bool playing;
			bool recording;
			int longestAniLength;
			bool needScriptReload;

			LinkedList *undoHistory[ANIRECORDER_MAX_UNDOS];
			std::string undoHistoryDesc[ANIRECORDER_MAX_UNDOS];
			int atHistory;
			int endOfHistory;
			bool addedLastHistory;
	};


	AniRecorder::AniRecorder(Game *game)
	{
		this->impl = new AniRecorderImpl(game);
		if (Ani::getRecordPath() == NULL)
		{
#ifdef LEGACY_FILES
			Ani::setRecordPath("Data/Record");
#else
			Ani::setRecordPath("data/record");
#endif
			anirecorder_cleanupSandbox = true;
		}
		setRecords(Ani::getRecordPath());
		anirecorder_cleanupSandbox = false;

    UnitLevelAI::setAllEnabled(false);
		// player and hostiles disabled, doors and neutrals not
		//UnitLevelAI::setPlayerAIEnabled(0, false);
		//UnitLevelAI::setPlayerAIEnabled(1, false);
	}


	AniRecorder::~AniRecorder()
	{
    //UnitLevelAI::setAllEnabled(true);
		//UnitLevelAI::setPlayerAIEnabled(0, true);
		//UnitLevelAI::setPlayerAIEnabled(1, true);
		delete impl;
	}


	void AniRecorder::reload()
	{
		Game *game = impl->game;
		delete impl;
		this->impl = new AniRecorderImpl(game);
		setRecords(Ani::getRecordPath());
	}


	void AniRecorder::calculateLongestAniLength()
	{
		int posBeforeCalc = impl->currentPosition;

		int initialLength = impl->longestAniLength;
		int lastLength = impl->longestAniLength;
		for (int i = 1; i < 60; i++)
		{
			stop();
			seekToTime(i * GAME_TICKS_PER_SECOND);
			play();
			AniManager::getInstance()->run();
			run();
			int prevFrame = impl->longestAniLength;
			AniManager::getInstance()->run();
			run();
			stop();
			if (impl->longestAniLength > lastLength
				&& impl->longestAniLength != prevFrame)
			{
				lastLength = impl->longestAniLength;
			} else {
				if (i * GAME_TICKS_PER_SECOND > initialLength)
				{
					break;
				}
			}
		}

		impl->currentPosition = posBeforeCalc;
	}


	void AniRecorder::addUndoHistory(const char *description)
	{
		int delh = impl->atHistory;
		while(delh != ((impl->endOfHistory + 1) % ANIRECORDER_MAX_UNDOS))
		{
			LinkedList *dellist = impl->undoHistory[delh];
			if (dellist != NULL)
			{
				impl->undoHistory[delh] = NULL;
				impl->undoHistoryDesc[delh] = "";

				while (!dellist->isEmpty())
				{
					TextFileModifier *tfm = (TextFileModifier *)dellist->popLast();
					tfm->closeFile();
					delete tfm;
				}
			}
			
			delh = (delh + 1) % ANIRECORDER_MAX_UNDOS;
		}

		LinkedList *newlist = new LinkedList();
		frozenbyte::editor::FileWrapper fwbase(
			std::string(impl->recordDir), std::string("*.dhs"));
		std::vector<std::string> allBaseFiles = fwbase.getAllFiles();
		for (int j = 0; j < (int)allBaseFiles.size(); j++)
		{
			const char *realfilename = allBaseFiles[j].c_str();
			TextFileModifier *tfm = new TextFileModifier();
			bool loadok = tfm->loadFile(realfilename);
			if (loadok)
			{
				newlist->append(tfm);
			} else {
				Logger::getInstance()->warning("AniRecorder::addUndoHistory() - Failed to load file.");
				delete tfm;
			}
		}
		impl->undoHistory[impl->atHistory] = newlist;
		impl->undoHistoryDesc[impl->atHistory] = description;

		impl->atHistory = (impl->atHistory + 1) % ANIRECORDER_MAX_UNDOS;
		impl->endOfHistory = impl->atHistory;

		impl->addedLastHistory = false;
	}


	void AniRecorder::applyUndoHistory()
	{
		//int prevHistory = (impl->atHistory + ANIRECORDER_MAX_UNDOS - 1) % ANIRECORDER_MAX_UNDOS;

		//assert(impl->undoHistory[prevHistory] != NULL);
		assert(impl->undoHistory[impl->atHistory] != NULL);

		LinkedList *fileDataList = impl->undoHistory[impl->atHistory];
		LinkedListIterator iter(fileDataList);

		while (iter.iterateAvailable())
		{
			TextFileModifier *tfm = (TextFileModifier *)iter.iterateNext();
			tfm->saveFile();
		}

		// cannot do this, this would blow up the undo history (recreates the whole impl)
		//this->reload();
		impl->needScriptReload = true;

		// TODO: this should be optimized?
		int oldpos = impl->currentPosition;
		impl->longestAniLength = 0;
		impl->currentPosition = 0;
		rewind();
		calculateLongestAniLength();
		if (oldpos > this->getLongestAniLength())
			oldpos = this->getLongestAniLength();
		seekToTime(oldpos);
	}


	void AniRecorder::undo()
	{
		if (impl->atHistory == impl->endOfHistory)
		{
			if (!impl->addedLastHistory)
			{
				addUndoHistory("last undo history");
				int goBack = (impl->atHistory + ANIRECORDER_MAX_UNDOS - 1) % ANIRECORDER_MAX_UNDOS;
				impl->atHistory = goBack;
				impl->endOfHistory = goBack;

				impl->addedLastHistory = true;
			}
		}
		int prevHistory = (impl->atHistory + ANIRECORDER_MAX_UNDOS - 1) % ANIRECORDER_MAX_UNDOS;
		int prevHistory2 = (prevHistory + ANIRECORDER_MAX_UNDOS - 1) % ANIRECORDER_MAX_UNDOS;
		if (prevHistory != impl->endOfHistory
			&& prevHistory2 != impl->endOfHistory
			&& impl->undoHistory[prevHistory] != NULL)
		{
			impl->atHistory = prevHistory;
			applyUndoHistory();
		} else {
			Logger::getInstance()->warning("AniRecorder::undo() - Reached beginning of undo history.");
		}
	}


	void AniRecorder::redo()
	{
		if (impl->atHistory != impl->endOfHistory)
		{
			int nextHistory = (impl->atHistory + 1) % ANIRECORDER_MAX_UNDOS;
			impl->atHistory = nextHistory;
			//if (impl->atHistory != impl->endOfHistory)
			//{
				assert(impl->undoHistory[impl->atHistory] != NULL);
				applyUndoHistory();
			//}
		} else {
			Logger::getInstance()->warning("AniRecorder::undo() - Already reached end of undo history.");
		}
	}

	bool AniRecorder::canUndo()
	{
		int prevHistory = (impl->atHistory + ANIRECORDER_MAX_UNDOS - 1) % ANIRECORDER_MAX_UNDOS;
		int prevHistory2 = (prevHistory + ANIRECORDER_MAX_UNDOS - 1) % ANIRECORDER_MAX_UNDOS;
		if (prevHistory != impl->endOfHistory
			&& prevHistory2 != impl->endOfHistory
			&& impl->undoHistory[prevHistory] != NULL)
		{
			return true;
		} else {
			return false;
		}
	}

	bool AniRecorder::canRedo()
	{
		if (impl->atHistory != impl->endOfHistory)
		{
			return true;
		} else {
			return false;
		}
	}


	void AniRecorder::run()
	{
		if (impl->playing)
		{
			impl->currentPosition++;

			// have all anis ended?
			bool allEnded = true;
			for (int i = 0; i < ANIRECORDER_MAX_ANIS; i++)
			{
				if (impl->anis[i] != NULL)
				{
					if (!impl->anis[i]->hasPlayEnded())
					{
						allEnded = false;
					}
				}
			}
			if (allEnded && !impl->recording)
			{
				impl->longestAniLength = impl->currentPosition;

				stop();
			} else {
				if (impl->currentPosition > impl->longestAniLength)
				{
					impl->longestAniLength = impl->currentPosition;
				}
			}
		}
	}


	bool AniRecorder::isRecording()
	{
		return impl->recording;
	}

	bool AniRecorder::isPlaying()
	{
		return impl->playing;
	}


	void AniRecorder::setRecords(const char *recordDir)
	{
		Game *game = impl->game;
		delete impl;
		this->impl = new AniRecorderImpl(game);

		impl->needScriptReload = true;

		if (recordDir == NULL)
		{
			assert(0);
			return;
		}
		impl->recordDir = new char[strlen(recordDir) + 1];
		impl->recordBatchFile = new char[strlen(recordDir) + 64 + 1];
		impl->cameraFile = new char[strlen(recordDir) + 64 + 1];
		impl->recordIdFile = new char[strlen(recordDir) + 32 + 1];
		impl->recordIdString = new char[32];

		strcpy(impl->recordDir, recordDir);

		strcpy(impl->recordIdFile, recordDir);
		strcat(impl->recordIdFile, "/aniid.txt");

		strcpy(impl->recordIdString, "no_id");
		util::SimpleParser sp;
		if (sp.loadFile(impl->recordIdFile))
		{
			if (sp.next())
			{
				char *line = sp.getLine();
				if (line != NULL 
					&& line[0] != '\0'
					&& strlen(line) < 32)
				{
					strcpy(impl->recordIdString, line);
				} else {
					Logger::getInstance()->error("AniRecorder::setRecords - aniid.txt contained an invalid id string.");
				}
			} else {
				Logger::getInstance()->error("AniRecorder::setRecords - aniid.txt file did not contain any id string.");
			}
		} else {
			Logger::getInstance()->error("AniRecorder::setRecords - Failed to open aniid.txt file.");
		}

		if (anirecorder_cleanupSandbox)
		{
			// if this is sandbox, delete all records...
			// (make sure thats the case, by checking both id and path...
#ifdef LEGACY_FILES
			if (strcmp(recordDir, "Data/Record") == 0 && strcmp(impl->recordIdString, "sandbox") == 0)
#else
			if (strcmp(recordDir, "data/record") == 0 && strcmp(impl->recordIdString, "sandbox") == 0)
#endif
			{
				Logger::getInstance()->debug("AniRecorder::setRecords - Cleaning up sandbox.");

				// FIXME: MS Windows specific code!
				hiddencommand("Data\\Record\\sandbox_cleanup.bat");
			}
		}

		strcpy(impl->recordBatchFile, recordDir);
		strcat(impl->recordBatchFile, "/ani_batch_file_");
		strcat(impl->recordBatchFile, impl->recordIdString);
		strcat(impl->recordBatchFile, ".dhs");

		strcpy(impl->cameraFile, recordDir);
		strcat(impl->cameraFile, "/ani_camera_dumps_");
		strcat(impl->cameraFile, impl->recordIdString);
		strcat(impl->cameraFile, ".dhs");

		bool fileExists = false;
		FILE *f = fopen(impl->recordBatchFile, "rb");
		if (f != NULL)
		{
			fileExists = true;
			fclose(f);
		}

		if (!fileExists)
		{
			// first create initial file...
			FILE *f = fopen(impl->recordBatchFile, "wb");
			if (f != NULL)
			{
				fprintf(f, "#!dhs -nopp\r\n\r\n");
				fprintf(f, "// automatically generated file, hand edit at your own risk.\r\n\r\n");
				fprintf(f, "script ani_batch_file_%s\r\n\r\n", impl->recordIdString);
				fprintf(f, "sub play_ani_batch\r\n");
				fprintf(f, "// <--- ANIBATCH_ADD_MARKER --->\r\n// (keep your hands off this one)\r\n\r\n");
				fprintf(f, "endSub\r\n\r\n");
				fprintf(f, "endScript\r\n\r\n");
				fclose(f);
			} else {
				Logger::getInstance()->error("AniRecorder::setRecords - Failed to open file for writing.");
				return;
			}
		}

		util::TextFileModifier tfm;
		bool loadok = tfm.loadFile(impl->recordBatchFile);
		if (!loadok)
		{
			Logger::getInstance()->error("AniRecorder::setRecords - File load failed.");
		}

		for (int j = 0; j < ANIRECORDER_MAX_ANIS; j++)
		{
			char seekbuf[256];
			sprintf(seekbuf, "<--- START-ANI-ID-%d --->", j);
			if (tfm.setStartSelectionNearMarker(seekbuf))
			{
				sprintf(seekbuf, "<--- END-ANI-ID-%d --->", j);
				tfm.setEndSelectionNearMarker(seekbuf);
				
				char *aniStuffBuf = tfm.getSelectionAsNewBuffer();
				util::SimpleParser sp;
				sp.loadMemoryBuffer(aniStuffBuf, strlen(aniStuffBuf));
				while (sp.next())
				{
					char key[128+1];
					char value[128+1];
					char *line = sp.getLine();

					int llen = strlen(line);
					if (llen < 128)
					{
						for (int c = 0; c < llen; c++)
						{
							if (line[c] == ' ')
							{
								strcpy(key, line);
								key[c] = '\0';
								strcpy(value, &line[c + 1]);
								break;
							}
						}
					} else {
						key[0] = '\0';
						value[0] = '\0';
					}
					
					if (key != NULL)
					{
						if (strcmp(key, "setUnitByIdString") == 0)
						{
							int id = str2int(value);
							if (id != 0)
							{
								Unit *u = impl->game->units->getUnitById(id);
								if (u != NULL)
								{
									impl->anis[j] = AniManager::getInstance()->createNewAniInstance(u);
									//Ani::setRecordPath(impl->recordDir);
									char recnamebuf[128];
									sprintf(recnamebuf, "record_%d_%s", j, impl->recordIdString);
									impl->anis[j]->setName(recnamebuf);
								} else {
									Logger::getInstance()->error("AniRecorder::setRecords - Unit with given id number not found.");
									Logger::getInstance()->error("AniRecorder::setRecords - Or numeric id-string was misinterpreted as id-number.");
								}
							} else {
								Unit *u = impl->game->units->getUnitByIdString(value);
								if (u != NULL)
								{
									impl->anis[j] = AniManager::getInstance()->createNewAniInstance(u);
									//Ani::setRecordPath(impl->recordDir);
									char recnamebuf[128];
									sprintf(recnamebuf, "record_%d_%s", j, impl->recordIdString);
									impl->anis[j]->setName(recnamebuf);
								} else {
									Logger::getInstance()->error("AniRecorder::setRecords - Unit with given id-string not found.");
								}
							}
						}
					}
				}
			}
		}

		// NOTE:
		// A _very bad_ (inefficient) way to solve the longest ani length
		calculateLongestAniLength();
		rewind();

	}


	void AniRecorder::addScriptCommandContinued(Unit *unit, const char *scriptCommand)
	{
		assert(unit != NULL);

		int aninum = impl->getAniNumberForUnit(unit);

		if (aninum != -1)
		{
			char anifilebuf[512+32+1];
			if (strlen(impl->recordDir) < 256)
			{
				strcpy(anifilebuf, impl->recordDir);
				sprintf(&anifilebuf[strlen(anifilebuf)], "/ani_record_%d_%s.dhs", aninum, impl->recordIdString);

				util::TextFileModifier tfm;
				tfm.loadFile(anifilebuf);
				tfm.setStartSelectionToStart();
				tfm.setEndSelectionToEnd();
				char *tmp = tfm.getSelectionAsNewBuffer();

				util::TextFinder tfind(tmp, true);
				int tickPos = tfind.findOneOfMany("aniTick\r\n", (impl->currentPosition + 1));
				delete [] tmp;

				if (tickPos != -1)
				{
					tfm.setStartSelectionToPosition(tickPos);
					tfm.setEndSelectionToPosition(tickPos);
					tfm.addAfterSelection(scriptCommand);
				} else {
					Logger::getInstance()->warning("AniRecorder::addScriptCommand - Failed to find current position.");
				}
				tfm.saveFile();
				tfm.closeFile();
			}
		}

		impl->needScriptReload = true;
	}


	void AniRecorder::addScriptCommand(Unit *unit, const char *scriptCommand, const char *description)
	{
		addUndoHistory(description);
		addScriptCommandContinued(unit, scriptCommand);
	}


	void AniRecorder::play()
	{
		// make sure anis are not already running...
		stop();

		// first reload scripts, if necessary
		if (impl->needScriptReload)
		{
			impl->needScriptReload = false;
			util::ScriptManager::getInstance()->loadScripts(impl->recordBatchFile, impl->recordDir, true);
			for (int i = 0; i < ANIRECORDER_MAX_ANIS; i++)
			{
				if (impl->anis[i] != NULL)
				{
					char anifilebuf[512+32+1];
					if (strlen(impl->recordDir) < 256)
					{
						strcpy(anifilebuf, impl->recordDir);
						sprintf(&anifilebuf[strlen(anifilebuf)], "/ani_record_%d_%s.dhs", i, impl->recordIdString);
						util::ScriptManager::getInstance()->loadScripts(anifilebuf, impl->recordDir, true);
					}
				}
			}
		}

		// finally start running the anis
		{
			for (int i = 0; i < ANIRECORDER_MAX_ANIS; i++)
			{
				if (impl->anis[i] != NULL)
				{
					impl->anis[i]->startPlay();
				}
			}
		}

		// seek to current position
		if (impl->currentPosition > 1)
		{
			for (int i = 0; i < ANIRECORDER_MAX_ANIS; i++)
			{
				if (impl->anis[i] != NULL)
				{
					impl->anis[i]->leapToPosition(impl->currentPosition);
				}
			}
		}

		AniManager::getInstance()->run();

		impl->playing = true;
	}


	void AniRecorder::record(Unit *unit)
	{
		if (unit->getIdString() != NULL)
		{
			std::string desctmp = std::string("record unit ") + unit->getIdString();
			addUndoHistory(desctmp.c_str());
		} else {
			addUndoHistory("record unit (no id string)");
		}

		int recnum = impl->getAniNumberForUnit(unit);

		// temporarily remove the recorded unit (ani) from ani playlist...
		Ani *tmp = NULL;
		if (recnum != -1)
		{
			tmp = impl->anis[recnum];
			impl->anis[recnum] = NULL;
		}

		stop();

		// play the other anis
		play();

		// restore the recorded unit (ani) to ani playlist
		if (recnum != -1)
		{
			impl->anis[recnum] = tmp;
		}

		if (recnum != -1)
		{
			impl->recordingAni = AniManager::getInstance()->createNewAniInstance(unit);
			//Ani::setRecordPath(impl->recordDir);

			char recnamebuf[128];
			sprintf(recnamebuf, "record_%d_%s", recnum, impl->recordIdString);
			impl->recordingAni->setName(recnamebuf);

			assert(impl->currentPosition != -1);
			if (impl->currentPosition == 0)
			{
				impl->recordingAni->startRecord(false);
			} else {
				impl->recordingAni->startRecord(true, impl->currentPosition);
			}
		} else {
			Logger::getInstance()->error("AniRecorder::record - Failed to solve record number for unit.");
		}

		impl->recording = true;

	}


	void AniRecorder::addAniRecord(Unit *unit)
	{
		addUndoHistory("add unit");

		impl->needScriptReload = true;

		if (unit == NULL)
		{
			assert(0);
			return;
		}

		for (int i = 0; i < ANIRECORDER_MAX_ANIS; i++)
		{
			if (impl->anis[i] == NULL)
			{
				util::TextFileModifier tfm;
				bool loadok = tfm.loadFile(impl->recordBatchFile);
				if (!loadok)
				{
					Logger::getInstance()->error("AniRecorder::addAniRecord - File load failed.");
					return;
				}

				char seekbuf[256];
				for (int j = 0; j < ANIRECORDER_MAX_ANIS; j++)
				{
					bool aniUsed = false;
					if (impl->anis[j] != NULL)
						aniUsed = true;
					sprintf(seekbuf, "<--- START-ANI-ID-%d --->", j);
					if (tfm.setStartSelectionNearMarker(seekbuf) != aniUsed)
					{
						Logger::getInstance()->error("AniRecorder::addAniRecord - Ani batch file / ani memory integrity failure.");
					}
				}

				tfm.setStartSelectionNearMarker("<--- ANIBATCH_ADD_MARKER --->");
				tfm.setEndSelectionNearMarker("<--- ANIBATCH_ADD_MARKER --->");
				
				char addbuf[512];
				sprintf(addbuf, "\r\n// <--- START-ANI-ID-%d --->\r\n", i);
				tfm.addBeforeSelection(addbuf);
				if (unit->getIdString() != NULL)
				{
					sprintf(addbuf, "setUnitByIdString %s\r\n", unit->getIdString());
				} else {
					sprintf(addbuf, "setUnitByIdString %d\r\n", impl->game->units->getIdForUnit(unit));
					Logger::getInstance()->warning("AniRecorder::addAniRecord - Added unit has no id-string.");					
					Logger::getInstance()->warning("Attempting to use automatically generated unit id-number instead.");
					Logger::getInstance()->warning("This is not recommended and may not work properly.");
				}
				tfm.addBeforeSelection(addbuf);
				sprintf(addbuf, "showUnit\r\n");
				tfm.addBeforeSelection(addbuf);
				sprintf(addbuf, "startAniPlay record_%d_%s\r\n", i, impl->recordIdString);
				tfm.addBeforeSelection(addbuf);
				sprintf(addbuf, "restoreUnit\r\n");
				tfm.addBeforeSelection(addbuf);
				sprintf(addbuf, "// <--- END-ANI-ID-%d --->\r\n\r\n", i);
				tfm.addBeforeSelection(addbuf);

				tfm.saveFile();
				tfm.closeFile();

				impl->anis[i] = AniManager::getInstance()->createNewAniInstance(unit);
				char recnamebuf[128];
				sprintf(recnamebuf, "record_%d_%s", i, impl->recordIdString);
				impl->anis[i]->setName(recnamebuf);


				Ani *tmpAni = AniManager::getInstance()->createNewAniInstance(unit);
				//Ani::setRecordPath(impl->recordDir);
				tmpAni->setName(recnamebuf);
				tmpAni->startRecord(false, -1);
				AniManager::getInstance()->run();
				tmpAni->stopRecord();
				AniManager::getInstance()->deleteAni(tmpAni);


				return;
			}
		}
		Logger::getInstance()->warning("AniRecorder::addAniRecord - Maximum ani limit reached.");
	}


	void AniRecorder::removeAniRecord(Unit *unit)
	{
		addUndoHistory("remove unit");

		impl->needScriptReload = true;

		for (int i = 0; i < ANIRECORDER_MAX_ANIS; i++)
		{
			if (impl->anis[i] != NULL
				&& impl->anis[i]->getUnit() == unit)
			{
				util::TextFileModifier tfm;
				bool loadok = tfm.loadFile(impl->recordBatchFile);
				if (!loadok)
				{
					Logger::getInstance()->error("AniRecorder::removeAniRecord - File load failed.");
				}

				char rembuf[256];
				sprintf(rembuf, "<--- START-ANI-ID-%d --->", i);
				tfm.setStartSelectionNearMarker(rembuf);
				sprintf(rembuf, "<--- END-ANI-ID-%d --->", i);
				tfm.setEndSelectionNearMarker(rembuf);

				tfm.deleteSelection();

				tfm.saveFile();
				tfm.closeFile();

				AniManager::getInstance()->deleteAni(impl->anis[i]);
				impl->anis[i] = NULL;
				return;
			}
		}
		Logger::getInstance()->warning("AniRecorder::removeAniRecord - No ani exists for given unit.");
	}


	void AniRecorder::deleteCameraDump(int cameraDumpNumber)
	{
		addUndoHistory("delete camera");

		int i = cameraDumpNumber;

		util::TextFileModifier tfm;
		bool loadok = tfm.loadFile(impl->cameraFile);
		if (!loadok)
		{
			Logger::getInstance()->error("AniRecorder::removeCameraDump - File load failed.");
		}

		char rembuf[256];
		sprintf(rembuf, "<--- START-CAMERADUMP-%d --->", i);
		tfm.setStartSelectionNearMarker(rembuf);
		sprintf(rembuf, "<--- END-CAMERADUMP-%d --->", i);
		tfm.setEndSelectionNearMarker(rembuf);

		tfm.deleteSelection();

		tfm.saveFile();
		tfm.closeFile();
	}

	void AniRecorder::seekEndToTime(int ticks)
	{
		impl->currentEndPosition = ticks;
		if (impl->currentEndPosition < impl->currentPosition)
		{
			impl->currentEndPosition = impl->currentPosition;
		}
		if (impl->currentEndPosition > impl->longestAniLength)
		{
			impl->currentEndPosition = impl->longestAniLength;
		}
	}

	void AniRecorder::seekToTime(int ticks)
	{
		/*
		stop();

		{
			for (int i = 0; i < ANIRECORDER_MAX_ANIS; i++)
			{
				if (impl->anis[i] != NULL)
				{
					impl->anis[i]->startPlay();
					impl->anis[i]->leapToPosition(ticks);
				}
			}
		}
		AniManager::getInstance()->run();
		{
			for (int i = 0; i < ANIRECORDER_MAX_ANIS; i++)
			{
				if (impl->anis[i] != NULL)
				{
					impl->anis[i]->stopPlay();
				}
			}
		}

		impl->currentPosition = ticks;
		// ticks + 1?
		*/

		stop();
		impl->currentPosition = ticks;
		play();
		stop();
	}


	void AniRecorder::seekToMark(const char *mark)
	{
		stop();

		// TODO
		// umm... how to really do this, what about marks at different 
		// positions for different anis?
		/*
		for (int i = 0; i < ANIRECORDER_MAX_ANIS; i++)
		{
			if (impl->anis[i] != NULL)
			{
				impl->anis[i]->leapToMark(mark);
			}
		}
		AniManager::getInstance()->run();
		*/
	}


	int AniRecorder::getLongestAniLength()
	{
		return impl->longestAniLength;
	}


	int AniRecorder::getCurrentPosition()
	{
		return impl->currentPosition;
	}


	int AniRecorder::getCurrentEndPosition()
	{
		return impl->currentEndPosition;
	}


	int AniRecorder::dumpCameraPosition()
	{
		addUndoHistory("save camera");

		int camDumpNum = -1;

		bool fileExists = false;
		FILE *f = fopen(impl->cameraFile, "rb");
		if (f != NULL)
		{
			fileExists = true;
			fclose(f);
		}

		if (!fileExists)
		{
			// first create initial file...
			FILE *f = fopen(impl->cameraFile, "wb");
			if (f != NULL)
			{
				fprintf(f, "#!dhs -nopp\r\n\r\n");
				fprintf(f, "// automatically generated file, hand edit at your own risk.\r\n\r\n");
				fprintf(f, "script ani_camera_dump_%s\r\n", impl->recordIdString);
				fprintf(f, "// <--- CAMDUMP_ADD_MARKER --->\r\n// (keep your hands off this one)\r\n");
				fprintf(f, "endScript\r\n\r\n");
				fclose(f);
			} else {
				Logger::getInstance()->error("AniRecorder::dumpCameraPosition - Failed to open file for writing.");
				return -1;
			}
		}

		// now, write this dump to that file...
		util::TextFileModifier tfm;
		bool loadok = tfm.loadFile(impl->cameraFile);
		if (!loadok)
		{
			Logger::getInstance()->error("AniRecorder::dumpCameraPosition - File load failed.");
		}

		char subbuf[128];
		for (int i = 0; i < 99; i++)
		{
			// WARNING: assumes DOS linebreaks!
			sprintf(subbuf, "sub dumped_camera_position_%d\r", i);
			if (!tfm.setStartSelectionNearMarker(subbuf))
			{
				camDumpNum = i;
				break;
			}
		}

		tfm.setStartSelectionNearMarker("<--- CAMDUMP_ADD_MARKER --->");
		tfm.setEndSelectionNearMarker("<--- CAMDUMP_ADD_MARKER --->");

		char *camdumpbuf = new char[4096];

		ui::GameCamera *cam = impl->game->gameUI->getGameCamera();
		VC3 pos = cam->getPosition();

		camdumpbuf[0] = '\0';

		sprintf(&camdumpbuf[strlen(camdumpbuf)], "setCameraMode camera_centric\r\n");  
		//int intAngle = (int)cam->getAngleY();
		float floatAngle = cam->getAngleY();
		sprintf(&camdumpbuf[strlen(camdumpbuf)], "setCameraAngleFloat %f\r\n", floatAngle); 
		//int intBetaAngle = (int)cam->getBetaAngle();
		float floatBetaAngle = cam->getBetaAngle();
		sprintf(&camdumpbuf[strlen(camdumpbuf)], "setCameraBetaAngleFloat %f\r\n", floatBetaAngle); 
		sprintf(&camdumpbuf[strlen(camdumpbuf)], "setCameraFloatZoom %f\r\n", cam->getZoom()); 
		sprintf(&camdumpbuf[strlen(camdumpbuf)], "setPosition s,%f,%f\r\n", pos.x, pos.z); 
		sprintf(&camdumpbuf[strlen(camdumpbuf)], "setPositionHeight %f\r\n", pos.y); 
		sprintf(&camdumpbuf[strlen(camdumpbuf)], "setCameraPosition\r\n");
#ifdef PROJECT_SHADOWGROUNDS
		// nothing.. for backward compatibility don't save offsets...
#else
		VC3 posOffset = cam->getPositionOffset();
		VC3 targOffset = cam->getTargetOffset();
		sprintf(&camdumpbuf[strlen(camdumpbuf)], "setFloatValue %f;setCameraPositionOffsetXToFloatValue\r\n", posOffset.x); 
		sprintf(&camdumpbuf[strlen(camdumpbuf)], "setFloatValue %f;setCameraPositionOffsetYToFloatValue\r\n", posOffset.y); 
		sprintf(&camdumpbuf[strlen(camdumpbuf)], "setFloatValue %f;setCameraPositionOffsetZToFloatValue\r\n", posOffset.z); 
		sprintf(&camdumpbuf[strlen(camdumpbuf)], "setFloatValue %f;setCameraTargetOffsetXToFloatValue\r\n", targOffset.x); 
		sprintf(&camdumpbuf[strlen(camdumpbuf)], "setFloatValue %f;setCameraTargetOffsetYToFloatValue\r\n", targOffset.y); 
		sprintf(&camdumpbuf[strlen(camdumpbuf)], "setFloatValue %f;setCameraTargetOffsetZToFloatValue\r\n", targOffset.z); 
#endif

		// just to be sure, hopefully we don't have buffer overflows...
		assert(strlen(camdumpbuf) < 4096 * 3 / 4);

		char markbuf[128];
		sprintf(markbuf, "\r\n// <--- START-CAMERADUMP-%d --->\r\n\r\n", camDumpNum);
		tfm.addBeforeSelection(markbuf);
		strcat(subbuf, "\r\n");
		tfm.addBeforeSelection(subbuf);
		tfm.addBeforeSelection(camdumpbuf);
		tfm.addBeforeSelection("endSub\r\n");
		sprintf(markbuf, "\r\n// <--- END-CAMERADUMP-%d --->\r\n\r\n", camDumpNum);
		tfm.addBeforeSelection(markbuf);

		delete [] camdumpbuf;

		bool saveok = tfm.saveFile();
		if (!saveok)
		{
			Logger::getInstance()->error("AniRecorder::dumpCameraPosition - File save failed.");
		}

		return camDumpNum;
	}

	void AniRecorder::testCameraPosition(int cameraDumpNumber)
	{
		util::ScriptManager::getInstance()->loadScripts(impl->cameraFile, impl->recordDir, true);

		char subbuf[128];
		sprintf(subbuf, "dumped_camera_position_%d", cameraDumpNumber);
		char dumpscript[128];
		sprintf(dumpscript, "ani_camera_dump_%s", impl->recordIdString);
		impl->game->gameScripting->runMissionScript(dumpscript, subbuf);
	}

	void AniRecorder::interpolateCameraPosition(int cameraDumpNumber)
	{
		util::ScriptManager::getInstance()->loadScripts(impl->cameraFile, impl->recordDir, true);

		impl->game->gameUI->selectCamera(GAMEUI_CAMERA_CINEMATIC1);

		char subbuf[128];
		sprintf(subbuf, "dumped_camera_position_%d", cameraDumpNumber);
		char dumpscript[128];
		sprintf(dumpscript, "ani_camera_dump_%s", impl->recordIdString);
		impl->game->gameScripting->runMissionScript(dumpscript, subbuf);

		float interpTime = 5.0f;
		GameCamera *curcam = impl->game->gameUI->getGameCamera();
    impl->game->gameUI->selectCamera(GAMEUI_CAMERA_NORMAL);
    impl->game->gameUI->getGameCamera()->interpolateFrom(curcam, interpTime);
		impl->game->gameUI->getGameCamera()->doMovement(1);
	}

	LinkedList *AniRecorder::getCameraDumpList()
	{	
		Logger::getInstance()->debug("AniRecorder::getCameraDumpList - About to parse camera dump file for list.");

		LinkedList *ret = new LinkedList();

		util::TextFileModifier tfm;
		bool loadok = tfm.loadFile(impl->cameraFile);
		if (!loadok)
		{
			Logger::getInstance()->debug("AniRecorder::getCameraDumpList - File load failed.");
		} else {
			char subbuf[128];
			for (int i = 0; i < 99; i++)
			{
				// WARNING: assumes DOS linebreaks!
				sprintf(subbuf, "sub dumped_camera_position_%d\r", i);
				if (!tfm.setStartSelectionNearMarker(subbuf))
				{
					//break;
				} else {
					char *namebuf = new char[32];
					sprintf(namebuf, "%d", i);
					ret->append(namebuf);
				}
			}
		}

		return ret;
	}

	LinkedList *AniRecorder::getUnitList()
	{
		LinkedList *ret = new LinkedList();

		for (int i = 0; i < ANIRECORDER_MAX_ANIS; i++)
		{
			if (impl->anis[i] != NULL)
			{
				Unit *u = impl->anis[i]->getUnit();
				ret->append(u);
			}
		}

		return ret;
	}

	void AniRecorder::stop()
	{
		if (impl->recordingAni != NULL)
		{
			impl->recordingAni->stopRecord();
			AniManager::getInstance()->deleteAni(impl->recordingAni);
			impl->recordingAni = NULL;
			impl->needScriptReload = true;
		}
		for (int i = 0; i < ANIRECORDER_MAX_ANIS; i++)
		{
			if (impl->anis[i] != NULL)
			{
				if (!impl->anis[i]->hasPlayEnded())
				{
					impl->anis[i]->stopPlay();
				}
					
				Unit *u = impl->anis[i]->getUnit();
				u->setPath(NULL);
				u->setFinalDestination(u->getPosition());
				u->setWaypoint(u->getPosition());
			}
		}

		impl->recording = false;
		impl->playing = false;
	}

	void AniRecorder::rewind()
	{
		stop();

		impl->currentPosition = 0;

		play();

		/*
		for (int i = 0; i < ANIRECORDER_MAX_ANIS; i++)
		{
			if (impl->anis[i] != NULL)
			{
				impl->anis[i]->leapToPosition(0);
			}
		}
		*/
		
		AniManager::getInstance()->run();

		stop();
	}

	const char *AniRecorder::getCurrentAniId()
	{
		return impl->recordIdString;
	}


	void AniRecorder::dropOnGround(Unit *unit)
	{
		std::string undodesc = std::string("drop unit on ground");
		undodesc += std::string(" (") + this->getSliderPosOrRangeText() + ")";
		addUndoHistory(undodesc.c_str());

		assert(unit != NULL);

		int aninum = impl->getAniNumberForUnit(unit);

		if (aninum != -1)
		{
			char anifilebuf[512+32+1];
			if (strlen(impl->recordDir) < 256)
			{
				strcpy(anifilebuf, impl->recordDir);
				sprintf(&anifilebuf[strlen(anifilebuf)], "/ani_record_%d_%s.dhs", aninum, impl->recordIdString);

				AniTool anit = AniTool();
				if (anit.loadFile(anifilebuf))
				{
					anit.setSelectionStart(impl->currentPosition);
					anit.setSelectionEnd(impl->currentEndPosition);
					anit.dropMovementOnGround(impl->game->gameMap);
					anit.saveFile();
					anit.close();
				} else {
					Logger::getInstance()->error("AniRecorder::dropOnGround - Failed to load file.");
				}
			}
		}
		impl->needScriptReload = true;
	}

	void AniRecorder::deletePosition(Unit *unit)
	{
		std::string undodesc = std::string("delete position");
		undodesc += std::string(" (") + this->getSliderPosOrRangeText() + ")";
		addUndoHistory(undodesc.c_str());

		assert(unit != NULL);

		int aninum = impl->getAniNumberForUnit(unit);

		if (aninum != -1)
		{
			char anifilebuf[512+32+1];
			if (strlen(impl->recordDir) < 256)
			{
				strcpy(anifilebuf, impl->recordDir);
				sprintf(&anifilebuf[strlen(anifilebuf)], "/ani_record_%d_%s.dhs", aninum, impl->recordIdString);

				AniTool anit = AniTool();
				if (anit.loadFile(anifilebuf))
				{
					anit.setSelectionStart(impl->currentPosition);
					anit.setSelectionEnd(impl->currentEndPosition);
					anit.deleteSelection();
					anit.saveFile();
					anit.close();
				} else {
					Logger::getInstance()->error("AniRecorder::deletePosition - Failed to load file.");
				}
			}
		}
		impl->needScriptReload = true;
	}

	void AniRecorder::smoothPosition(Unit *unit, int smoothAmount)
	{
		std::string undodesc = std::string("smooth position");
		undodesc += std::string(" (") + this->getSliderPosOrRangeText() + ")";
		addUndoHistory(undodesc.c_str());

		assert(unit != NULL);
		assert(smoothAmount > 0);

		int aninum = impl->getAniNumberForUnit(unit);

		if (aninum != -1)
		{
			char anifilebuf[512+32+1];
			if (strlen(impl->recordDir) < 256)
			{
				strcpy(anifilebuf, impl->recordDir);
				sprintf(&anifilebuf[strlen(anifilebuf)], "/ani_record_%d_%s.dhs", aninum, impl->recordIdString);

				AniTool anit = AniTool();
				if (anit.loadFile(anifilebuf))
				{
					anit.setSelectionStart(impl->currentPosition);
					anit.setSelectionEnd(impl->currentEndPosition);
					anit.smoothMovement(smoothAmount);
					anit.saveFile();
					anit.close();
				} else {
					Logger::getInstance()->error("AniRecorder::smoothPosition - Failed to load file.");
				}
			}
		}
		impl->needScriptReload = true;
	}

	void AniRecorder::smoothRotation(Unit *unit, int smoothAmount)
	{
		std::string undodesc = std::string("smooth rotation");
		undodesc += std::string(" (") + this->getSliderPosOrRangeText() + ")";
		addUndoHistory(undodesc.c_str());

		assert(unit != NULL);
		assert(smoothAmount > 0);

		int aninum = impl->getAniNumberForUnit(unit);

		if (aninum != -1)
		{
			char anifilebuf[512+32+1];
			if (strlen(impl->recordDir) < 256)
			{
				strcpy(anifilebuf, impl->recordDir);
				sprintf(&anifilebuf[strlen(anifilebuf)], "/ani_record_%d_%s.dhs", aninum, impl->recordIdString);

				AniTool anit = AniTool();
				if (anit.loadFile(anifilebuf))
				{
					anit.setSelectionStart(impl->currentPosition);
					anit.setSelectionEnd(impl->currentEndPosition);
					anit.smoothRotation(smoothAmount);
					anit.saveFile();
					anit.close();
				} else {
					Logger::getInstance()->error("AniRecorder::smoothRotation - Failed to load file.");
				}
			}
		}
		impl->needScriptReload = true;
	}

	void AniRecorder::smoothAim(Unit *unit, int smoothAmount)
	{
		std::string undodesc = std::string("smooth aim");
		undodesc += std::string(" (") + this->getSliderPosOrRangeText() + ")";
		addUndoHistory(undodesc.c_str());

		assert(unit != NULL);
		assert(smoothAmount > 0);

		int aninum = impl->getAniNumberForUnit(unit);

		if (aninum != -1)
		{
			char anifilebuf[512+32+1];
			if (strlen(impl->recordDir) < 256)
			{
				strcpy(anifilebuf, impl->recordDir);
				sprintf(&anifilebuf[strlen(anifilebuf)], "/ani_record_%d_%s.dhs", aninum, impl->recordIdString);

				AniTool anit = AniTool();
				if (anit.loadFile(anifilebuf))
				{
					anit.setSelectionStart(impl->currentPosition);
					anit.setSelectionEnd(impl->currentEndPosition);
					anit.smoothAim(smoothAmount);
					anit.saveFile();
					anit.close();
				} else {
					Logger::getInstance()->error("AniRecorder::smoothAim - Failed to load file.");
				}
			}
		}
		impl->needScriptReload = true;
	}

	const char *AniRecorder::getUndoDesc()
	{
		int prevHistory = (impl->atHistory + ANIRECORDER_MAX_UNDOS - 1) % ANIRECORDER_MAX_UNDOS;
		return impl->undoHistoryDesc[prevHistory].c_str();
	}

	const char *AniRecorder::getRedoDesc()
	{
		//int nextHistory = (impl->atHistory + 1) % ANIRECORDER_MAX_UNDOS;
		return impl->undoHistoryDesc[impl->atHistory].c_str();
	}

	
	std::string AniRecorder::getSliderPosOrRangeText()
	{
		std::string tmp = "";
		if (getCurrentEndPosition() > getCurrentPosition())
		{
			tmp += int2str(getCurrentPosition());
			tmp += std::string(" - ");
			tmp += int2str(getCurrentEndPosition());
		} else {
			tmp += int2str(getCurrentPosition());
		}
		return tmp;
	}


}

