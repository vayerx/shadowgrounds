
#ifndef OPTIONAPPLIER_H
#define OPTIONAPPLIER_H

class IStorm3D;
class IStorm3D_Scene;
class IStorm3D_Terrain;

class Ogui;

namespace ui
{
	class LightManager;
}

namespace sfx
{
	class SoundMixer;
}

namespace game
{
	class Game;
	class GameOptionManager;

	class OptionApplier
	{
		public:
			static void applyOptions(Game *game, GameOptionManager *oman, Ogui *ogui);

			// (gamma options apply included in display options apply)
			static void applyGammaOptions(IStorm3D *s3d);

			static void applyDisplayOptions(IStorm3D *s3d, IStorm3D_Scene *scene, IStorm3D_Terrain *terrain,
				ui::LightManager *lightManager);

			static void applyLoggerOptions();

			static void applySoundOptions(sfx::SoundMixer *soundMixer);

			static void applyCameraOptions(IStorm3D_Scene *scene);


			// static bool doOptionsNeedApplying();
	};

}


#endif

