
#include "precompiled.h"

#include <map>

#include "../sound/SoundLooper.h"
#include "../game/GameUI.h"

#include "AmbientSound.h"
#include "AmbientSoundManager.h"

#include "../system/Logger.h"

#define AMBIENTSOUNDMAN_MAX_FIND_FREE_NUM 256

// no more whining about "identifier was truncated to '255' characters.."
#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif

using namespace sfx;

namespace ui
{


struct AmbientSoundManagerData {

	std::map< int, AmbientSound > m_sounds;

	game::GameUI* m_gameUI;
	SoundLooper* m_soundLooper;
	int m_selectedSound;
	VC3 m_listenerPosition;
	
	AmbientSoundManagerData(game::GameUI* gameUI_, SoundLooper* soundLooper) {
		assert(gameUI_ != NULL);
		assert(soundLooper != NULL);
		m_gameUI = gameUI_;
		m_soundLooper = soundLooper;
	}

	void clearAllAmbientSounds() {
		
		std::map<int, AmbientSound>::iterator it = m_sounds.begin();
		while(it != m_sounds.end()) {
			it->second.disable(true, m_soundLooper);
			it++;
		}
		
		m_sounds.clear();
	}
	
	void setSelectedAmbientSound(int id) {
		m_selectedSound = id;
	}

	void setListenerPosition(const VC3 &listenerPosition) {
		m_listenerPosition = listenerPosition;
	}

	int getSelectedAmbientSound() {
		return m_selectedSound;
	}

	void makeAmbientSoundFromDefString(int sel, const char* string) {
		m_sounds[sel].makeFromDefString(string);
	}

	void setAmbientSoundRange(int sel, int f) {
		m_sounds[sel].setRange(static_cast<float>(f));
	}
	
	void setAmbientSoundClipQuarters(int sel, int clipq) {
		m_sounds[sel].setClip(clipq);
	}
	
	void setAmbientSoundRollOff(int sel, int r) {
		m_sounds[sel].setRollOff(r);
	}

	void setAmbientSoundVolume(int sel, int v) {
		m_sounds[sel].setVolume(v);
	}

	void setAmbientSoundName(int sel, const char *name) {
		m_sounds[sel].setName(name);
	}

	void setAmbientSoundPosition(int sel, const Vector& v) {
		m_sounds[sel].setPosition(v);
	}

	void startAmbientSound(int sel) {
		m_sounds[sel].enable();
	}

	void stopAmbientSound(int sel, bool immediately) {
		m_sounds[sel].disable(immediately, m_soundLooper);
	}

	void run() 
	{
		std::map< int, AmbientSound >::iterator it = m_sounds.begin();
		while(it != m_sounds.end()) 
		{
			it->second.tick(m_gameUI, m_soundLooper, m_listenerPosition);
			it++;
		}
	
	}

};

AmbientSoundManager::AmbientSoundManager(game::GameUI* gameUI_, SoundLooper* looper_) {
	//boost::scoped_ptr<AmbientSoundManagerData> p(new AmbientSoundManagerData(gameUI_, looper_));
	//m.swap(p);
	m = new AmbientSoundManagerData(gameUI_, looper_);
}

AmbientSoundManager::~AmbientSoundManager() {

	m->clearAllAmbientSounds();
	
	delete m;

}

void AmbientSoundManager::clearAllAmbientSounds() {
	m->clearAllAmbientSounds();
}

void AmbientSoundManager::setSelectedAmbientSound(int i) {
	m->setSelectedAmbientSound(i);
}

void AmbientSoundManager::setNextFreeAmbientSound()
{
	int freeNum = -1;
	for (int i = 0; i < AMBIENTSOUNDMAN_MAX_FIND_FREE_NUM; i++)
	{
		bool usedAlready = false;
		std::map<int, AmbientSound>::iterator it = m->m_sounds.begin();
		while(it != m->m_sounds.end()) 
		{
			if (it->first == i)
			{
				usedAlready = true;
			}
			it++;
		}
		if (!usedAlready)
		{
			freeNum = i;
			break;
		}
	}
	if (freeNum != -1)
	{
		m->setSelectedAmbientSound(freeNum);
	} else {
		Logger::getInstance()->warning("AmbientSoundManager::setNextFreeAmbientSound - No free ambient sound id number found. (too many ambient sounds?)");
	}
}

int AmbientSoundManager::getSelectedAmbientSound() {
	return m->getSelectedAmbientSound();
}

void AmbientSoundManager::makeAmbientSoundFromDefString(int i, const char* string) {
	m->makeAmbientSoundFromDefString(i, string);
}

void AmbientSoundManager::setAmbientSoundRange(int i, float range) {
	m->setAmbientSoundRange(i, (int)range);
}

void AmbientSoundManager::setAmbientSoundClip(int i, int clipQuarters) {
	m->setAmbientSoundRange(i, clipQuarters);
}

void AmbientSoundManager::setAmbientSoundRollOff(int i, int rollOff) {
	m->setAmbientSoundRollOff(i, rollOff);
}

void AmbientSoundManager::setAmbientSoundVolume(int i, int volume) {
	m->setAmbientSoundVolume(i, volume);
}

void AmbientSoundManager::setAmbientSoundName(int i, const char *name) {
	m->setAmbientSoundName(i, name);
}

void AmbientSoundManager::setAmbientSoundPosition(int i, const Vector& pos) {
	m->setAmbientSoundPosition(i, pos);
}

void AmbientSoundManager::startAmbientSound(int i) {
	m->startAmbientSound(i);
}

void AmbientSoundManager::stopAmbientSound(int i, bool immediately) {
	m->stopAmbientSound(i, immediately);
}

void AmbientSoundManager::run() {
	m->run();
}

void AmbientSoundManager::setListenerPosition(const Vector& listenerPosition)
{
  m->setListenerPosition(listenerPosition);
}


int AmbientSoundManager::getAmbientSoundNumberByName(const char *name)
{
	if (name == NULL)
	{
		assert(!"AmbientSoundManager::getAmbientSoundNumberByName - Null name parameter given.");
		return -1;
	}

	int ret = -1;

	std::map<int, AmbientSound>::iterator it = m->m_sounds.begin();
	while(it != m->m_sounds.end()) 
	{
		if (it->second.getName() != NULL
			&& strcmp(it->second.getName(), name) == 0)
		{
			if (ret != -1)
			{
				Logger::getInstance()->warning("AmbientSoundManager::getAmbientSoundNumberByName - Multiple ambient sounds with requested name.");
				Logger::getInstance()->debug(name);
			}
			ret = it->first;
		}
		it++;
	}
	return ret;
}

void AmbientSoundManager::selectAmbientSoundByName(const char *name)
{
	int num = getAmbientSoundNumberByName(name);
	if (num != -1)
	{
		setSelectedAmbientSound(num);
	} else {
		Logger::getInstance()->warning("AmbientSoundManager::getAmbientSoundNumberByName - No ambient sound found with given name.");
		Logger::getInstance()->debug(name);
	}
}



} // ui
