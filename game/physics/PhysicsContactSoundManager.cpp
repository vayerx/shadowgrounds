
#include "precompiled.h"

#include "PhysicsContactSoundManager.h"
#include "AbstractPhysicsObject.h"
#include "../GameUI.h"
#include "../Game.h"
#include "../../sound/sounddefs.h"
#include "../../util/SoundMaterialParser.h"

#include <sstream>


namespace game
{

	class PhysicsContactSoundManagerImpl
	{
	public:
		PhysicsContactSoundManagerImpl()
		{
			soundmp = new util::SoundMaterialParser();
		};

		~PhysicsContactSoundManagerImpl()
		{
			delete soundmp;
		};

		util::SoundMaterialParser *soundmp;
		GameUI *gameUI;
	};



	PhysicsContactSoundManager::PhysicsContactSoundManager(GameUI *gameUI)
	{
		impl = new PhysicsContactSoundManagerImpl();
		impl->gameUI = gameUI;
	}

	PhysicsContactSoundManager::~PhysicsContactSoundManager()
	{
		delete impl;
	}

	void PhysicsContactSoundManager::reloadConfiguration()
	{
		delete impl->soundmp;
		impl->soundmp = new util::SoundMaterialParser();
	}

	void PhysicsContactSoundManager::physicsContact(const PhysicsContact &contact)
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

		// int claw_material = impl->soundmp->getMaterialIndexByName( "claw" );
		// int claw_body_material = impl->soundmp->getMaterialIndexByName( "claw_body" );

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

			bool makeSound = false;
			VC3 soundpos;

			if(o)
			{
				/*
				// debug logging
				if( contact.contactForceLen > 1.0f && smlist[smindex].sounds.empty() == false )
				{
					VC3 accel = o->getAcceleration();
					VC3 angaccel = o->getAngularAcceleration();

					std::stringstream ss;
					ss << contact.contactForceLen << ", " << accel.GetLength() << ", " << angaccel.GetLength() << std::endl;
					Logger::getInstance()->error( ss.str().c_str() );
				}
				*/

				if(contact.contactForceLen >= smlist[smindex].requiredForce)
				{
					// TODO: use contact position instead of object's position.
					soundpos = o->getPosition();

					VC3 accel = o->getAcceleration();
					VC3 angaccel = o->getAngularAcceleration();

					

					

					if (accel.GetSquareLength() >= smlist[smindex].requiredAcceleration * smlist[smindex].requiredAcceleration
						|| angaccel.GetSquareLength() >= smlist[smindex].requiredAngularAcceleration * smlist[smindex].requiredAngularAcceleration)
					{
						makeSound = true;

						/*
						std::vector<std::string> soundlist = smlist[smindex].sounds;
						if (soundlist.size() > 0)
						{
							// TODO: some better pseudo-random logic here maybe...
							int sndnum = rand() % soundlist.size();
							const char *soundfile = soundlist[sndnum].c_str();

							float volumeFactor = 1.0f;
							if (smlist[smindex].requiredForce > 0.0f)
							{
								// 40% - 100% volume factor (100% required force - 200% required force)
								volumeFactor = (contact.contactForceLen / smlist[smindex].requiredForce) - 1.0f;
								volumeFactor *= 0.6f;
								volumeFactor += 0.4f;
								if (volumeFactor > 1.0f)
									volumeFactor = 1.0f;

								assert(volumeFactor >= 0.4f);
								assert(volumeFactor <= 1.0f);
							}

							impl->gameUI->playSoundEffect(soundfile, soundpos.x, soundpos.y, soundpos.z, 
								false, (int)(DEFAULT_SOUND_EFFECT_VOLUME * volumeFactor), DEFAULT_SOUND_RANGE, DEFAULT_SOUND_PRIORITY_NORMAL);
						}
						*/
					}

				}

				// not to repeat the effect too frequently
				if( o != NULL && makeSound && ( impl->gameUI->game->gameTimer - o->getLastEffectSoundTick() ) > ( 200 / GAME_TICK_MSEC ) )
				{
					o->setLastEffectSoundTick( impl->gameUI->game->gameTimer ); 
				}
				else
				{
					makeSound = false;
				}
			}
			else
			{
				if(contact.contactForceLen >= smlist[smindex].requiredForce)
				{
					makeSound = true;

#ifdef PHYSICS_PHYSX
					if(i == 0)
						contact.physicsObject1->getPosition(soundpos);
					else
						contact.physicsObject2->getPosition(soundpos);
#endif
				}
			}

			if(makeSound)
			{
				std::vector<std::string> soundlist = smlist[smindex].sounds;
				if (soundlist.size() > 0)
				{
					// TODO: some better pseudo-random logic here maybe...
					int sndnum = rand() % soundlist.size();
					// const char *soundfile = soundlist[sndnum].c_str();
					
					float volumeFactor = 1.0f;
					std::string soundfile = soundlist[sndnum];

					if (smlist[smindex].requiredForce > 0.0f)
					{
						// 100% required force - 500% required force
						float contactFactor = contact.contactForceLen / ( smlist[smindex].requiredForce * 5.0f );
						if( contactFactor > 1.0f ) 
							contactFactor = 1.0f;

						int sndnum = (int)((float)( soundlist.size() - 1 ) * contactFactor + 0.5f);
						sndnum += ( rand() % 3 ) - 1;

						if( sndnum < 0 ) 
							sndnum = 0;
						
						if( sndnum > (signed)( soundlist.size() - 1 ) ) 
							sndnum = (signed)( soundlist.size() - 1 );
						
						soundfile = soundlist[sndnum];
						/*
						// 40% - 100% volume factor (100% required force - 200% required force)
						volumeFactor = (contact.contactForceLen / smlist[smindex].requiredForce) - 1.0f;
						volumeFactor *= 0.6f;
						volumeFactor += 0.4f;
						if (volumeFactor > 1.0f)
							volumeFactor = 1.0f;

						assert(volumeFactor >= 0.4f);
						assert(volumeFactor <= 1.0f);
						*/
					}

					impl->gameUI->playSoundEffect(soundfile.c_str(), soundpos.x, soundpos.y, soundpos.z, 
						false, (int)(DEFAULT_SOUND_EFFECT_VOLUME * volumeFactor), DEFAULT_SOUND_RANGE, DEFAULT_SOUND_PRIORITY_NORMAL);
				}
			}
		}
	}
}


