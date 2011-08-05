
#ifndef CAMERASCRIPTING_H
#define CAMERASCRIPTING_H

#include <string>

namespace util
{
	class ScriptProcess;
}

namespace game
{
	class CameraAreaParameters
	{
	public:
		CameraAreaParameters()
		{
			corner[0] = VC3(-1,0,1);
			corner[1] = VC3(1,0,1);
			corner[2] = VC3(1,0,-1);
			corner[3] = VC3(-1,0,-1);
			FOV = 90;
			angle = 0;
			betaAngle = 0;
			bank = 0;
			offset = VC3(0, 2, 0);
			follow = VC3(1, 1, 1);
			target = VC3(0, 0, 0);
			animation = "";
			name = "";
			group = 0;
			distance = 10;
			collision = 0;
			// script.c_str();
		}

		int type;
		VC3 corner[4];
		float FOV;
		float angle;
		float betaAngle;
		float bank;
		float distance;
		VC3 offset;
		VC3 follow;
		VC3 target;
		std::string animation;
		std::string name;
		int group;
		int collision;
/*		VC3 corner[4] = (VC3(-1,0,1), VC3(1,0,1), VC3(1,0,-1), VC3(-1,0,-1));
		float FOV = 90;
		float angle = 0;
		float betaAngle = 0;
		VC3 offset = VC3(0, 2, 0);
		VC3 follow = VC3(1, 1, 1);
		VC3 target = VC3(0, 0, 0);
		int group = 0;*/
	};

	class Game;
	class GameScriptData;

	class CameraScripting
	{
		public:			
			/** 
			 * Just processes one command...
			 */
			static void process(util::ScriptProcess *sp, 
				int command, floatint intFloat, char *stringData, ScriptLastValueType *lastValue,
				GameScriptData *gsd, Game *game);
//			static void initializeCameraAreaParameters();
		private:
			static CameraAreaParameters cameraAreaParameters;
	};
}

#endif



