#ifndef AMBIENT_SOUND_MANAGER_H
#define AMBIENT_SOUND_MANAGER_H

namespace sfx {
	class SoundLooper;
} // sfx

namespace game 
{
	class GameUI;
}


namespace ui
{
	struct AmbientSoundManagerData;	

	class AmbientSoundManager 
	{

	public:

		AmbientSoundManager(game::GameUI* gameUI_, sfx::SoundLooper* looper_);

		virtual ~AmbientSoundManager();

		void clearAllAmbientSounds(); 
	 		
		void setNextFreeAmbientSound();

		void setSelectedAmbientSound(int i); 

		int getSelectedAmbientSound();

		void setAmbientSoundRange(int i, float f);

		void setAmbientSoundClip(int i, int clipQuarters);

		void setAmbientSoundPosition(int i, const Vector& pos);

		void setAmbientSoundRollOff(int i, int rollOff);

		void setAmbientSoundVolume(int i, int volume);

		void setAmbientSoundName(int i, const char *name);

		void makeAmbientSoundFromDefString(int i, const char* defString);
		
		void startAmbientSound(int i);

		void stopAmbientSound(int i, bool immediately);

		void setListenerPosition(const Vector& listenerPosition);

		void run();

		int getAmbientSoundNumberByName(const char *name);

		void selectAmbientSoundByName(const char *name);

	private:

		AmbientSoundManagerData* m;	

	};

}

#endif
