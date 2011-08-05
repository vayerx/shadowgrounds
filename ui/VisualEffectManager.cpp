#include <fstream>

#include "precompiled.h"

#include "VisualEffectManager.h"

#include <Storm3D_UI.h>

#include <assert.h>
#include "VisualEffect.h"
#include "VisualEffectType.h"
#include "VisualObjectModel.h"
#include "ParticleManager.h"
#include "../game/gamedefs.h"
#include "../ui/Spotlight.h"
#include "../ui/LightManager.h"
#include "../ui/DynamicLightManager.h"
#include "../ui/ParticleCollision.h"
#include "../ui/ParticleArea.h"
#include "../container/LinkedList.h"
#include "../convert/str2int.h"
#include "../util/SimpleParser.h"
#include "../system/Logger.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_effects.h"
#include "../game/options/options_physics.h"
#include "../game/physics/GamePhysics.h"
#include "../system/SystemRandom.h"
#include "Muzzleflasher.h"
#include "Ejecter.h"
#include "../game/scaledefs.h"
#include "../util/DecalSystem.h"
#include "../util/DecalManager.h"
#include "../physics/physics_lib.h"

#include "../game/Game.h"
#include "../game/GameMap.h"
#include "../game/GameScene.h"
#include "../util/ColorMap.h"
#include "../game/UnitList.h"
#include "../game/Unit.h"

#include "../game/GameUI.h"

#include <string>
#include <vector>
#include <list>

#include "../editor/parser.h"
#include "../filesystem/input_stream.h"
#include "../filesystem/file_package_manager.h"
#include "../particle_editor2/particleeffect.h"
#include "../particle_editor2/ParticlePhysics.h"

#include "../filesystem/ifile_list.h"
#include "igios.h"

#define MAX_VISUAL_EFFECT_TYPES 384

using namespace frozenbyte;
using namespace frozenbyte::particle;

namespace ui
{
	extern int visual_effect_allocations;

	class ManagedVisualEffectEntry 
	{
	public:
		ManagedVisualEffectEntry(VisualEffect *visualEffect, int lifetimeInTicks)
		{
			this->lifetimeInTicks = lifetimeInTicks;
			this->visualEffect = visualEffect;
		}
		int lifetimeInTicks;
		VisualEffect *visualEffect;
	};

	// static storage for visual effect filenames
	// TODO: cleanup method.
	VisualEffectType *visualEffectTypes = NULL;

	VisualEffectManager::VisualEffectManager(IStorm3D *storm3d, IStorm3D_Scene *scene)
	{
		visualEffects = new LinkedList();
		//particleManager = new ParticleManager(storm3d,
		//	scene->GetParticleSystem());

		managedEffects = new LinkedList();

		particleEffectManager = new ParticleEffectManager(storm3d, scene);
		if(game::SimpleOptions::getBool(DH_OPT_B_PHYSICS_ENABLED))
			particleEffectManager->enablePhysics(true);

		this->decalSystem = NULL;

		this->storm3d = storm3d;
		this->scene = scene;
		this->terrain = NULL;
		this->gameScene = NULL;

		//this->particleRunTwice = false;
		particleRunCounter = 0;

		if (visualEffectTypes == NULL)
			loadVisualEffectTypes();		
	}


	VisualEffectManager::~VisualEffectManager()
	{
		deleteManagedEffects();
		delete managedEffects;

		// just to be sure...
		freeDecalEffects();

		while (!visualEffects->isEmpty())
		{
			VisualEffect *v = (VisualEffect *)visualEffects->popLast();
			v->freeReference();
		}
		delete visualEffects;
		delete particleEffectManager;

		assert(visual_effect_allocations == 0);

		// needed if storm generation changes...
		unloadVisualEffectTypes();
	}



	void VisualEffectManager::deleteManagedEffects()
	{
		while (!managedEffects->isEmpty())
		{
			ManagedVisualEffectEntry *entry = (ManagedVisualEffectEntry*)managedEffects->popLast();

			VisualEffect *vef = entry->visualEffect;
			this->deleteVisualEffect(vef);
			vef->freeReference();

			delete entry;
		}
	}


	void VisualEffectManager::updateManagedEffects()
	{
		// optimization hack, no need to use safelinkedlistiterator...
		LinkedList entryDeleteList;

		LinkedListIterator iter(managedEffects);
		while(iter.iterateAvailable())
		{
			ManagedVisualEffectEntry *entry = (ManagedVisualEffectEntry*)iter.iterateNext();
			entry->lifetimeInTicks--;
			if (entry->lifetimeInTicks <= 0)
			{
				VisualEffect *vef = entry->visualEffect;
				this->deleteVisualEffect(vef);
				vef->freeReference();

				entryDeleteList.append(entry);
			}
		}

		while (!entryDeleteList.isEmpty())
		{
			ManagedVisualEffectEntry *entry = (ManagedVisualEffectEntry*)entryDeleteList.popLast();
			managedEffects->remove(entry);
			delete entry;
		}

	}



	void VisualEffectManager::setTerrain(IStorm3D_Terrain *terrain)
	{
		// spotlights depend on terrain.. make sure they get deleted...
		// (call setTerrain with NULL param before deleting the terrain!)
		while (!visualEffects->isEmpty())
		{
			VisualEffect *v = (VisualEffect *)visualEffects->popLast();
			v->freeReference();
		}
		this->terrain = terrain;
	}


	void VisualEffectManager::detachVisualEffectsFromUnits()
	{
		LinkedListIterator iter = LinkedListIterator(visualEffects);
		while (iter.iterateAvailable())
		{
			VisualEffect *v = (VisualEffect *)iter.iterateNext();
			if (v->follow != NULL)
			{
				v->follow = NULL;
			}
		}
	}


	void VisualEffectManager::unloadVisualEffectTypes()
	{
		delete [] visualEffectTypes;
		visualEffectTypes = NULL;
	}


	void VisualEffectManager::loadVisualEffectTypes()
	{
		// NOTE: not thread safe.

		// read in visual effects
		visualEffectTypes = new VisualEffectType[MAX_VISUAL_EFFECT_TYPES];
		util::SimpleParser sp;
		int atId = 0;
#ifdef LEGACY_FILES
		if (sp.loadFile("Data/Effects/visualeffects.txt"))
		{
#else
		// read from multiple files, scan directory/subdirectories, then read in all those files...

		// read in files...
		boost::shared_ptr<filesystem::IFileList> files = filesystem::FilePackageManager::getInstance().findFiles("data/effect/visualeffect", "*.vef");
		std::vector<std::string> allFiles;
		filesystem::getAllFiles(*files, "data/effect/visualeffect", allFiles, true);

		for (int vef = 0; vef < (int)allFiles.size(); vef++)
		{
			Logger::getInstance()->debug("VisualEffectManager - About to load a visual effect file.");
			Logger::getInstance()->debug(allFiles[vef].c_str());
			// for each found file,
			bool loadOk = sp.loadFile(allFiles[vef].c_str());
			if (!loadOk)
			{
				continue;
				Logger::getInstance()->error("VisualEffectManager - Failed to load an effect file.");
			}

#endif
			bool insideEffect = false;
			while (sp.next())
			{
				bool lineok = false;
				char *k = sp.getKey();
				if (k != NULL)
				{
					if (!insideEffect)
						sp.error("VisualEffectManager - Parse error, key=value pair outside visualeffect block.");
					char *v = sp.getValue();
					// treat empty lines as null...
					if (v[0] == '\0') v = NULL;

					if (strcmp(k, "name") == 0)
					{
						if (v != NULL)
						{
							for (int i = 0; i < atId; i++)
							{
								if (visualEffectTypes[i].getName() != NULL
								  && strcmp(v, visualEffectTypes[i].getName()) == 0)
								{
									sp.error("VisualEffectManager - Duplicate visual effect name.");
									break;
								}
							}
						}
						visualEffectTypes[atId].setName(v);
						lineok = true;
					}
					if (strcmp(k, "model") == 0)
					{
						visualEffectTypes[atId].setModelFilename(v);
						lineok = true;
					}
					if (strcmp(k, "lighteffect") == 0)
					{
						visualEffectTypes[atId].setSpotLightEffect(v);
						lineok = true;
#ifndef PROJECT_SHADOWGROUNDS
						Logger::getInstance()->warning("VisualEffectManager::loadVisualEffectTypes - \"lighteffect\" key deprecated (use \"spotlight\" instead).");
#endif
					}
					if (strcmp(k, "spotlight") == 0)
					{
						visualEffectTypes[atId].setSpotLightEffect(v);
						lineok = true;
					}
					if (strcmp(k, "pointlight") == 0)
					{
						visualEffectTypes[atId].setPointLightEffect(v);
						lineok = true;
					}
					if (strcmp(k, "fadeout") == 0)
					{
						if (v != NULL && v[0] == '1')
							visualEffectTypes[atId].setFadeout(true);
						lineok = true;
					}
					if (strcmp(k, "particleeffect") == 0)
					{
						visualEffectTypes[atId].setParticleEffect(v);
						lineok = true;
					}
					if (strcmp(k, "hardware_fluid_particleeffect") == 0)
					{
						visualEffectTypes[atId].setParticleEffectHardwareFluid(v);
						lineok = true;
					}
					if (strcmp(k, "hardware_rigid_particleeffect") == 0)
					{
						visualEffectTypes[atId].setParticleEffectHardwareRigid(v);
						lineok = true;
					}
					if (strcmp(k, "decaleffect") == 0)
					{
						visualEffectTypes[atId].setDecalEffect(v);
						lineok = true;
					}
					if (strcmp(k, "decalamount") == 0)
					{
						if (v != NULL)
						{
							visualEffectTypes[atId].setDecalAmount(str2int(v));
						} else {
							visualEffectTypes[atId].setDecalAmount(0);
						}
						lineok = true;
					}
					if (strcmp(k, "decalamountvariation") == 0)
					{
						if (v != NULL)
						{
							visualEffectTypes[atId].setDecalAmountVariation(str2int(v));
						} else {
							visualEffectTypes[atId].setDecalAmountVariation(0);
						}
						lineok = true;
					}
					if (strcmp(k, "decalpositionrandom") == 0)
					{
						if (v != NULL)
						{
							visualEffectTypes[atId].setDecalPositionRandom(str2int(v));
						} else {
							visualEffectTypes[atId].setDecalPositionRandom(0);
						}
						lineok = true;
					}
					if (strcmp(k, "decalautoremove") == 0)
					{
						if (v != NULL && str2int(v))
						{
							visualEffectTypes[atId].setDecalAutoRemove(true);
						} else {
							visualEffectTypes[atId].setDecalAutoRemove(false);
						}
						lineok = true;
					}
					if (strcmp(k, "decalpositioning") == 0)
					{
						if (v != NULL)
						{
							if (strcmp(v, "velocity") == 0)
							{
								visualEffectTypes[atId].setDecalPositioning(
									DecalPositionCalculator::DECAL_POSITIONING_VELOCITY);
								lineok = true;
							}
							if (strcmp(v, "origin") == 0)
							{
								visualEffectTypes[atId].setDecalPositioning(
									DecalPositionCalculator::DECAL_POSITIONING_ORIGIN);
								lineok = true;
							}
							if (strcmp(v, "downward") == 0)
							{
								visualEffectTypes[atId].setDecalPositioning(
									DecalPositionCalculator::DECAL_POSITIONING_DOWNWARD);
								lineok = true;
							}
						} else {
							lineok = true;
						}
					}
					if (strcmp(k, "camerashakeamount") == 0)
					{
						if (v != NULL)
						{
							visualEffectTypes[atId].setCameraShakeAmount(str2int(v));
						} else {
							visualEffectTypes[atId].setCameraShakeAmount(0);
						}
						lineok = true;
					}
					if (strcmp(k, "attachtospawnmodel") == 0)
					{
						if (v != NULL)
						{
							if (str2int(v) != 0)
								visualEffectTypes[atId].setAttachToSpawnModel(true);
							else
								visualEffectTypes[atId].setAttachToSpawnModel(false);
						} else {
							visualEffectTypes[atId].setAttachToSpawnModel(false);
						}
						lineok = true;
					}
					if (strcmp(k, "follow") == 0)
					{
						if (v != NULL)
						{
							if (strcmp(v, "none") == 0)
							{
								visualEffectTypes[atId].setFollow(
									VisualEffectType::VISUALEFFECT_FOLLOW_NONE);
								lineok = true;
							}
							if (strcmp(v, "origin") == 0)
							{
								visualEffectTypes[atId].setFollow(
									VisualEffectType::VISUALEFFECT_FOLLOW_ORIGIN);
								lineok = true;
							}
							if (strcmp(v, "object") == 0)
							{
								visualEffectTypes[atId].setFollow(
									VisualEffectType::VISUALEFFECT_FOLLOW_OBJECT);
								lineok = true;
							}
						}
					}
					if (strcmp(k, "type") == 0)
					{
						if (v != NULL)
						{
							if (strcmp(v, "normal") == 0)
							{
								visualEffectTypes[atId].setType(
									VisualEffectType::VISUALEFFECT_TYPE_NORMAL);
								lineok = true;
							}
							if (strcmp(v, "ray") == 0)
							{
								visualEffectTypes[atId].setType(
									VisualEffectType::VISUALEFFECT_TYPE_RAY);
								lineok = true;
							}
							if (strcmp(v, "muzzleflash") == 0)
							{
								visualEffectTypes[atId].setType(
									VisualEffectType::VISUALEFFECT_TYPE_MUZZLEFLASH);
								lineok = true;
							}
							if (strcmp(v, "eject") == 0)
							{
								visualEffectTypes[atId].setType(
									VisualEffectType::VISUALEFFECT_TYPE_EJECT);
								lineok = true;
							}
						}
					}
					if (strcmp(k, "modeleffect") == 0)
					{
						if (v != NULL)
						{
							if (strcmp(v, "none") == 0)
							{
								lineok = true;
							}
							if (strcmp(v, "additive") == 0)
							{
								visualEffectTypes[atId].setModelEffect(
									VISUALOBJECTMODEL_EFFECT_ADDITIVE);
								lineok = true;
							}
						}
					}
					
				} else {
					char *l = sp.getLine();
					if (strcmp(l, "visualeffect") == 0)
					{
						if (insideEffect)
							sp.error("VisualEffectManager - Parse error, } expected.");
						if (atId < MAX_VISUAL_EFFECT_TYPES - 1)
						{
							atId++;
						} else {
							sp.error("VisualEffectManager - Too many effects, limit reached.");
						}
						lineok = true;
					}
					if (strcmp(l, "{") == 0)
					{
						if (insideEffect)
							sp.error("VisualEffectManager - Parse error, unexpected {.");
						insideEffect = true;
						lineok = true;
					}
					if (strcmp(l, "}") == 0)
					{
						if (!insideEffect)
							sp.error("VisualEffectManager - Parse error, unexpected }.");
						insideEffect = false;
						lineok = true;
					}
				}
				if (!lineok)
				{
					sp.error("VisualEffectManager - Unknown command or bad key/value pair.");
				}
			}				
#ifdef LEGACY_FILES
		}	else {
			Logger::getInstance()->error("VisualEffectManager - Failed to load effects.");
#endif
		}
	}


	VisualEffect *VisualEffectManager::createNewManagedVisualEffect(int visualEffectId,
		int lifetimeInTicks,
		IPointableObject *object, IPointableObject *origin,
		const VC3 &position, const VC3 &endPosition, const VC3 &rotation,
		const VC3 &velocity, game::Game *game, int muzzleflashBarrelNumber,
		IStorm3D_Model *spawnModel)
	{
		VisualEffect *vef = this->createNewVisualEffect(visualEffectId, object, origin, position, endPosition,
			rotation, velocity, game, muzzleflashBarrelNumber, spawnModel);

		if (vef != NULL)
		{
			vef->addReference();

			ManagedVisualEffectEntry *entry = new ManagedVisualEffectEntry(vef, lifetimeInTicks);

			this->managedEffects->append(entry);
		}

		return vef;
	}


	VisualEffect *VisualEffectManager::createNewVisualEffect(int visualEffectId,
		IPointableObject *object, IPointableObject *origin,
		const VC3 &position, const VC3 &endPosition, const VC3 &rotation,
		const VC3 &velocity, game::Game *game, int muzzleflashBarrelNumber,
		IStorm3D_Model *spawnModel)
	{
		VisualEffect *v = NULL;
		if (visualEffectId >= 0 && visualEffectId < MAX_VISUAL_EFFECT_TYPES)
		{
			v = new VisualEffect(&visualEffectTypes[visualEffectId],
				object, origin, position, endPosition, rotation, muzzleflashBarrelNumber);
			v->addReference();

			visualEffects->append(v);

			// effect_types :
			//		explosion			// explosions, smoke (position)
			//		ammo				// flame_thrower, (start, end) 
			//		environment			// rain, snow
			
			// spawn new particle effect?
			int particleEffectID = visualEffectTypes[visualEffectId].getParticleEffectID();

			// if effect has a hardware rigid particle id and option enabled, used that instead
			int rigidID = visualEffectTypes[visualEffectId].getParticleEffectHardwareRigidID();
			if (rigidID != -1 && game::SimpleOptions::getBool(DH_OPT_B_PHYSICS_PARTICLES))
			{
				particleEffectID = rigidID;
			}
			// if effect has a hardware fluid particle id and option enabled, used that instead
			int fluidID = visualEffectTypes[visualEffectId].getParticleEffectHardwareFluidID();
			if (fluidID != -1 && game::SimpleOptions::getBool(DH_OPT_B_PHYSICS_FLUIDS_ENABLED))
			{
#if defined(PHYSICS_PHYSX) && !defined(NX_DISABLE_FLUIDS)
				int fluidParticlesAmount = game->getGamePhysics()->getPhysicsLib()->getActiveFluidParticleAmount();
				//if (game->getGamePhysics()->getPhysicsLib()->isRunningInHardware()
				//	&& fluidParticlesAmount < game::SimpleOptions::getInt(DH_OPT_I_PHYSICS_MAX_FLUID_PARTICLES))
				if (fluidParticlesAmount < game::SimpleOptions::getInt(DH_OPT_I_PHYSICS_MAX_FLUID_PARTICLES))
#else
				if (true)
#endif
				{
					particleEffectID = fluidID;
				}
			}

			if(particleEffectID != -1) 
			{				
#ifdef PHYSICS_PHYSX
				if(game::SimpleOptions::getBool(DH_OPT_B_PHYSICS_PARTICLES) && game->getGamePhysics() && game->getGamePhysics()->getPhysicsLib())
				{
					boost::shared_ptr<physics::PhysicsLib> ptr(game->getGamePhysics()->getPhysicsLib(), NullDeleter());
					particleEffectManager->setPhysics(ptr);
				}

				if(game::SimpleOptions::getBool(DH_OPT_B_PHYSICS_PARTICLES))
					particleEffectManager->enableParticlePhysics(true);
				else
					particleEffectManager->enableParticlePhysics(false);
#endif

				v->setParticleEffect(particleEffectManager->addEffectToScene(particleEffectID), 
					velocity, rotation);

				if(v->particleEffect)
				{
					boost::shared_ptr<frozenbyte::particle::IParticleCollision> ptr = boost::static_pointer_cast<frozenbyte::particle::IParticleCollision> (particleCollision);
					//boost::shared_ptr<frozenbyte::particle::IParticleCollision> ptr = boost::static_pointer_cast<frozenbyte::particle::IParticleCollision> (fluidParticleCollision);
					boost::shared_ptr<frozenbyte::particle::IParticleArea> area = boost::static_pointer_cast<frozenbyte::particle::IParticleArea> (particleArea);
					v->particleEffect->setCollision(ptr);
					v->particleEffect->setArea(area);
					if (visualEffectTypes[visualEffectId].doesAttachToSpawnModel())
					{
						if (spawnModel != NULL)
						{
							v->particleEffect->setSpawnModel(spawnModel);
						} else {
							LOG_WARNING_W_DEBUG("VisualEffectManager::createNewVisualEffect - Effect should be attached to spawn model, but null spawn model parameter was given.", visualEffectTypes[visualEffectId].getName());
						}
					}


					// Apply lighting
					{
						ui::PointLights lights;
						VC3 position = v->position;
						position.x = position.x / game->gameMap->getScaledSizeX() + .5f;
						position.z = position.z / game->gameMap->getScaledSizeY() + .5f;

						ui::LightManager *lightManager = game->gameUI->getLightManager();

						int ox = game->gameMap->scaledToObstacleX(position.x);
						int oy = game->gameMap->scaledToObstacleY(position.z);
						if (game->gameMap->isWellInScaledBoundaries(position.x, position.z))
						{
							if(game->gameMap->getAreaMap()->isAreaAnyValue(ox, oy, AREAMASK_INBUILDING))
								lightManager->getLighting(v->position, lights, 1.f, true, true);
							else
								lightManager->getLighting(v->position, lights, 1.f, true, false);
						}

						COL ambient = lightManager->getApproximatedLightingForIndices(v->position, lights);
						v->particleEffect->setLighting(ambient, lights.lightIndices);
					}

				}
			}

			// shake camera?
			if (visualEffectTypes[visualEffectId].getCameraShakeAmount() > 0)
			{
				// HACK: hard coded shake time here.
				int shakeTime = 500;
				game->gameUI->getGameCamera()->setShakeEffect(visualEffectTypes[visualEffectId].getCameraShakeAmount(), shakeTime, position);
			}
			
			// spawn new decals?
			int decalEffectID = visualEffectTypes[visualEffectId].getDecalEffectID();
			int decalAmount = visualEffectTypes[visualEffectId].getDecalAmount();
			if(decalEffectID != -1 && decalAmount > 0) 
			{
				DecalPositionCalculator::DECAL_POSITIONING decalPosType
					= visualEffectTypes[visualEffectId].getDecalPositioning();
				int decalPosRandom
					= visualEffectTypes[visualEffectId].getDecalPositionRandom();

				assert(decalSystem != NULL);

				decalSystem->setEraseProperties(game::SimpleOptions::getInt(DH_OPT_I_DECAL_FADE_TIME));	
				decalSystem->setMaxDecalAmount(game::SimpleOptions::getInt(DH_OPT_I_DECAL_MAX_AMOUNT));	

				int amountVariation = visualEffectTypes[visualEffectId].getDecalAmountVariation();
				int randomVariation = (SystemRandom::getInstance()->nextInt() % (amountVariation * 2 + 1) - amountVariation);
				decalAmount += randomVariation;

				LinkedList *ulist = game->units->getAllUnits();
				LinkedListIterator uiter(ulist);

				while (uiter.iterateAvailable())
				{
					game::Unit *u = (game::Unit *)uiter.iterateNext();
					if (u->getVisualObject() != NULL)
					{
						u->getVisualObject()->setCollidable(false);
					}
				}

				for (int i = 0; i < decalAmount; i++)
				{
					VC3 decalPos;
					QUAT decalRot;
					bool decalPosOk = DecalPositionCalculator::calculateDecalPosition(
						gameScene, position, velocity, decalPosType, decalPosRandom, &decalPos, &decalRot);

					if (decalPosOk)
					{
						frozenbyte::DecalIdentifier id;

						COL light = game->gameMap->colorMap->getColorAtScaled(decalPos.x, decalPos.z);

						// Apply lighting
						{
							ui::PointLights lights;

							ui::LightManager *lightManager = game->gameUI->getLightManager();
							lightManager->getLighting(v->position, lights, 1.f, true, false);

							light = lightManager->getApproximatedLightingForIndices(v->position, lights);
							light *= 0.5f;
						}

						int x = game->gameMap->scaledToObstacleX(decalPos.x);
						int y = game->gameMap->scaledToObstacleY(decalPos.z);
						bool inBuilding = game->gameMap->getAreaMap()->isAreaAnyValue(x, y, AREAMASK_INBUILDING);
						decalSystem->spawnDecal(id, decalEffectID, decalPos, decalRot, light, inBuilding);

						if(v->effectType && v->effectType->getDecalAutoRemove())
							v->addRemovableDecal(id);
					}
				}

				uiter = LinkedListIterator(ulist);

				while (uiter.iterateAvailable())
				{
					game::Unit *u = (game::Unit *)uiter.iterateNext();
					if (u->getVisualObject() != NULL)
					{
						u->getVisualObject()->setCollidable(true);
					}
				}

			}

			// light effects? (spot, point)
			if (visualEffectTypes[visualEffectId].getSpotLightEffect() != NULL)
			{
				assert(terrain != NULL);
				if (terrain != NULL)
				{
					ui::Spotlight *sp = new ui::Spotlight(*storm3d, *terrain,
						*scene, NULL,
						std::string(visualEffectTypes[visualEffectId].getSpotLightEffect()));
					v->setSpotlight(sp);

					updateSpotlightPosition(v, position, origin, rotation);
				}
			}
			if (visualEffectTypes[visualEffectId].getPointLightEffect() != NULL)
			{
				assert(game->gameUI->getDynamicLightManager() != NULL);
				if (game->gameUI->getDynamicLightManager() != NULL)
				{
					DynamicLightManager *dlman = game->gameUI->getDynamicLightManager();
					int dLightType = dlman->getLightType(visualEffectTypes[visualEffectId].getPointLightEffect());
					if (dLightType != -1)
					{
						int dLightInstance = dlman->addLightInstance(dLightType, position, 1.0f, 1.0f);
						UnifiedHandle pointl = dlman->getUnifiedHandle(dLightType, dLightInstance);
						v->setPointlight(pointl, game->gameUI->getDynamicLightManager());
					} else {
						LOG_ERROR_W_DEBUG("VisualEffectManager::loadVisualEffectTypes - Given dynamic point light type not defined.", visualEffectTypes[visualEffectId].getPointLightEffect());
					}
				}
			}

			
		} else {
			assert(!"visual effect id out of range.");
		}
		return v;
	}


	void VisualEffectManager::updateSpotlightPosition(VisualEffect *v, 
		const VC3 &position, IPointableObject *origin, const VC3 &rotation)
	{
		assert(v != NULL);

		const char *lightEffect = v->effectType->getSpotLightEffect();

		if (lightEffect == NULL)
			return;

		Spotlight *sp = v->spotlight;

		if (sp == NULL)
			return;

		// HACK: if it's electricflash...
		if (strncmp(lightEffect, "electricflash", 13) == 0)
		{
			VC3 sppos = origin->getPointerPosition();
			sppos.y += 2.8f;
			float angle = UNIT_ANGLE_TO_RAD(rotation.y);
			VC3 dir = VC3(sinf(angle), -6.00f, cosf(angle));
			dir.Normalize();
			sppos.x -= dir.x * 12.5f;
			sppos.z -= dir.z * 12.5f;
			sp->setPosition(sppos);

			sppos = origin->getPointerPosition();
			sppos.x -= dir.x * 12.f;
			sppos.z -= dir.z * 12.f;
			sp->setFakePosition(sppos);
			sp->setDirection(dir);
		}
		// HACK: if it's flamerflash...
		else if (strncmp(lightEffect, "flamerflash", 11) == 0)
		{
			VC3 sppos = origin->getPointerPosition();
			sppos.y += 2.8f;
			float angle = UNIT_ANGLE_TO_RAD(rotation.y);
			VC3 dir = VC3(sinf(angle), -9.00f, cosf(angle));
			dir.Normalize();
			sppos.x -= dir.x * 25.5f;
			sppos.z -= dir.z * 25.5f;
			sp->setPosition(sppos);

			sppos = origin->getPointerPosition();
			sppos.x -= dir.x * 25.f;
			sppos.z -= dir.z * 25.f;
			sp->setFakePosition(sppos);
			sp->setDirection(dir);
		}
		// HACK: if it's muzzleflash...
		else if (strncmp(lightEffect, "muzzleflash", 11) == 0)
		{
			VC3 sppos = origin->getPointerPosition();
			sppos.y += 2.8f;
			float angle = UNIT_ANGLE_TO_RAD(rotation.y);
			VC3 dir = VC3(sinf(angle), -6.00f, cosf(angle));
			dir.Normalize();
			sppos.x -= dir.x * 9.5f;
			sppos.z -= dir.z * 9.5f;
			sp->setPosition(sppos);
			
			sppos = origin->getPointerPosition();
			sppos.x -= dir.x * 9.f;
			sppos.z -= dir.z * 9.f;
			sp->setFakePosition(sppos);
			sp->setDirection(dir);
		} else {
			VC3 sppos = position;
			sppos.y += 2.8f;
			// HACK: if it's electric_flow... / flamethrower
			if (strncmp(lightEffect, "electric_flow", 13) == 0
				|| strncmp(lightEffect, "flamethrower", 12) == 0)
			{
				sppos.y -= 2.0f;
			}
			sp->setPosition(sppos);
			float angle = UNIT_ANGLE_TO_RAD(rotation.y);
			//float angle = 
			//	3.1415f * (float)(SystemRandom::getInstance()->nextInt() & SYSTEMRANDOM_MAX_VALUE)
			//	/ (float)(SYSTEMRANDOM_MAX_VALUE+1);
			VC3 dir = VC3(sinf(angle), -0.05f, cosf(angle));
			dir.Normalize();
			sp->setDirection(dir);
		}
	}


	void VisualEffectManager::deleteVisualEffect(VisualEffect *v)
	{
	  v->setDeleteFlag();
	}


	int VisualEffectManager::getVisualEffectIdByName(const char *effectname)
	{
		assert(effectname != NULL);

		if (visualEffectTypes == NULL)
			loadVisualEffectTypes();		

		// TODO: this is not very effective, but on the other hand
		// this is not meant to be called very often.
		for (int i = 0; i < MAX_VISUAL_EFFECT_TYPES; i++)
		{
			if (visualEffectTypes[i].getName() != NULL
				&& strcmp(visualEffectTypes[i].getName(), effectname) == 0)
				return i;
		}
		return -1;
	}

	/**
	 * Run all of the visual effects. This should be called at static
	 * rate, once a game tick.
	 */
	void VisualEffectManager::run()
	{
		updateManagedEffects();

		// check for visual effects marked to be deleted
		// optimization trick: seperate deletion list, thanks to it
		// we won't have to use SafeLinkedListIterator, which would
		// be much more costly.

		// create a list of effects to be deleted
		LinkedList *delList = NULL;
		LinkedListIterator iter = LinkedListIterator(visualEffects);
		while (iter.iterateAvailable())
		{
			VisualEffect *v = (VisualEffect *)iter.iterateNext();
			if (v->isDeleteFlag())
			{
				if (v->deleteFrameCounter == 0)
				{
					// HACK!!!
					// special case: muzzleflash
					if (v->effectType->getType() == VisualEffectType::VISUALEFFECT_TYPE_MUZZLEFLASH)
					{
						if (v->visualObject != NULL && v->follow != NULL)
						{
							Muzzleflasher::deleteMuzzleflash(v->follow);
						}
					}

					if(!v->muzzleAttachment.empty())
					{
						if (v->visualObject != NULL && v->follow != NULL)
						{
							Muzzleflasher::deleteMuzzleflash(v->follow, v->muzzleAttachment);
						}
					}

					// HACK!!!
					// special case: eject
					if (v->effectType->getType() == VisualEffectType::VISUALEFFECT_TYPE_EJECT)
					{
						if (v->visualObject != NULL && v->follow != NULL)
						{
							Ejecter::deleteEject(v->follow);
						}
					}

					if (v->spotlight != NULL)
					{
						delete v->spotlight;
						v->spotlight = NULL;
					}

					if(decalSystem)
					{
						for(unsigned int i = 0; i < v->removableDecals.size(); ++i)
							decalSystem->eraseDecal(v->removableDecals[i]);

						v->removableDecals.clear();
					}

					v->visualObject->setVisible(false);
					v->visualObject->setInScene(false);
					v->follow = NULL;

					// now, as this is done, delete counter may be increased.
					v->advanceDeleteCounter = true;
				}
				if (v->deleteFrameCounter >= 5)
				{
					if (delList == NULL)
						delList = new LinkedList();
					delList->append(v);
				}
			}
		}
		// if some effects marked for deletion, delete them now...
		// actually just free the reference.
		if (delList != NULL)
		{
			while (!delList->isEmpty())
			{
				VisualEffect *v = (VisualEffect *)delList->popLast();
				visualEffects->remove(v);
				v->freeReference();
			}
			// then delete the delList itself.
			delete delList;
		}

		// run each effect
		LinkedListIterator iter2 = LinkedListIterator(visualEffects);
		while (iter2.iterateAvailable())
		{
			VisualEffect *v = (VisualEffect *)iter2.iterateNext();
			v->run();
		}

		// finally run the particlemanager...
		//particleManager->stepForwards(1);

		if (GAME_TICKS_PER_SECOND == 100)
		{
			// HACK: call the particle effect tick twice in every 3 frames to get ~67Hz 
			if(++particleRunCounter == 3)
				particleRunCounter = 0;
			else
				particleEffectManager->tick();
		} else {
			particleEffectManager->tick();
		}
		//if(++particleRunCounter == 3)
		//	particleRunCounter = 0;
		//else
		//	particleEffectManager->tick();

		//particleRunTwice = !particleRunTwice;
		//if (particleRunTwice)
		//	particleEffectManager->tick();

		decalSystem->update(GAME_TICK_MSEC);
	}


	/**
	 * Prepare all the visual effects for rendering. This should be
	 * called once for each frame (before the storm scene rendering).
	 */
	void VisualEffectManager::prepareForRender(game::GameMap *gameMap,
		util::ColorMap *colorMap, ui::LightManager *lightManager)
	{
		// render each visual effect
		LinkedListIterator iter = LinkedListIterator(visualEffects);
		while (iter.iterateAvailable())
		{
			VisualEffect *v = (VisualEffect *)iter.iterateNext();
			v->prepareForRender();
			if (colorMap != NULL)
			{
				ui::PointLights lights;
				if(v->visualObject || v->particleEffect)
				{
					VC3 position = v->position;
					position.x = position.x / gameMap->getScaledSizeX() + .5f;
					position.z = position.z / gameMap->getScaledSizeY() + .5f;

					int ox = gameMap->scaledToObstacleX(position.x);
					int oy = gameMap->scaledToObstacleY(position.z);
					if (gameMap->isWellInScaledBoundaries(position.x, position.z))
					{
						lights.ambient = gameMap->colorMap->getColor(position.x, position.z);

						if(gameMap->getAreaMap()->isAreaAnyValue(ox, oy, AREAMASK_INBUILDING))
							lightManager->getLighting(v->position, lights, 1.f, true, true);
						else
							lightManager->getLighting(v->position, lights, 1.f, true, false);
					}
				}

				if(v->visualObject != NULL)
				{
					v->visualObject->setSelfIllumination(lights.ambient);
					for(int i = 0; i < LIGHT_MAX_AMOUNT; ++i)
						v->visualObject->getStormModel()->SetLighting(i, lights.lightIndices[i]);
				}
				if(v->particleEffect)
				{
					v->visualObject->setSelfIllumination(lights.ambient);
					//v->particleEffect->setLighting(lights.ambient, lights.lights[0].position, lights.lights[0].color, lights.lights[0].range);
					//v->particleEffect->setLighting(lights.ambient, lights.lightIndices);

					COL ambient = lightManager->getApproximatedLightingForIndices(v->position, lights);
					v->particleEffect->setLighting(ambient, lights.lightIndices);
				}
			}
			if (v->advanceDeleteCounter)
			{
				v->deleteFrameCounter++;
			}
		}

		// render particles
		//particleManager->render();
		particleEffectManager->render();
	}

	void VisualEffectManager::loadParticleEffects() 
	{
		for (int i = 0; i < MAX_VISUAL_EFFECT_TYPES; i++)
		{
			visualEffectTypes[i].setParticleEffectID(-1);
			visualEffectTypes[i].setParticleEffectHardwareFluidID(-1);
			visualEffectTypes[i].setParticleEffectHardwareRigidID(-1);

			// load the particle effect for this visual effect, if one was defined in conf
			const char *particleEffect = visualEffectTypes[i].getParticleEffect();
			if(particleEffect != NULL) 
			{
				frozenbyte::editor::EditorParser parser(false, false);

				filesystem::InputStream stream = filesystem::FilePackageManager::getInstance().getFile(particleEffect);
				if(!stream.isEof())
				{
					stream >> parser;

					int efId = particleEffectManager->loadParticleEffect(parser);
					visualEffectTypes[i].setParticleEffectID(efId);
					if (efId == -1)
					{
						Logger::getInstance()->warning("VisualEffectManager::loadParticleEffects - Particle effect load failed.");
						Logger::getInstance()->debug(particleEffect);
					}
				} else {
					Logger::getInstance()->warning("VisualEffectManager::loadParticleEffects - Particle effect was defined for visual effect, but was not found.");
					Logger::getInstance()->debug(particleEffect);
				}
			}

			// should we try to load the hardware fluid particles too?
			// (only available on hardware)
			//if (game::SimpleOptions::getBool(DH_OPT_B_PHYSICS_FLUIDS_ENABLED)
			//	&& game::SimpleOptions::getBool(DH_OPT_B_PHYSICS_USE_HARDWARE))
			if (game::SimpleOptions::getBool(DH_OPT_B_PHYSICS_FLUIDS_ENABLED))
			{
				const char *particleEffectHardwareFluid = visualEffectTypes[i].getParticleEffectHardwareFluid();
				if(particleEffectHardwareFluid != NULL) 
				{
					frozenbyte::editor::EditorParser parser(false, false);

					filesystem::InputStream stream = filesystem::FilePackageManager::getInstance().getFile(particleEffectHardwareFluid);
					if(!stream.isEof())
					{
						stream >> parser;

						int efId = particleEffectManager->loadParticleEffect(parser);
						visualEffectTypes[i].setParticleEffectHardwareFluidID(efId);
						if (efId == -1)
						{
							Logger::getInstance()->warning("VisualEffectManager::loadParticleEffects - Hardware fluid particle effect load failed.");
							Logger::getInstance()->debug(particleEffectHardwareFluid);
						}
					} else {
						Logger::getInstance()->warning("VisualEffectManager::loadParticleEffects - Hardware fluid particle effect was defined for visual effect, but was not found.");
						Logger::getInstance()->debug(particleEffectHardwareFluid);
					}
				}
			}

			// should we try to load the hardware rigid particles too?
			// (notice, option naming that does not match particle effect naming)
			if (game::SimpleOptions::getBool(DH_OPT_B_PHYSICS_PARTICLES))
			{
				const char *particleEffectHardwareRigid = visualEffectTypes[i].getParticleEffectHardwareRigid();
				if(particleEffectHardwareRigid != NULL) 
				{
					frozenbyte::editor::EditorParser parser;

					filesystem::InputStream stream = filesystem::FilePackageManager::getInstance().getFile(particleEffectHardwareRigid);
					if(!stream.isEof())
					{
						stream >> parser;

						int efId = particleEffectManager->loadParticleEffect(parser);
						visualEffectTypes[i].setParticleEffectHardwareRigidID(efId);
						if (efId == -1)
						{
							Logger::getInstance()->warning("VisualEffectManager::loadParticleEffects - Hardware rigid particle effect load failed.");
							Logger::getInstance()->debug(particleEffectHardwareRigid);
						}
					} else {
						Logger::getInstance()->warning("VisualEffectManager::loadParticleEffects - Hardware rigid particle effect was defined for visual effect, but was not found.");
						Logger::getInstance()->debug(particleEffectHardwareRigid);
					}
				}
			}

		}
	
	}

	void VisualEffectManager::freeParticleEffects() 
	{	
		// not used yet	
	}


	void VisualEffectManager::loadDecalEffects() 
	{
		assert(this->decalSystem == NULL);
		assert(this->terrain != NULL);

		decalSystem = new frozenbyte::DecalSystem(*storm3d, terrain->getDecalSystem());

		for (int i = 0; i < MAX_VISUAL_EFFECT_TYPES; i++)
		{
			visualEffectTypes[i].setDecalEffectID(-1);
			const char *decalEffect = visualEffectTypes[i].getDecalEffect();
			if(decalEffect != NULL) 
			{
				int id = decalSystem->getEffectId(decalEffect);
				visualEffectTypes[i].setDecalEffectID(id);
				if (id == -1)
				{
					//assert(!"VisualEffectManager::loadDecalEffects - Decal effect was defined for visual effect, but was not found.");
					Logger::getInstance()->warning("VisualEffectManager::loadDecalEffects - Decal effect was defined for visual effect, but was not found.");
					Logger::getInstance()->debug(decalEffect);
				}
			}						
		}
	
	}

	void VisualEffectManager::freeDecalEffects() 
	{	
		if (decalSystem != NULL)
		{
			delete decalSystem;
			decalSystem = NULL;
		}
	}

	void VisualEffectManager::setGameScene(game::GameScene *gameScene) 
	{	
		this->gameScene = gameScene;

		if(gameScene)
		{
			particleCollision.reset(new ParticleCollision(gameScene->getGameMap(), gameScene));
			particleArea.reset(new ParticleArea(gameScene->getGameMap(), gameScene));
			fluidParticleCollision.reset(new FluidParticleCollision(gameScene->getGameMap(), gameScene));
		}
		else
		{
			particleCollision.reset();
			fluidParticleCollision.reset();
			particleArea.reset();
		}
	}

	void VisualEffectManager::enableParticleInsideCheck(bool enable)
	{
		if(particleArea)
			particleArea->enableInsideCheck(enable);
	}

	frozenbyte::particle::ParticleEffectManager *VisualEffectManager::getParticleEffectManager()
	{
		return particleEffectManager;
	}

}

