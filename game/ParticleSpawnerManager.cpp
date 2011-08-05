
#include "precompiled.h"

#include "ParticleSpawnerManager.h"
#include "ParticleSpawner.h"
#include "../container/LinkedList.h"
#include <IStorm3D_Scene.h>
#include "Game.h"
#include "GameScene.h"
#include "SimpleOptions.h"
#include "options/options_graphics.h"


// NOTE: see the range calculation bug later on in this file...
// (that is why SG needs an extra high range)

#ifdef PROJECT_SHADOWGROUNDS
#define PARTICLE_ONSCREEN_MAX_RANGE 35
#else
#define PARTICLE_ONSCREEN_MAX_RANGE 20
#endif

namespace game
{

	ParticleSpawnerManager::ParticleSpawnerManager(Game *game)
	{
		this->game = game;
		this->spawnerList = new LinkedList();
		this->playerPosition = VC3(0,0,0);
	}	


  ParticleSpawnerManager::~ParticleSpawnerManager()
	{
		deleteAllParticleSpawners();

		delete spawnerList;
	}


  ParticleSpawner *ParticleSpawnerManager::createParticleSpawner()
	{
		ParticleSpawner *ret = new ParticleSpawner(game);
		spawnerList->append(ret);
		return ret;
	}


  void ParticleSpawnerManager::deleteParticleSpawner(ParticleSpawner *spawner)
	{
		spawnerList->remove(spawner);
		delete spawner;
	}


  void ParticleSpawnerManager::deleteAllParticleSpawners()
	{
		while (!spawnerList->isEmpty())
		{
			ParticleSpawner *ps = static_cast<ParticleSpawner *> (spawnerList->popLast());
			delete ps;
		}
	}


  void ParticleSpawnerManager::disableAllParticleSpawners()
	{
		LinkedListIterator iter(spawnerList);
		while (iter.iterateAvailable())
		{
			ParticleSpawner *s = (ParticleSpawner *)iter.iterateNext();
			s->disable();
		}
	}


	int ParticleSpawnerManager::getParticleSpawnerAmount()
	{
		int ret = 0;
		LinkedListIterator iter(spawnerList);
		while (iter.iterateAvailable())
		{
			iter.iterateNext();
			ret++;
		}
		return ret;
	}


  ParticleSpawner *ParticleSpawnerManager::getParticleSpawnerByName(const char *name)
	{
		LinkedListIterator iter(spawnerList);
		while (iter.iterateAvailable())
		{
			ParticleSpawner *s = (ParticleSpawner *)iter.iterateNext();
			if (s->getName() != NULL)
			{
				if (strcmp(s->getName(), name) == 0)
					return s;
			}
		}

		return NULL;
	}

  
	void ParticleSpawnerManager::run()
	{
		IStorm3D_Scene *scene = game->getGameScene()->getStormScene();
		IStorm3D_Camera *camera = (scene) ? scene->GetCamera() : 0;
		int effectLevel = game::SimpleOptions::getInt(DH_OPT_I_PARTICLE_EFFECTS_LEVEL);

		LinkedListIterator iter(spawnerList);
		while (iter.iterateAvailable())
		{
			ParticleSpawner *s = (ParticleSpawner *)iter.iterateNext();

			VC3 posDiff = s->position;
			posDiff -= playerPosition;
#ifdef PROJECT_SHADOWGROUNDS
			// NOTE: THIS IS A BUG!!! - LEFT UNFIXED TO SG, IN ORDER TO AVOID BLOWING IT UP...
			posDiff.y = playerPosition.y;
#else
			posDiff.y = 0;
#endif

			float posDiffLenSq = posDiff.GetSquareLength();
			s->distanceToListenerSq = posDiffLenSq;

			if(camera && effectLevel >= 100)
			{
#ifdef PROJECT_CLAW_PROTO
				if (!camera->TestSphereVisibility(s->position, 10.f)
					|| posDiffLenSq > 100*100)
#else
				if(!camera->TestSphereVisibility(s->position, 10.f))
#endif
					s->outsideScreen = true;
				else
					s->outsideScreen = false;
			}
			else
			{
				if (posDiffLenSq < PARTICLE_ONSCREEN_MAX_RANGE * PARTICLE_ONSCREEN_MAX_RANGE)
					s->outsideScreen = false;
				else
					s->outsideScreen = true;
			}

			/*
			float posDiffLenSq = posDiff.GetSquareLength();
			if (posDiffLenSq < PARTICLE_ONSCREEN_MAX_RANGE * PARTICLE_ONSCREEN_MAX_RANGE)
			{
				s->outsideScreen = false;
			} else {
				s->outsideScreen = true;
			}

			if(camera)
			{
				if(!camera->TestSphereVisibility(s->position, 10.f))
					s->outsideScreen = true;
				else
					s->outsideScreen = false;
			}
			else
				s->outsideScreen = true;
			*/

			s->run();
		}
	}

	void ParticleSpawnerManager::setPlayerPosition(const VC3 &playerPosition)
	{
		this->playerPosition = playerPosition;
	}


}

