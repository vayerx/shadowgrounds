
#include "precompiled.h"

#include "MusicPlaylist.h"

#include <assert.h>
#include <vector>
#include "SoundMixer.h"
#include "../system/SystemRandom.h"
#include "../system/Logger.h"
#include "../util/SimpleParser.h"

#include "../util/Debug_MemoryManager.h"

#define MUSICPLAYLIST_DEFAULT_FADETIME 3000

#define MUSICPLAYLIST_ALLOWED_TRACK_CHANGE_TIME_LIMIT 4000

namespace sfx {

MusicPlaylist::MusicPlaylist(SoundMixer *mixer)
{
  this->mixer = mixer;
  currentBank = 0;
  playing = false;
  suffle = false;
	musicFadeTime = MUSICPLAYLIST_DEFAULT_FADETIME;
  for (int i = 0; i < MUSICPLAYLIST_BANKS; i++)
  {
    atTrack[i] = 0;
    tracksInBank[i] = 0;
  }
  if (mixer != NULL)
  {
    mixer->setMusicLooping(false);
    Logger::getInstance()->debug("MusicPlaylist - Initialized with mixer.");
  } else {
    Logger::getInstance()->debug("MusicPlaylist - Initialized without mixer (no sounds).");
  }
}


MusicPlaylist::~MusicPlaylist()
{
  for (int i = 0; i < MUSICPLAYLIST_BANKS; i++)
  {
    for (int j = 0; j < (int)fileList[i].size(); j++)
    {
      delete [] fileList[i][j];
    }
    fileList[i].clear();
  }
}


void MusicPlaylist::play()
{

	static int musicplaylist_last_playing_track_msg_time = -MUSICPLAYLIST_ALLOWED_TRACK_CHANGE_TIME_LIMIT ;
	
	if (Timer::getTime() > musicplaylist_last_playing_track_msg_time + MUSICPLAYLIST_ALLOWED_TRACK_CHANGE_TIME_LIMIT) 
	{
		Logger::getInstance()->debug("MusicPlaylist::play - Playing track.");
		musicplaylist_last_playing_track_msg_time = Timer::getTime();
	}

  if (mixer != NULL)
  {
    //if (!playing)
    //{
			// NEW: if empty playlist, stop playing.
			if (tracksInBank[currentBank] == 0)
			{
				Logger::getInstance()->debug("MusicPlaylist::play - No tracks in current playlist, so stopping.");
				mixer->setMusic(NULL);
				playing = false;
			} else {
				if (atTrack[currentBank] < tracksInBank[currentBank])
				{
					assert((int)fileList[currentBank].size() == tracksInBank[currentBank]);
					char *filename = fileList[currentBank][atTrack[currentBank]];
					mixer->setMusic(filename, musicFadeTime);
				}
				playing = true;
			}
    //}
  }
}


void MusicPlaylist::playFile(char *filename)
{
  Logger::getInstance()->debug("MusicPlaylist::playFile - Playing file.");
  if (mixer != NULL)
  {
    if (!playing)
    {
      mixer->setMusic(filename, musicFadeTime);
      playing = true;
    }
  }
}


void MusicPlaylist::stop()
{
  Logger::getInstance()->debug("MusicPlaylist::stop - Music stopping.");
  if (mixer != NULL)
  {
    if (playing)
    {
      mixer->setMusic(NULL, musicFadeTime);
      playing = false;
    }
  }
}


void MusicPlaylist::nextTrack()
{
  if (suffle)
  {
    SystemRandom r;
    if (tracksInBank[currentBank] > 0)
      atTrack[currentBank] = r.nextInt() % 
        tracksInBank[currentBank];
    else
      atTrack[currentBank] = 0;
  } else {
    atTrack[currentBank]++;
    if (atTrack[currentBank] >= tracksInBank[currentBank])
      atTrack[currentBank] = 0;
  }
  play();
}


void MusicPlaylist::previousTrack()
{
  if (suffle)
  {
    nextTrack();  // oh well... ;)
  } else {
    atTrack[currentBank]--;
    if (atTrack[currentBank] < 0)
      atTrack[currentBank] = tracksInBank[currentBank] - 1;
    if (atTrack[currentBank] < 0)
      atTrack[currentBank] = 0;
  }
  play();
}


void MusicPlaylist::loadPlaylist(int bank, const char *filename)
{
  Logger::getInstance()->debug("MusicPlaylist::loadPlaylist - Loading playlist...");
  Logger::getInstance()->debug(filename);
  for (int i = 0; i < (int)fileList[bank].size(); i++)
  {
    delete [] fileList[bank][i];
  }
  fileList[bank].clear();

  util::SimpleParser parser;

  if (parser.loadFile(filename))
  {
    while (parser.next())
    {
      char *tmp = parser.getLine();
      if (tmp != NULL)
      {
        char *tmpcopy = new char[strlen(tmp) + 1];
        strcpy(tmpcopy, tmp);
        fileList[bank].push_back(tmpcopy); 
      }
    }
  } else {
    Logger::getInstance()->warning("MusicPlaylist::loadPlaylist - Failed to open playlist file.");
    Logger::getInstance()->debug(filename);
  }

  tracksInBank[bank] = fileList[bank].size();
}


void MusicPlaylist::setBank(int bank)
{
  currentBank = bank;
}


void MusicPlaylist::setSuffle(bool suffle)
{
  // TODO: proper suffle implementation.
  // (now we'll just rely on random track selection, which is not 
  // a real suffle, the way it should be.)
  this->suffle = suffle;
}



void MusicPlaylist::setMusicFadeTime(int musicFadeTime)
{
	this->musicFadeTime = musicFadeTime;
}


void MusicPlaylist::run()
{
  if (playing)
  {
    if (mixer != NULL
      && mixer->hasMusicEnded())
    {
			static int musicplaylist_last_track_changing_msg_time = -MUSICPLAYLIST_ALLOWED_TRACK_CHANGE_TIME_LIMIT;
			static bool musicplaylist_last_track_changing_msg_multi = false;
			
			if (Timer::getTime() > musicplaylist_last_track_changing_msg_time + MUSICPLAYLIST_ALLOWED_TRACK_CHANGE_TIME_LIMIT) 
			{
				Logger::getInstance()->debug("MusicPlaylist::run - Track changing.");
				musicplaylist_last_track_changing_msg_time = Timer::getTime();
				musicplaylist_last_track_changing_msg_multi = false;
			} else {
				if (!musicplaylist_last_track_changing_msg_multi)
				{
					Logger::getInstance()->warning("MusicPlaylist::run - Multiple track changes within time limit, possibly due to missing music track file.");
					musicplaylist_last_track_changing_msg_multi = true;
				}
			}
      nextTrack();
    }
  }
}

} // sfx

