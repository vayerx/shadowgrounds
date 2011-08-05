
#ifndef MUSICPLAYLIST_H
#define MUSICPLAYLIST_H

#include "SoundMixer.h"
#include <vector>

#include "playlistdefs.h"
#define MUSICPLAYLIST_BANKS PLAYLIST_AMOUNT

namespace sfx {

class MusicPlaylist
{
public:
  explicit MusicPlaylist(SoundMixer *mixer);
  ~MusicPlaylist();

  void play();
  void stop();
  void nextTrack();
  void previousTrack();
  void loadPlaylist(int bank, const char *filename);
  void setBank(int bank);
  void run();
  void setSuffle(bool suffle);
  void playFile(char *filename);
  void setMusicFadeTime(int musicFadeTime);

private:
  SoundMixer *mixer;
  int currentBank;
  int atTrack[MUSICPLAYLIST_BANKS];
  int tracksInBank[MUSICPLAYLIST_BANKS];
  std::vector <char *>fileList[MUSICPLAYLIST_BANKS];

  bool playing;
  bool suffle;

	int musicFadeTime;
};

} // sfx

#endif
