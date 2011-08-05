// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

#include <string>
#include <vector>

#include "storm3d_video_player.h"
#include "storm3d_videostreamer.h"
#include "storm3d.h"
#include "storm3d_scene.h"

struct Storm3D_VideoPlayerData
{
	Storm3D &storm;
	Storm3D_Scene &scene;
	boost::shared_ptr<Storm3D_VideoStreamer> streamer;

	Storm3D_VideoPlayerData(Storm3D &storm_, Storm3D_Scene &scene_, const char *filename, class IStorm3D_StreamBuilder *streamBuilder)
	:	storm(storm_),
		scene(scene_)
	{
		streamer.reset(static_cast<Storm3D_VideoStreamer *> (storm.CreateVideoStreamer(filename, streamBuilder, false)));
	}
};

Storm3D_VideoPlayer::Storm3D_VideoPlayer(Storm3D &storm, Storm3D_Scene &scene, const char *fileName, IStorm3D_StreamBuilder *streamBuilder)
{
	boost::scoped_ptr<Storm3D_VideoPlayerData> tempData(new Storm3D_VideoPlayerData(storm, scene, fileName, streamBuilder));
	data.swap(tempData);
}

Storm3D_VideoPlayer::~Storm3D_VideoPlayer()
{
}

void Storm3D_VideoPlayer::play()
{
	if(!data->streamer)
		return;

	while(!data->streamer->hasEnded())
	{
		Sleep(0);
		data->streamer->render(&data->scene);
		data->scene.RenderScene(true);

		MSG windowsMessage = { 0 };
		while(PeekMessage(&windowsMessage, 0, 0, 0, PM_REMOVE)) 
		{
			TranslateMessage(&windowsMessage);
			DispatchMessage(&windowsMessage);
		}
		/*
		MSG windowsMessage = { 0 };
		GetMessage(&windowsMessage, 0, 0, 0);
		{
			TranslateMessage(&windowsMessage);
			DispatchMessage(&windowsMessage);
		}
		*/

		if(GetKeyState(VK_ESCAPE) & 0x80)
			break;
		if(GetKeyState(VK_SPACE) & 0x80)
			break;
		if(GetKeyState(VK_RETURN) & 0x80)
			break;
	}
}
