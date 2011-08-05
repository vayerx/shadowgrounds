#ifndef GAMEVIDEOPLAYER_H
#define GAMEVIDEOPLAYER_H

#include <Storm3D_UI.h>
class IStorm3D_StreamBuilder;

namespace ui
{
	class GameVideoPlayer
	{
	public:
		static void playVideo(IStorm3D_Scene *scene, const char *filename, IStorm3D_StreamBuilder *streamBuilder);
	};

}
#endif // GAMEVIDEOPLAYER_H
