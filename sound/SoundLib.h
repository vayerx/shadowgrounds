#ifndef SOUNDLIB_H
#define SOUNDLIB_H

//#define USE_DSOUND

#ifdef USE_DSOUND

#include "windows.h"
#include "stdio.h"
#include <dsound.h>
#include <dshow.h>
#include <commctrl.h>
#include <commdlg.h>

#define SLIB_SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SLIB_SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#define WM_GRAPHNOTIFY  WM_USER+13
#ifndef DSBCAPS_CTRLALL
#define DSBCAPS_CTRLALL ( DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY )
#endif

#define MAX_SOUNDS 512 

#define WAVEFILE_READ   1
#define WAVEFILE_WRITE  2

class CWaveFile
{
public:
    WAVEFORMATEX* m_pwfx;        // Pointer to WAVEFORMATEX structure
    HMMIO         m_hmmio;       // MM I/O handle for the WAVE
    MMCKINFO      m_ck;          // Multimedia RIFF chunk
    MMCKINFO      m_ckRiff;      // Use in opening a WAVE file
    DWORD         m_dwSize;      // The size of the wave file
    MMIOINFO      m_mmioinfoOut;
    DWORD         m_dwFlags;
    BOOL          m_bIsReadingFromMemory;
    BYTE*         m_pbData;
    BYTE*         m_pbDataCur;
    ULONG         m_ulDataSize;

protected:
    HRESULT ReadMMIO();
    HRESULT WriteMMIO( WAVEFORMATEX *pwfxDest );

public:
    CWaveFile();
    ~CWaveFile();

    HRESULT Open( const char *strFileName, WAVEFORMATEX* pwfx, DWORD dwFlags );
    HRESULT OpenFromMemory( BYTE* pbData, ULONG ulDataSize, WAVEFORMATEX* pwfx, DWORD dwFlags );
    HRESULT Close();

    HRESULT Read( BYTE* pBuffer, DWORD dwSizeToRead, DWORD* pdwSizeRead );
    HRESULT Write( UINT nSizeToWrite, BYTE* pbData, UINT* pnSizeWrote );

    DWORD   GetSize();
    HRESULT ResetFile();
    WAVEFORMATEX* GetFormat() { return m_pwfx; };
};

class CSound {

protected:

    DWORD                m_dwDSBufferSize;
    CWaveFile*           m_pWaveFile;
    DWORD                m_dwNumBuffers;


public:

    LPDIRECTSOUNDBUFFER* m_apDSBuffer;
    HRESULT RestoreBuffer( LPDIRECTSOUNDBUFFER pDSB, BOOL* pbWasRestored );

	int BaseFrequency;

    CSound( LPDIRECTSOUNDBUFFER* apDSBuffer, DWORD dwDSBufferSize, DWORD dwNumBuffers, CWaveFile* pWaveFile );
    virtual ~CSound();

    HRESULT Get3DBufferInterface( DWORD dwIndex, LPDIRECTSOUND3DBUFFER* ppDS3DBuffer );
    HRESULT FillBufferWithSound( LPDIRECTSOUNDBUFFER pDSB, BOOL bRepeatWavIfBufferLarger );
    LPDIRECTSOUNDBUFFER GetFreeBuffer();
    LPDIRECTSOUNDBUFFER GetBuffer( DWORD dwIndex );

    LPDIRECTSOUND		 m_pDS;

};

class CSoundStream
{
public:

  // initialized or not
  bool initOk;

	// DirectShow interfaces
	IGraphBuilder *pGB;
	IMediaControl *pMC;
	IMediaEventEx *pME;
	IBasicAudio   *pBA;
	IMediaSeeking *pMS;

	// Functions 

	virtual void SetVolume(int Volume); // Volume at 0 is full volume , -10000 is silence
	virtual void Stop();
	virtual void Play();
	virtual void Pause();
	virtual int IsComplete();

	CSoundStream();
	~CSoundStream();

};

class CSoundLib {

protected:

	float Volume;
	HWND g_Window;
    LPDIRECTSOUND m_pDS;

	LPDIRECTSOUNDBUFFER g_Sounds[MAX_SOUNDS];
	LPDIRECTSOUND3DBUFFER g_Sounds3D[MAX_SOUNDS];

	LPDIRECTSOUND3DLISTENER g_pDSListener;
	DS3DLISTENER            g_dsListenerParams;

    HRESULT Initialize( HWND hWnd, DWORD dwCoopLevel, DWORD dwPrimaryChannels, DWORD dwPrimaryFreq, DWORD dwPrimaryBitRate );
    inline  LPDIRECTSOUND GetDirectSound() { return m_pDS; }
    HRESULT SetPrimaryBufferFormat( DWORD dwPrimaryChannels, DWORD dwPrimaryFreq, DWORD dwPrimaryBitRate );
    HRESULT Get3DListenerInterface( LPDIRECTSOUND3DLISTENER* ppDSListener );

    HRESULT Create( CSound** ppSound, const char *strWaveFileName, DWORD dwCreationFlags = 0, GUID guid3DAlgorithm = GUID_NULL, DWORD dwNumBuffers = 1 );
    HRESULT CreateFromMemory( CSound** ppSound, BYTE* pbData, ULONG ulDataSize, LPWAVEFORMATEX pwfx, DWORD dwCreationFlags = 0, GUID guid3DAlgorithm = GUID_NULL, DWORD dwNumBuffers = 1 );

public:

	CSoundLib();
	~CSoundLib();

	int Initialize(HWND Window);

	CSound* LoadSample(const char *Filename);

	void SetListener(float x,float y,float z,
	 				float vx,float vy,float vz,
					float dx,float dy,float dz,
					float ux,float uy,float uz);

	int CreateSound(CSound *Sound,int Looping);
	void SetSound(int Sound,float x,float y,float z,float vx,float vy,float vz,int Volume,DWORD Position);
	void GetSound(int Sound,float &x,float &y,float &z,float &vx,float &vy,float &vz,int &Volume,DWORD &Position);
	void StopSound(int Sound);

	void SetSoundVolume(int Sound,int Volumed);

	bool IsSoundPlaying(int soundHandle);

	void Update();

	CSoundStream* CreateMP3Stream(const char *Filename);

	void SetGlobalVolume(int Left,int Right); // (Volume is 0-65535)
};

#else // fmod

#include <windows.h>
#include <fmod.h>
#include <datatypedef.h>
#include <string>
#include <map>
#include <boost/shared_array.hpp>

namespace sfx {

class SoundLib;

class Sound
{
	FSOUND_SAMPLE *sample;
	int volume;
	int priority;
	int length;
	bool fileNotFound;

	std::string filename;

	void updateProperties();

public:
	Sound(const char *file, int flags);
	~Sound();

	void setDefaultVolume(float value);
	void setPriority(int value);
	int getLength() const;

	const std::string &getFileName() const;

	friend SoundLib;
};

class SoundStream
{
	FSOUND_STREAM *stream;
	int channel;

	boost::shared_array<char> buffer;
	std::string filename;

	float baseVolume;
	float volume;
	int type; // SoundMixer::SoundStreamType

public:

	explicit SoundStream(const char *file);
	~SoundStream();

	void setBaseVolume(float value);
	void setVolume(float value);
	void setPanning(float value);
	void setLooping(bool loop);
	void play();
	void pause();
	void stop();

	inline void setType(int t) { type = t; }
	inline int getType() const { return type; }

	bool hasEnded() const;
	const std::string &getFileName() const;

	friend SoundLib;
};

class SoundLib
{
	bool initialized;
	bool useHardware;
	bool useEax;
	int mixrate;
	int softwareChannels;
	int minHardwareChannels;
	int maxHardwareChannels;
	int speakerType;

	float frequencyFactor;

	std::map<std::string, FSOUND_REVERB_PROPERTIES> soundAreas;
	std::map<std::string, std::string> areaAlias;

public:
	SoundLib();
	~SoundLib();

	enum SpeakerType
	{
		DolbyDigital = FSOUND_SPEAKERMODE_DOLBYDIGITAL,
		HeadPhones = FSOUND_SPEAKERMODE_HEADPHONES,
		MonoSpeakers = FSOUND_SPEAKERMODE_MONO,
		StereoSpeakers = FSOUND_SPEAKERMODE_STEREO,
		QuadSpeakers = FSOUND_SPEAKERMODE_QUAD,
		SurroundSpeakers = FSOUND_SPEAKERMODE_SURROUND
	};

	void setProperties(int mixrate, int softwareChannels);
	void setAcceleration(bool useHW, bool useEax, int minHardwareChannels, int maxHardwareChannels);
	void setSpeakers(SpeakerType speakerType);
	bool initialize(HWND window);
	void setFrequencyFactor(float scalar);

	Sound *loadSample(const char *file);
	SoundStream *createStream(const char *file);

	// Sounds
	int createSound(Sound *sound, int priority);
	void playSound(int sound);
	void setSoundLoop(int sound, bool loop);
	void setSoundPaused(int sound, bool pause);
	void setSoundVolume(int sound, float value);
	void setSoundFrequency(int sound, int value);
	void setSound3D(int sound, const VC3 &position, const VC3 &velocity);
	void stopSound(int sound);

	float getSoundVolume(int sound) const;
	int getSoundFrequency(int sound) const;
	int getSoundTime(int sound) const;
	bool isSoundPlaying(int sound) const;

	// Music

	// General
	void setGlobalVolume(float volume);
	void setListener(const VC3 &position, const VC3 &velocity, const VC3 &forwardDirection, const VC3 &upDirection);
	void setSoundArea(const std::string &name);

	void update();
};

} // sfx

#endif // fmod

#endif
