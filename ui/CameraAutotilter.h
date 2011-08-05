
#ifndef CAMERAAUTOTILTER_H
#define CAMERAAUTOTILTER_H

#include <DatatypeDef.h>

namespace game
{
	class GameMap;
}

namespace ui
{
	class CameraAutotilter
	{
		public:
			CameraAutotilter(game::GameMap *gameMap);

			~CameraAutotilter();

			VC3 getTilt(const VC3 &position, float alphaAngle);

		private:
			game::GameMap *gameMap;
			VC3 lastRotation;
			VC3 lastPosition;
			int lastTime;

	};

}

#endif



