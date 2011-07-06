
#include "precompiled.h"

#include "ParticleSpawnerManager.h"
#include "ParticleSpawner.h"
#include "../container/LinkedList.h"
#include <IStorm3d_Scene.h>
#include "Game.h"
#include "GameScene.h"
#include "SimpleOptions.h"
#include "UnifiedHandleManager.h"
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

			if (s->attached)
			{
				if (game->unifiedHandleManager->doesObjectExist(s->attachedTo))
				{
					s->position = game->unifiedHandleManager->getObjectPosition(s->attachedTo);					
				}
			}

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



	void ParticleSpawnerManager::attachParticleSpawner(UnifiedHandle particleSpawner, UnifiedHandle toObject)
	{
		ParticleSpawner *p = getParticleSpawnerByUnifiedHandle(particleSpawner);
		if (p != NULL)
		{
			assert(!p->attached);
			p->attached = true;
			p->attachedTo = toObject;
		}
	}


	UnifiedHandle ParticleSpawnerManager::findUnifiedHandleByUniqueEditorObjectHandle(UniqueEditorObjectHandle ueoh) const
	{
		assert(!"ParticleSpawnerManager::findUnifiedHandleByUniqueEditorObjectHandle - TODO.");
		return UNIFIED_HANDLE_NONE;
	}



	UnifiedHandle ParticleSpawnerManager::getUnifiedHandle(ParticleSpawner *spawner) const
	{
		// TODO: optimize! (this is really REALLY unoptimal)
		int atpos = 0;
		LinkedListIterator iter(spawnerList);
		while (iter.iterateAvailable())
		{
			ParticleSpawner *p = (ParticleSpawner *)iter.iterateNext();
			if (p == spawner)
			{
				UnifiedHandle ret = UNIFIED_HANDLE_FIRST_PARTICLE_SPAWNER + atpos;
				return ret;
			}

			++atpos;
		}

		assert(!"ParticleSpawnerManager::getUnifiedHandle - not in list, this should never happen.");

		return UNIFIED_HANDLE_NONE;
	}



	ParticleSpawner *ParticleSpawnerManager::getParticleSpawnerByUnifiedHandle(UnifiedHandle handle) const
	{
		if (!VALIDATE_UNIFIED_HANDLE_BITS(handle))
		{
			assert(!"ParticleSpawnerManager::getParticleSpawnerByUnifiedHandle - invalid handle paramater.");
			return false;
		}
		if (!IS_UNIFIED_HANDLE_PARTICLE_SPAWNER(handle))
		{
			assert(!"ParticleSpawnerManager::getParticleSpawnerByUnifiedHandle - invalid handle paramater.");
			return false;
		}

		int index = handle - UNIFIED_HANDLE_FIRST_PARTICLE_SPAWNER;

		// TODO: optimize! (this is really REALLY unoptimal)
		int atpos = 0;
		LinkedListIterator iter(spawnerList);
		while (iter.iterateAvailable())
		{
			ParticleSpawner *p = (ParticleSpawner *)iter.iterateNext();
			if (atpos == index)
				return p;

			++atpos;
		}

		return NULL;
	}



	bool ParticleSpawnerManager::doesParticleSpawnerExist(UnifiedHandle handle) const
	{
		assert(VALIDATE_UNIFIED_HANDLE_BITS(handle));
		assert(IS_UNIFIED_HANDLE_PARTICLE_SPAWNER(handle));

		if (handle == UNIFIED_HANDLE_NONE)
		{
			assert(!"ParticleSpawnerManager::doesParticleSpawnerExist - invalid handle paramater.");
			return false;
		}

		ParticleSpawner *orig = getParticleSpawnerByUnifiedHandle(handle);
		if (orig != NULL)
		{
			return true;
		} else {
			return false;
		}
	}



	UnifiedHandle ParticleSpawnerManager::getFirstParticleSpawner() const
	{
		// TODO: optimize! (this is really unoptimal)
		LinkedListIterator iter(spawnerList);
		if (iter.iterateAvailable())
		{
			ParticleSpawner *p = (ParticleSpawner *)iter.iterateNext();
			return getUnifiedHandle(p);
		} else {
			return UNIFIED_HANDLE_NONE;
		}
	}



	UnifiedHandle ParticleSpawnerManager::getNextParticleSpawner(UnifiedHandle handle) const
	{
		assert(VALIDATE_UNIFIED_HANDLE_BITS(handle));
		assert(IS_UNIFIED_HANDLE_PARTICLE_SPAWNER(handle));

		if (handle == UNIFIED_HANDLE_NONE)
		{
			assert(!"ParticleSpawnerManager::getNextParticleSpawner - invalid handle paramater.");
			return UNIFIED_HANDLE_NONE;
		}

		ParticleSpawner *orig = getParticleSpawnerByUnifiedHandle(handle);
		if (orig == NULL)
		{
			assert(!"ParticleSpawnerManager::getNextParticleSpawner - invalid handle paramater.");
			return UNIFIED_HANDLE_NONE;
		}

		// TODO: optimize! (this is really REALLY unoptimal)
		LinkedListIterator iter(spawnerList);
		while (iter.iterateAvailable())
		{
			ParticleSpawner *p = (ParticleSpawner *)iter.iterateNext();
			if (p == orig)
			{
				if (iter.iterateAvailable())
				{
					ParticleSpawner *p2 = (ParticleSpawner *)iter.iterateNext();
					return getUnifiedHandle(p2);
				} else {
					return UNIFIED_HANDLE_NONE;
				}
			}
		}
		return UNIFIED_HANDLE_NONE;
	}



	VC3 ParticleSpawnerManager::getParticleSpawnerPosition(UnifiedHandle unifiedHandle) const
	{
		ParticleSpawner *p = getParticleSpawnerByUnifiedHandle(unifiedHandle);
		if (p != NULL)
		{
			return p->position;
		} else {
			assert(!"ParticleSpawnerManager::getParticleSpawnerPosition - invalid handle paramater.");
			return VC3(0,0,0);
		}
	}



	void ParticleSpawnerManager::setParticleSpawnerPosition(UnifiedHandle unifiedHandle, const VC3 &position)
	{
		ParticleSpawner *p = getParticleSpawnerByUnifiedHandle(unifiedHandle);
		if (p != NULL)
		{
			p->position = position;
		} else {
			assert(!"ParticleSpawnerManager::setParticleSpawnerPosition - invalid handle paramater.");
		}
	}

}

