#ifndef AMBIENT_SOUND_H
#define AMBIENT_SOUND_H

#ifndef INCLUDED_STL_STRING
#define INCLUDED_STL_STRING
#include <string>
#endif

namespace sfx {
	class SoundLooper;
} // sfx

namespace game {
	class GameUI;
};

namespace ui
{

	class AmbientSoundManager;
	struct AmbientSoundManagerData;

	
class AmbientSound {

	// key and handles for the soundLooper
	int		key;
	int		handle;
	bool	looped;

	bool enabled;
	Vector position;
	bool played;
	float range;
	int rollOff;
	int volume;
	int clip;
	std::string defString;

	char *name;

public:

	AmbientSound();
	~AmbientSound();
	AmbientSound(const AmbientSound& rhs);
	AmbientSound& operator=(const AmbientSound& rhs);		

	void setRange(float f);
	void setPosition(const Vector& pos);
	void setRollOff(int r);
	void setVolume(int m);
	void setClip(int quarters);
	void enable();
	void disable(bool immediately, sfx::SoundLooper* looper);
	void makeFromDefString(const char* string);

	void setName(const char *name);
	const char *getName() const;
	
	void tick(game::GameUI* gameUI, sfx::SoundLooper* looper,
		const VC3 &listenerPosition);

	friend class AmbientSoundManager;
	friend struct AmbientSoundManagerData;
};

} // ui


#endif
