
#include "precompiled.h"

#include "../sound/SoundLooper.h"
#include "../sound/sounddefs.h"
#include "../game/GameUI.h"

#include "AmbientSound.h"
#include "../util/ClippedCircle.h"

#include "../convert/str2int.h"
#include "../system/Logger.h"

using namespace sfx;

namespace ui
{

AmbientSound::AmbientSound() {
	played = false;
	enabled = false;
	name = NULL;
	clip = 15;

	// umm... initialization was missing for all these...?
	key = 0;
	handle = 0;
	looped = false;
	range = 0;
	position = VC3(0,0,0);
	rollOff = true;
	volume = 100;
	defString = "";
}

AmbientSound::~AmbientSound() {
	if (this->name != NULL)
	{
		delete [] this->name;
		this->name = NULL;
	}
}

AmbientSound::AmbientSound(const AmbientSound& rhs) {
	*this = rhs;
	name = NULL;
	setName(rhs.name);
}

AmbientSound& AmbientSound::operator=(const AmbientSound& rhs) {

	played = rhs.played;
	enabled = rhs.enabled;
	key = rhs.key;
	handle = rhs.handle;
	looped = rhs.looped;
	range = rhs.range;
	position = rhs.position;
	rollOff = rhs.rollOff;
	volume = rhs.volume;
	defString = rhs.defString;
	clip = rhs.clip;
	name = NULL;
	setName(rhs.name);

	return *this;

}

void AmbientSound::setRange(float f) {
	range = f;
}

void AmbientSound::setPosition(const Vector& pos) {
	position = pos;
}

void AmbientSound::setRollOff(int r) {
	rollOff = r;
}

void AmbientSound::setVolume(int v) {
	volume = v;
}

void AmbientSound::enable() {

	enabled = true;
}

void AmbientSound::disable(bool immediately, sfx::SoundLooper* soundLooper) {

	enabled = false;
	if(played) {
		soundLooper->stopLoopedSound(handle, key, immediately);
		played = false;
	}
	
}

void AmbientSound::makeFromDefString(const char* string) {
	defString = string;
}
		
void AmbientSound::tick(game::GameUI* gameUI, SoundLooper* looper,
	const VC3 &listenerPosition) {

	if(!enabled)
		return;

	/*
	Vector v = listenerPosition - position;
	v.y = 0.0f; // (use 2d range, not 3d range)
	float distSq = v.GetSquareLength();
	*/

	bool inArea = util::ClippedCircle::isInsideArea(listenerPosition, position, range, clip);

	//if(distSq < range * range)  
	if (inArea)
	{
		if(!played) 
		{			
			if(rollOff == 0) 
			{
				if(gameUI->parseSoundFromDefinitionString(const_cast<char*>(defString.c_str()), listenerPosition.x, listenerPosition.y, listenerPosition.z,
					&looped, &handle, &key, false, range, DEFAULT_SOUND_PRIORITY_LOW, false, true) != -1) 
				{
					assert(looped);
					played = true;
				}
			} else {
				if(gameUI->parseSoundFromDefinitionString(const_cast<char*>(defString.c_str()), position.x, position.y, position.z,
					&looped, &handle, &key, false, range, DEFAULT_SOUND_PRIORITY_LOW, false, true) != -1) 
				{
					assert(looped);
					played = true;			
				}
			}
			
		}
	} else {
		if(played) 
		{
			//looper->stopLoopedSound(handle, key, false);
			looper->stopLoopedSound(handle, key, true);
			played = false;
		}
	}

	if (played)
	{
		if(!looper->isSoundStillLooping(handle, key))
			played = false;

		/*
		// Doesn't really work here
		if(!looper->isSoundStillPlaying(handle, key))
		{
			played = false;
			looper->stopLoopedSound(handle, key, true);
		}
		*/

		looper->setSoundPosition(handle, key, position.x, position.y, position.z);
	}

	if(played)
		looper->setSoundVolume(handle, key, volume);
/*
	if (played)
	{
		float dist = sqrtf(distSq);
		int vol = int((1.0f - (dist / range)) * 100.0f);
		if (vol < 0) vol = 0;
		if (vol > 100) vol = 100;
		//Logger::getInstance()->error(int2str(vol));
		looper->setSoundVolume(handle, key, vol);
	//} else {
		//Logger::getInstance()->error("NOT PLAYED");
	}
*/
}

void AmbientSound::setName(const char *name)
{
	if (this->name != NULL)
	{
		delete [] this->name;
		this->name = NULL;
	}
	if (name != NULL)
	{
		this->name = new char[strlen(name) + 1];
		strcpy(this->name, name);
	}
}

const char *AmbientSound::getName() const
{
	return this->name;
}

void AmbientSound::setClip(int quarters)
{
	clip = quarters;
}



} // ui
