
#include "precompiled.h"

#include "GameVideoPlayer.h"

#include <string.h>
#include "../game/SimpleOptions.h"
#include "../game/options/options_video.h"
#include "../system/Logger.h"
#include "../system/Timer.h"

#define EXTERNAL_VIDEO_PLAYER_NAME "Data\\Executables\\videoplayer.exe"

namespace ui
{

	void GameVideoPlayer::playVideo(IStorm3D_Scene *scene, const char *filename, IStorm3D_StreamBuilder *streamBuilder)
	{
		if (game::SimpleOptions::getBool(DH_OPT_B_VIDEO_ENABLED))
		{
			Logger::getInstance()->debug("GameVideoPlayer::playVideo - About to play a video.");
			Logger::getInstance()->debug(filename);
			if (game::SimpleOptions::getBool(DH_OPT_B_EXTERNAL_VIDEO_PLAYER))
			{
				if (filename != NULL)
				{
					Logger::getInstance()->debug("GameVideoPlayer::playVideo - Using external video player.");
					char buf[256];
					strcpy(buf, EXTERNAL_VIDEO_PLAYER_NAME);
					strcat(buf, " ");
					strcat(buf, filename);
					Logger::getInstance()->debug(buf);
					system(buf);
				}
			} else {
				int time = Timer::getTime();
				scene->RenderVideo(filename, streamBuilder);
				int newTime = Timer::getTime();

				Timer::addTimeSub(newTime - time);
			}
		} else {
			Logger::getInstance()->debug("GameVideoPlayer::playVideo - Videos disabled, skipping video play.");
			Logger::getInstance()->debug(filename);
		}
	}

}
