
#include "precompiled.h"

#include "PhysicsContactEffectManager.h"
#include "AbstractPhysicsObject.h"
#include "../Game.h"
#include "../GameUI.h"
#include "../ui/VisualEffectManager.h"
#include "../ui/VisualEffect.h"
#include "../../util/SoundMaterialParser.h"

#include "../../util/Debug_MemoryManager.h"


namespace game
{

	class PhysicsContactEffectManagerImpl
	{
	public:
		PhysicsContactEffectManagerImpl()
		{
			soundmp = new util::SoundMaterialParser();
		};

		~PhysicsContactEffectManagerImpl()
		{
			//deleteAllEffects();
			delete soundmp;
		};

		/*
		void deleteAllEffects()
		{
			while (!effectsRunning.isEmpty())
			{
				VisualEffect *vef = (VisualEffect *)effectsRunning.popLast();
				if (game->gameUI->getVisualEffectManager() != NULL)
				{
					game->gameUI->getVisualEffectManager()->deleteVisualEffect(vef);
					vef->freeReference();
				}
			}
		}

		void updateEffects()
		{
			// HACK: for now, just delete the effects every second or so?
			if ((game->gameTimer & 63) == 0)
			{
				deleteAllEffects();
			}
			// FIXME: real implementation!!!
		}
		*/

		util::SoundMaterialParser *soundmp;
		Game *game;
		//LinkedList effectsRunning;
	};



	PhysicsContactEffectManager::PhysicsContactEffectManager(Game *game)
	{
		impl = new PhysicsContactEffectManagerImpl();
		impl->game = game;
	}

	PhysicsContactEffectManager::~PhysicsContactEffectManager()
	{
		delete impl;
	}

	void PhysicsContactEffectManager::reloadConfiguration()
	{
		delete impl->soundmp;
		impl->soundmp = new util::SoundMaterialParser();
	}

	/*
	void PhysicsContactEffectManager::deleteAllEffects()
	{
		impl->deleteAllEffects();
	}

	void PhysicsContactEffectManager::updateEffects()
	{
		impl->updateEffects();
	}
	*/

	void PhysicsContactEffectManager::physicsContact(const PhysicsContact &contact)
	{
		// WARNING: unsafe IGamePhysicsObject -> AbstractPhysicsObject casts!
		AbstractPhysicsObject *o1 = (AbstractPhysicsObject *)contact.obj1;
		AbstractPhysicsObject *o2 = (AbstractPhysicsObject *)contact.obj2;
		//assert(o1 != NULL);
		//assert(o2 != NULL);
		assert(contact.physicsObject1);
		assert(contact.physicsObject2);

#ifdef PHYSICS_PHYSX
		int sm1 = contact.physicsObject1->getIntData();
		int sm2 = contact.physicsObject2->getIntData();
		if(sm1 == SOUNDMATERIALPARSER_NO_SOUND_INDEX || sm2 == SOUNDMATERIALPARSER_NO_SOUND_INDEX)
		{
			return;
		}
#else
		if (o1->getSoundMaterial() == SOUNDMATERIALPARSER_NO_SOUND_INDEX
			|| o2->getSoundMaterial() == SOUNDMATERIALPARSER_NO_SOUND_INDEX)
		{
			return;
		}
#endif

		const util::SoundMaterialParser::SoundMaterialList &smlist = impl->soundmp->getSoundMaterials();
		for (int i = 0; i < 2; i++)
		{
			AbstractPhysicsObject *o = o1;
			if (i == 1) 
			{
				o = o2;
			}
#ifdef PHYSICS_PHYSX
			int smindex = sm1;
			if (i == 1) 
			{
				smindex = sm2;
			}
			assert(smindex >= 0 && smindex < (int)smlist.size());
#else
			int smindex = o->getSoundMaterial();
#endif

			bool makeEffect = false;
			VC3 effectpos;

			if(o)
			{
				if(contact.contactForceLen >= smlist[smindex].requiredEffectForce)
				{
					effectpos = contact.contactPosition;

					VC3 accel = o->getAcceleration();
					VC3 angaccel = o->getAngularAcceleration();

					if (accel.GetSquareLength() >= smlist[smindex].requiredEffectAcceleration * smlist[smindex].requiredEffectAcceleration
						|| angaccel.GetSquareLength() >= smlist[smindex].requiredEffectAngularAcceleration * smlist[smindex].requiredEffectAngularAcceleration)
					{
						makeEffect = true;
					}

				}
			}
			else
			{
				if(contact.contactForceLen >= smlist[smindex].requiredEffectForce)
				{
					makeEffect = true;
					effectpos = contact.contactPosition;
				}
			}

			if(makeEffect)
			{
				if (o)
				{
					if (impl->game->gameTimer < o->getLastEffectTick() + (smlist[smindex].effectMaxRate / GAME_TICK_MSEC))
					{
						makeEffect = false;
					} else {
						o->setLastEffectTick(impl->game->gameTimer);
					}
				}
			}

			if(makeEffect)
			{
				std::vector<std::string> effectlist = smlist[smindex].effects;
				if (effectlist.size() > 0)
				{
					float effectFactor = 1.0f;
					if (smlist[smindex].requiredEffectForce > 0.0f)
					{
						// 0% - 100% effect factor (100% required force - 200% required force)
						effectFactor = (contact.contactForceLen / smlist[smindex].requiredEffectForce) - 1.0f;
						if (effectFactor > 1.0f)
							effectFactor = 1.0f;

						assert(effectFactor >= 0.0f);
						assert(effectFactor <= 1.0f);
					}

					int effnum = (int)(effectFactor * (effectlist.size() - 1));
					if (effnum < 0) effnum = 0;
					if (effnum >= (int)effectlist.size()) effnum = (int)effectlist.size() - 1;
					const char *effname = effectlist[effnum].c_str();

					ui::VisualEffectManager *vefman = impl->game->gameUI->getVisualEffectManager();
					if (vefman != NULL)
					{
						assert(effname != NULL);

						// TODO: optimize this!!!
						int visualEffId = vefman->getVisualEffectIdByName(effname);

						if (visualEffId != -1)
						{
							// TODO: proper lifetime
							int lifetime = GAME_TICKS_PER_SECOND / 2;
							VisualEffect *vef = vefman->createNewManagedVisualEffect(visualEffId, lifetime, NULL, NULL,
								effectpos, effectpos, VC3(0,0,0), VC3(0,0,0), impl->game);

							if (vef == NULL)
							{
								Logger::getInstance()->error("PhysicsContactEffectManager::physicsContact - Failed to create visual effect.");
								Logger::getInstance()->debug(effname);
							}
						} else {
							Logger::getInstance()->error("PhysicsContactEffectManager::physicsContact - Given visual effect name not found.");
							Logger::getInstance()->debug(effname);
						}
					}
				}
			}
		}
	}
}


