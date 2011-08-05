
#ifndef VISUALEFFECTMANAGER_H
#define VISUALEFFECTMANAGER_H

#include <DatatypeDef.h>

#ifndef INCLUDED_BOOST_SHARED_PTR_HPP
#define INCLUDED_BOOST_SHARED_PTR_HPP
#include <boost/shared_ptr.hpp>
#endif

class LinkedList;
class IStorm3D;
class IStorm3D_Scene;
class IStorm3D_Terrain;
class IStorm3D_Model;

namespace game
{
	class GameScene;
	class Game;
	class GameMap;
}

namespace util
{
	class ColorMap;
}

namespace frozenbyte
{
	namespace particle
	{
		class ParticleEffectManager;
		class IParticleEffect;
		class ParticlePhysics;
	}

	class DecalSystem;
}


namespace ui
{
	class IPointableObject;
	class VisualEffect;
	class LightManager;
	class ParticleCollision;
	class FluidParticleCollision;
	class ParticleArea;

	class VisualEffectManager
	{

		public:
			VisualEffectManager(IStorm3D *storm3d, IStorm3D_Scene *scene);

			~VisualEffectManager();

			/**
			 * Creates a new PARTLY UNMANAGED visual effect. Meaning that the caller must
			 * handle appropriate visual effect addReference/freeReference calls as well
			 * appropriate deleteVisualEffect call for the effect.
			 */
			VisualEffect *createNewVisualEffect(int visualEffectId,
				IPointableObject *object, IPointableObject *origin,
				const VC3 &position, const VC3 &endPosition, 
				const VC3 &rotation, const VC3 &velocity, game::Game *game,
				int muzzleflashBarrelNumber = 1, IStorm3D_Model *spawnModel = NULL);

			/**
			 * Creates a new FULLY MANAGED visual effect. Meaning that the
			 * caller should not permanently store the returned visualeffect 
			 * pointer. The VisualEffectManager will have full ownership
			 * of the effect and will handle destruction of the created effect.
			 */
			VisualEffect *createNewManagedVisualEffect(int visualEffectId,
				int lifetimeInTicks,
				IPointableObject *object, IPointableObject *origin,
				const VC3 &position, const VC3 &endPosition, 
				const VC3 &rotation, const VC3 &velocity, game::Game *game,
				int muzzleflashBarrelNumber = 1, IStorm3D_Model *spawnModel = NULL);

			void updateSpotlightPosition(VisualEffect *v,
				const VC3 &position, IPointableObject *origin, const VC3 &rotation);

			void deleteVisualEffect(VisualEffect *v);

			// had to change this to static for easy access from 
			// bullet code.
			static int getVisualEffectIdByName(const char *effectname);

			void loadParticleEffects();

			void freeParticleEffects();
			
			void loadDecalEffects();

			void freeDecalEffects();
			
			void run();

			void prepareForRender(game::GameMap *gameMap, util::ColorMap *colorMap, ui::LightManager *lightManager);

			void setTerrain(IStorm3D_Terrain *terrain);

			void setGameScene(game::GameScene *gameScene);
			void enableParticleInsideCheck(bool enable);

			void detachVisualEffectsFromUnits();

			// oh crap...
			frozenbyte::particle::ParticleEffectManager *getParticleEffectManager();

		private:
			LinkedList *visualEffects;

			IStorm3D *storm3d;
			IStorm3D_Scene *scene;
			IStorm3D_Terrain *terrain;

			game::GameScene *gameScene;

			frozenbyte::DecalSystem *decalSystem;

			//bool particleRunTwice;
			int particleRunCounter;

			//ParticleManager *particleManager;

			LinkedList *managedEffects;

			boost::shared_ptr<ParticleCollision> particleCollision;
			boost::shared_ptr<FluidParticleCollision> fluidParticleCollision;
			boost::shared_ptr<ParticleArea> particleArea;
			frozenbyte::particle::ParticleEffectManager* particleEffectManager;

			static void loadVisualEffectTypes();
			static void unloadVisualEffectTypes();

			void deleteManagedEffects();
			void updateManagedEffects();
	};
}

#endif


