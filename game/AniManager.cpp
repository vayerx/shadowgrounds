
#include "precompiled.h"

#include "AniManager.h"

#include "Ani.h"
#include "scripting/GameScripting.h"
#include "../container/LinkedList.h"
#include "../system/Logger.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../util/Debug_MemoryManager.h"


namespace game
{

	class AniManagerImpl
	{
		private:
			AniManagerImpl(GameScripting *gameScripting)
			{
				this->gameScripting = gameScripting;
				anis = new LinkedList();
				currentScriptAni = NULL;
				aniPaused = false;
			}

			~AniManagerImpl()
			{
				while (!anis->isEmpty())
				{
					delete (Ani *) anis->popLast();
				}
				delete anis;
			}

			static AniManager *instance;
			LinkedList *anis;
			Ani *currentScriptAni;
			GameScripting *gameScripting;
			bool aniPaused;

		friend class AniManager;
	};

	AniManager *AniManagerImpl::instance = NULL;

	namespace {
		
		struct AniTracker
		{
			AniTracker()
			{
			}

			~AniTracker()
			{
				AniManager::getInstance()->cleanInstance();
			}
		};

		AniTracker aniTracker;
	}

	AniManager::AniManager(GameScripting *gameScripting)
	{
		this->impl = new AniManagerImpl(gameScripting);
	}


	AniManager::~AniManager()
	{
		delete impl;
	}


	void AniManager::createInstance(GameScripting *gameScripting)
	{
		if (AniManagerImpl::instance == NULL)
		{
			AniManagerImpl::instance = new AniManager(gameScripting);
		} else {
			assert(!"AniManager::createInstance - Attempt to create multiple instances (an instance already exists).");
		}
	}


	AniManager *AniManager::getInstance()
	{
		if (AniManagerImpl::instance == NULL)
		{
			//assert(!"AniManager::getInstance - Instance must first be created.");
			return NULL;
		}
		return AniManagerImpl::instance;
	}


	void AniManager::cleanInstance()
	{
		if (AniManagerImpl::instance != NULL)
		{
			delete AniManagerImpl::instance;
			AniManagerImpl::instance = NULL;
		}
	}


	// run all anis
	void AniManager::run()
	{
		if (!impl->aniPaused)
		{
			LinkedListIterator iter(impl->anis);
			while (iter.iterateAvailable())
			{
				Ani *ani = (Ani *)iter.iterateNext();
				this->setCurrentScriptAni(ani);
				ani->run();
			}
			this->setCurrentScriptAni(NULL);
		}
	}


	// creates a new (yet empty) ani
	Ani *AniManager::createNewAniInstance(Unit *unit)
	{
		Ani *ani = new Ani(impl->gameScripting, unit);
		impl->anis->append(ani);
		return ani;
	}


	// deletes a previously created ani
	void AniManager::deleteAni(Ani *ani)
	{
		assert(ani != NULL);
		if (impl->anis->isEmpty())
		{
			Logger::getInstance()->error("AniManager::deleteAni - Attempting to delete an ani from empty ani list.");
			assert(!"AniManager::deleteAni - Attempting to delete an ani from empty ani list.");
			return;
		}

		impl->anis->remove(ani);
		delete ani;
	}


	// get an existing ani with given name
	Ani *AniManager::getAniByName(const char *aniName)
	{
		if (aniName == NULL)
			return NULL;

		// TODO: what if there are multiple anis with same name?
		LinkedListIterator iter(impl->anis);
		while (iter.iterateAvailable())
		{
			Ani *ani = (Ani *)iter.iterateNext();
			const char *tmp = ani->getName();
			if (tmp != NULL)
			{
				if (strcmp(aniName, tmp) == 0)
				{
					return ani;
				}
			}
		}

		return NULL;
	}


	// returns true if all ani plays have ended
	bool AniManager::isAllAniComplete()
	{
		LinkedListIterator iter(impl->anis);
		while (iter.iterateAvailable())
		{
			Ani *ani = (Ani *)iter.iterateNext();
			if (!ani->hasPlayEnded())
				return false;
		}

		return true;
	}


	// get the ani being currently run (for scripting system)
	Ani *AniManager::getCurrentScriptAni()
	{
		return impl->currentScriptAni;
	}


	// needed or not??
	void AniManager::setCurrentScriptAni(Ani *ani)
	{
		impl->currentScriptAni = ani;
	}


	void AniManager::setAniPause(bool aniPaused)
	{
		impl->aniPaused = aniPaused;
	}


	void AniManager::stopAniForUnit(Unit *unit)
	{
		SafeLinkedListIterator iter(impl->anis);
		while (iter.iterateAvailable())
		{
			Ani *ani = (Ani *)iter.iterateNext();
			if (ani->getUnit() == unit)
			{
				if (!ani->hasPlayEnded())
					ani->stopPlay();
				//this->deleteAni(ani);
			}
		}
	}

	void AniManager::stopAllAniPlay()
	{
		LinkedListIterator iter(impl->anis);
		while (iter.iterateAvailable())
		{
			Ani *ani = (Ani *)iter.iterateNext();
			if (!ani->hasPlayEnded())
				ani->stopPlay();
		}
	}

	void AniManager::leapAllAniPlayToEnd()
	{
		LinkedListIterator iter(impl->anis);
		while (iter.iterateAvailable())
		{
			Ani *ani = (Ani *)iter.iterateNext();
			if (!ani->hasPlayEnded())
			{
				ani->leapToEnd();
			}
		}

		// NEW: run one tick for all anis (to make sure they are at the end state
		// when this call returns (so that cinematic scripts can assume the leap actually 
		// immediately runs the ani)
		this->run();
	}

}


