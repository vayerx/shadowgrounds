#include "precompiled.h"

#include <boost/lexical_cast.hpp>

#ifdef _WIN32
typedef unsigned __int64 Uint64;
#else
typedef uint64_t Uint64;
#endif

#include "SoundLib.h"

// NOTE: unwanted dependency (due to sound_missing_warning option)
#include "../game/SimpleOptions.h"
#include "../game/options/options_debug.h"
#include "../filesystem/file_package_manager.h"
#include "../filesystem/input_stream.h"

#ifndef PROJECT_VIEWER
extern bool isThisDeveloper();
#endif

using namespace frozenbyte;

#ifdef USE_DSOUND

#include <dsound.h>
#include <assert.h>

#include "../util/Debug_MemoryManager.h"


//
// vX.X - 6.6.2002 - jpkokkon
// Modified to handle CSoundStream load failures properly
//
// vX.X - 18.6.2002 - jpkokkon
// SAFE_DELETE macros remamed to SLIB_SAFE_DELETE 
// (so compiler won't whine about redefinitions when used with storm)
//

// vX.XX - 1.8.2002 - psd
// smaller rollof

#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "strmiids.lib")

CSoundLib::CSoundLib()
{
	m_pDS=0;
	memset(g_Sounds,0,sizeof(g_Sounds));
	Volume=1;
}

CSoundLib::~CSoundLib()
{
	SAFE_RELEASE(m_pDS);
}

void CSoundLib::SetGlobalVolume(int Left,int Right)
{
	waveOutSetVolume(0, (Left)+(Right<<16));
}

//-----------------------------------------------------------------------------
// HRESULT CSoundLib::Get3DListenerInterface( LPDIRECTSOUND3DLISTENER* ppDSListener )
//-----------------------------------------------------------------------------
HRESULT CSoundLib::Get3DListenerInterface( LPDIRECTSOUND3DLISTENER* ppDSListener )
{
    HRESULT             hr;
    DSBUFFERDESC        dsbdesc;
    LPDIRECTSOUNDBUFFER pDSBPrimary = NULL;

    if( ppDSListener == NULL )
        return E_INVALIDARG;
    if( m_pDS == NULL )
        return CO_E_NOTINITIALIZED;

    *ppDSListener = NULL;

    // Obtain primary buffer, asking it for 3D control
    ZeroMemory( &dsbdesc, sizeof(DSBUFFERDESC) );
    dsbdesc.dwSize = sizeof(DSBUFFERDESC);
    dsbdesc.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER;

		// TEMP: force software mixing.
		// TODO: use an option for this.
		dsbdesc.dwFlags |= DSBCAPS_LOCSOFTWARE;

    if( FAILED( hr = m_pDS->CreateSoundBuffer( &dsbdesc, &pDSBPrimary, NULL ) ) )
        return S_FALSE;

    if( FAILED( hr = pDSBPrimary->QueryInterface( IID_IDirectSound3DListener, 
                                                  (VOID**)ppDSListener ) ) )
    {
        SAFE_RELEASE( pDSBPrimary );
        return S_FALSE;
    }

    // Release the primary buffer, since it is not need anymore
    SAFE_RELEASE( pDSBPrimary );

	// psd
	//(*ppDSListener)->SetRolloffFactor(DS3D_MINROLLOFFFACTOR , DS3D_IMMEDIATE);
	// increased rolloff from .3 -> .5 
	// (as game ranges have dropped significantly)
	// -- jpk
	// and more... .5 -> 2.0
	// -- jpk
	//(*ppDSListener)->SetRolloffFactor(2.0f , DS3D_IMMEDIATE);
	HRESULT retCode = (*ppDSListener)->SetRolloffFactor(2.0f , DS3D_IMMEDIATE);
	//HRESULT retCode = (*ppDSListener)->SetRolloffFactor(DS3D_MINROLLOFFFACTOR , DS3D_IMMEDIATE);
	assert(retCode == DS_OK);

  return S_OK;
}

//-----------------------------------------------------------------------------
// HRESULT CSoundLib::Create
//-----------------------------------------------------------------------------
HRESULT CSoundLib::Create( CSound** ppSound, 
                               const char *strWaveFileName, 
                               DWORD dwCreationFlags, 
                               GUID guid3DAlgorithm,
                               DWORD dwNumBuffers )
{
    HRESULT hr;
    HRESULT hrRet = S_OK;
    DWORD   i;
    LPDIRECTSOUNDBUFFER* apDSBuffer     = NULL;
    DWORD                dwDSBufferSize = NULL;
    CWaveFile*           pWaveFile      = NULL;

    if( m_pDS == NULL )
        return CO_E_NOTINITIALIZED;
    if( strWaveFileName == NULL || ppSound == NULL || dwNumBuffers < 1 )
        return E_INVALIDARG;

    apDSBuffer = new LPDIRECTSOUNDBUFFER[dwNumBuffers];
    if( apDSBuffer == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto LFail;
    }

    pWaveFile = new CWaveFile();
    if( pWaveFile == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto LFail;
    }

    pWaveFile->Open( strWaveFileName, NULL, WAVEFILE_READ );

    if( pWaveFile->GetSize() == 0 )
    {
        // Wave is blank, so don't create it.
        hr = E_FAIL;
        goto LFail;
    }

    // Make the DirectSound buffer the same size as the wav file
    dwDSBufferSize = pWaveFile->GetSize();

    // Create the direct sound buffer, and only request the flags needed
    // since each requires some overhead and limits if the buffer can 
    // be hardware accelerated
    DSBUFFERDESC dsbd;
    ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
    dsbd.dwSize          = sizeof(DSBUFFERDESC);
    dsbd.dwFlags         = dwCreationFlags;
    dsbd.dwBufferBytes   = dwDSBufferSize;
    dsbd.guid3DAlgorithm = guid3DAlgorithm;
    dsbd.lpwfxFormat     = pWaveFile->m_pwfx;

    // DirectSound is only guarenteed to play PCM data.  Other
    // formats may or may not work depending the sound card driver.
    hr = m_pDS->CreateSoundBuffer( &dsbd, &apDSBuffer[0], NULL );

    // Be sure to return this error code if it occurs so the
    // callers knows this happened.
    if( hr == DS_NO_VIRTUALIZATION )
        hrRet = DS_NO_VIRTUALIZATION;
            
    if( FAILED(hr) )
    {
        // DSERR_BUFFERTOOSMALL will be returned if the buffer is
        // less than DSBSIZE_FX_MIN (100ms) and the buffer is created
        // with DSBCAPS_CTRLFX.
        if( hr != DSERR_BUFFERTOOSMALL )
            S_FALSE;
            
        goto LFail;
    }

    for( i=1; i<dwNumBuffers; i++ )
    {
        if( FAILED( hr = m_pDS->DuplicateSoundBuffer( apDSBuffer[0], &apDSBuffer[i] ) ) )
        {
            goto LFail;
        }
    }

    // Create the sound
    *ppSound = new CSound( apDSBuffer, dwDSBufferSize, dwNumBuffers, pWaveFile );
	(*ppSound)->m_pDS=m_pDS;

  // this was a non-array delete, changed it to array delete --jpk
    if (apDSBuffer) delete [] apDSBuffer;
    return hrRet;

LFail:
    // Cleanup
    if (pWaveFile) delete pWaveFile;
    if (apDSBuffer) delete [] apDSBuffer;
    return hr;
}

//-----------------------------------------------------------------------------
// HRESULT CSoundLib::CreateFromMemory
//-----------------------------------------------------------------------------
HRESULT CSoundLib::CreateFromMemory( CSound** ppSound, 
                                        BYTE* pbData,
                                        ULONG  ulDataSize,
                                        LPWAVEFORMATEX pwfx,
                                        DWORD dwCreationFlags, 
                                        GUID guid3DAlgorithm,
                                        DWORD dwNumBuffers )
{
    HRESULT hr;
    DWORD   i;
    LPDIRECTSOUNDBUFFER* apDSBuffer     = NULL;
    DWORD                dwDSBufferSize = NULL;
    CWaveFile*           pWaveFile      = NULL;

    if( m_pDS == NULL )
        return CO_E_NOTINITIALIZED;
    if( pbData == NULL || ppSound == NULL || dwNumBuffers < 1 )
        return E_INVALIDARG;

    apDSBuffer = new LPDIRECTSOUNDBUFFER[dwNumBuffers];
    if( apDSBuffer == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto LFail;
    }

    pWaveFile = new CWaveFile();
    if( pWaveFile == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto LFail;
    }

    pWaveFile->OpenFromMemory( pbData,ulDataSize, pwfx, WAVEFILE_READ );


    // Make the DirectSound buffer the same size as the wav file
    dwDSBufferSize = ulDataSize;

    // Create the direct sound buffer, and only request the flags needed
    // since each requires some overhead and limits if the buffer can 
    // be hardware accelerated
    DSBUFFERDESC dsbd;
    ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
    dsbd.dwSize          = sizeof(DSBUFFERDESC);
    dsbd.dwFlags         = dwCreationFlags;
    dsbd.dwBufferBytes   = dwDSBufferSize;
    dsbd.guid3DAlgorithm = guid3DAlgorithm;
    dsbd.lpwfxFormat     = pwfx;

    if( FAILED( hr = m_pDS->CreateSoundBuffer( &dsbd, &apDSBuffer[0], NULL ) ) )
    {
        goto LFail;
    }

    for( i=1; i<dwNumBuffers; i++ )
    {
        if( FAILED( hr = m_pDS->DuplicateSoundBuffer( apDSBuffer[0], &apDSBuffer[i] ) ) )
        {
            goto LFail;
        }
    }

    // Create the sound
    *ppSound = new CSound( apDSBuffer, dwDSBufferSize, dwNumBuffers, pWaveFile );

    if (apDSBuffer) delete apDSBuffer;
    return S_OK;

LFail:
    // Cleanup
   
    if (apDSBuffer) delete apDSBuffer;
    return hr;
}

//-----------------------------------------------------------------------------
// HRESULT CSoundLib::SetPrimaryBufferFormat(DWORD dwPrimaryChannels,DWORD dwPrimaryFreq,DWORD dwPrimaryBitRate)
// Sets primary direcsound buffer
//-----------------------------------------------------------------------------
HRESULT CSoundLib::SetPrimaryBufferFormat(DWORD dwPrimaryChannels,DWORD dwPrimaryFreq,DWORD dwPrimaryBitRate)
{
    HRESULT             hr;
    LPDIRECTSOUNDBUFFER pDSBPrimary = NULL;

    if( m_pDS == NULL )
        return CO_E_NOTINITIALIZED;

    // Get the primary buffer 
    DSBUFFERDESC dsbd;
    ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
    dsbd.dwSize        = sizeof(DSBUFFERDESC);
    dsbd.dwFlags       = DSBCAPS_PRIMARYBUFFER;
    dsbd.dwBufferBytes = 0;
    dsbd.lpwfxFormat   = NULL;
       
    if( FAILED( hr = m_pDS->CreateSoundBuffer( &dsbd, &pDSBPrimary, NULL ) ) )
        return S_FALSE;

    WAVEFORMATEX wfx;
    ZeroMemory( &wfx, sizeof(WAVEFORMATEX) ); 
    wfx.wFormatTag      = WAVE_FORMAT_PCM; 
    wfx.nChannels       = (WORD) dwPrimaryChannels; 
    wfx.nSamplesPerSec  = dwPrimaryFreq; 
    wfx.wBitsPerSample  = (WORD) dwPrimaryBitRate; 
    wfx.nBlockAlign     = wfx.wBitsPerSample / 8 * wfx.nChannels;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

    if( FAILED( hr = pDSBPrimary->SetFormat(&wfx) ) )
        return S_FALSE;

    SAFE_RELEASE( pDSBPrimary );

    return S_OK;
}

int CSoundLib::Initialize(HWND Window)
{
	g_Window=Window;

    HRESULT             hr;
    LPDIRECTSOUNDBUFFER pDSBPrimary = NULL;

    SAFE_RELEASE( m_pDS );

    // Create IDirectSound using the primary sound device
    if( FAILED( hr = DirectSoundCreate( NULL, &m_pDS, NULL ) ) )
	{
		return S_FALSE;
	}

    // Set DirectSound coop level 
    if( FAILED( hr = m_pDS->SetCooperativeLevel( g_Window, DSSCL_PRIORITY ) ) )
        return S_FALSE;
    
    // Set primary buffer format
    SetPrimaryBufferFormat( 2, 44100, 16 );

	Get3DListenerInterface(&g_pDSListener);

	return S_OK;
}

//-----------------------------------------------------------------------------
// CSound CSoundLib::LoadSample(char *Filename)
// Loads sample from ".wav" wavefile
//-----------------------------------------------------------------------------
CSound* CSoundLib::LoadSample(const char *Filename)
{
	CSound *Temp;

  HRESULT hr = Create( &Temp, Filename, DSBCAPS_LOCDEFER|DSBCAPS_CTRL3D|DSBCAPS_CTRLVOLUME|DSBCAPS_GETCURRENTPOSITION2 , DS3DALG_HRTF_FULL,1);

  // check for failure added...
  // NOTICE: may leak memory in case of failure(?)
  // -- jpk
  if (FAILED(hr)) return NULL;

	return Temp;
}

int CSoundLib::CreateSound(CSound* Sound,int Looping)
{
	int i;
	HRESULT hr;
	BOOL bRestored;

	// Find free sound
	for (i=0;i<MAX_SOUNDS;i++) if (!g_Sounds[i]) break;

  // added missing check when not finding a free sound...
  // -jpk
  if (i == MAX_SOUNDS) return -1;


	m_pDS->DuplicateSoundBuffer( Sound->m_apDSBuffer[0], &g_Sounds[i] );

	LPDIRECTSOUNDBUFFER pDSB=g_Sounds[i];

    if( pDSB == NULL )
        return -1;

    // Restore the buffer if it was lost
    if( FAILED( hr = Sound->RestoreBuffer( pDSB, &bRestored ) ) )
        return -1;

    if( bRestored )
    {
        // The buffer was restored, so we need to fill it with new data
        if( FAILED( hr = Sound->FillBufferWithSound( pDSB, false ) ) )
            return -1;
    }
    
	// Get 3D interface
	g_Sounds[i]->QueryInterface( IID_IDirectSound3DBuffer, (VOID**)&g_Sounds3D[i] );

	// Start playing the sound
	g_Sounds[i]->Play(0,0,Looping?DSBPLAY_LOOPING:0);

	return i;
}

void CSoundLib::Update()
{
	int i;

	// Clear sounds that aren't playing anymore...
	for (i=0;i<MAX_SOUNDS;i++) if (g_Sounds[i]) {
		DWORD Status;
		g_Sounds[i]->GetStatus(&Status);
		if (!(Status&DSBSTATUS_PLAYING)) {
			g_Sounds[i]->Release();
			g_Sounds[i]=0;
			g_Sounds3D[i]->Release();
			g_Sounds3D[i]=0;
		}
	}

	g_pDSListener->CommitDeferredSettings();

}


// added query for getting sound status...
// (as we need to know when it has ended)
// -jpk
bool CSoundLib::IsSoundPlaying(int soundHandle)
{
	if (g_Sounds[soundHandle] != 0)
	{
		return true;
	} else {
		return false;
	}
}


void CSoundLib::SetListener(float x,float y,float z,
							float vx,float vy,float vz,
							float dx,float dy,float dz,
							float ux,float uy,float uz)
{
	g_pDSListener->SetPosition(x,y,z,DS3D_DEFERRED);
	g_pDSListener->SetVelocity(vx,vy,vz,DS3D_DEFERRED);
	g_pDSListener->SetOrientation(dx,dy,dz,ux,uy,uz,DS3D_DEFERRED);
}

void CSoundLib::SetSound(int Sound,float x,float y,float z,float vx,float vy,float vz,int Volumed,DWORD Position)
{
	if (Sound<0) return;
	if (!g_Sounds[Sound]) return;

	g_Sounds3D[Sound]->SetPosition(x,y,z,DS3D_DEFERRED);
	g_Sounds3D[Sound]->SetVelocity(vx,vy,vz,DS3D_DEFERRED);
	g_Sounds[Sound]->SetVolume(Volumed);
	g_Sounds[Sound]->SetCurrentPosition(Position);
	
}

void CSoundLib::SetSoundVolume(int Sound,int Volumed)
{
	if (Sound<0) return;
	if (!g_Sounds[Sound]) return;

	g_Sounds[Sound]->SetVolume(Volumed);
}

void CSoundLib::GetSound(int Sound,float &x,float &y,float &z,float &vx,float &vy,float &vz,int &Volumed,DWORD &Position)
{
	if (Sound<0) return;
	if (!g_Sounds[Sound]) return;

	D3DVECTOR Pos;

	g_Sounds3D[Sound]->GetPosition(&Pos);

	x=Pos.x;
	y=Pos.y;
	z=Pos.z;

	g_Sounds3D[Sound]->GetVelocity(&Pos);

	vx=Pos.x;
	vy=Pos.y;
	vz=Pos.z;

	LONG Volume;

	g_Sounds[Sound]->GetVolume(&Volume);Volumed=Volume;

	g_Sounds[Sound]->GetCurrentPosition(&Position,0);
	
}

void CSoundLib::StopSound(int Sound)
{
	if (Sound<0) return;
	if (!g_Sounds[Sound]) return;

	g_Sounds[Sound]->Stop();
}

//-----------------------------------------------------------------------------
// Name: CSound::CSound()
// Desc: Constructs the class
//-----------------------------------------------------------------------------
CSound::CSound( LPDIRECTSOUNDBUFFER* apDSBuffer, DWORD dwDSBufferSize, 
                DWORD dwNumBuffers, CWaveFile* pWaveFile )
{
    DWORD i;

    m_apDSBuffer = new LPDIRECTSOUNDBUFFER[dwNumBuffers];
    for( i=0; i<dwNumBuffers; i++ )
        m_apDSBuffer[i] = apDSBuffer[i];

    m_dwDSBufferSize = dwDSBufferSize;
    m_dwNumBuffers   = dwNumBuffers;
    m_pWaveFile      = pWaveFile;
	BaseFrequency	 = pWaveFile->m_pwfx->nSamplesPerSec;

    FillBufferWithSound( m_apDSBuffer[0], FALSE );

    // Make DirectSound do pre-processing on sound effects
    for( i=0; i<dwNumBuffers; i++ )
        m_apDSBuffer[i]->SetCurrentPosition(0);
}

//-----------------------------------------------------------------------------
// Name: CSound::~CSound()
// Desc: Destroys the class
//-----------------------------------------------------------------------------
CSound::~CSound()
{
    for( DWORD i=0; i<m_dwNumBuffers; i++ )
    {
        SAFE_RELEASE( m_apDSBuffer[i] ); 
    }

    SLIB_SAFE_DELETE_ARRAY( m_apDSBuffer ); 
    SLIB_SAFE_DELETE( m_pWaveFile );
}

//-----------------------------------------------------------------------------
// Name: CSound::FillBufferWithSound()
// Desc: Fills a DirectSound buffer with a sound file 
//-----------------------------------------------------------------------------
HRESULT CSound::FillBufferWithSound( LPDIRECTSOUNDBUFFER pDSB, BOOL bRepeatWavIfBufferLarger )
{
    HRESULT hr; 
    VOID*   pDSLockedBuffer      = NULL; // Pointer to locked buffer memory
    DWORD   dwDSLockedBufferSize = 0;    // Size of the locked DirectSound buffer
    DWORD   dwWavDataRead        = 0;    // Amount of data read from the wav file 

    if( pDSB == NULL )
        return CO_E_NOTINITIALIZED;

    // Make sure we have focus, and we didn't just switch in from
    // an app which had a DirectSound device
    if( FAILED( hr = RestoreBuffer( pDSB, NULL ) ) ) 
        return S_FALSE;

    // Lock the buffer down
    if( FAILED( hr = pDSB->Lock( 0, m_dwDSBufferSize, 
                                 &pDSLockedBuffer, &dwDSLockedBufferSize, 
                                 NULL, NULL, 0L ) ) )
        return S_FALSE;

    // Reset the wave file to the beginning 
    m_pWaveFile->ResetFile();

    if( FAILED( hr = m_pWaveFile->Read( (BYTE*) pDSLockedBuffer,
                                        dwDSLockedBufferSize, 
                                        &dwWavDataRead ) ) )           
        return S_FALSE;

    if( dwWavDataRead == 0 )
    {
        // Wav is blank, so just fill with silence
        FillMemory( (BYTE*) pDSLockedBuffer, 
                    dwDSLockedBufferSize, 
                    (BYTE)(m_pWaveFile->m_pwfx->wBitsPerSample == 8 ? 128 : 0 ) );
    }
    else if( dwWavDataRead < dwDSLockedBufferSize )
    {
        // If the wav file was smaller than the DirectSound buffer, 
        // we need to fill the remainder of the buffer with data 
        if( bRepeatWavIfBufferLarger )
        {       
            // Reset the file and fill the buffer with wav data
            DWORD dwReadSoFar = dwWavDataRead;    // From previous call above.
            while( dwReadSoFar < dwDSLockedBufferSize )
            {  
                // This will keep reading in until the buffer is full 
                // for very short files
                if( FAILED( hr = m_pWaveFile->ResetFile() ) )
                    return S_FALSE;

                hr = m_pWaveFile->Read( (BYTE*)pDSLockedBuffer + dwReadSoFar,
                                        dwDSLockedBufferSize - dwReadSoFar,
                                        &dwWavDataRead );
                if( FAILED(hr) )
                    return S_FALSE;

                dwReadSoFar += dwWavDataRead;
            } 
        }
        else
        {
            // Don't repeat the wav file, just fill in silence 
            FillMemory( (BYTE*) pDSLockedBuffer + dwWavDataRead, 
                        dwDSLockedBufferSize - dwWavDataRead, 
                        (BYTE)(m_pWaveFile->m_pwfx->wBitsPerSample == 8 ? 128 : 0 ) );
        }
    }

    // Unlock the buffer, we don't need it anymore.
    pDSB->Unlock( pDSLockedBuffer, dwDSLockedBufferSize, NULL, 0 );

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CSound::RestoreBuffer()
// Desc: Restores the lost buffer. *pbWasRestored returns true if the buffer was 
//       restored.  It can also NULL if the information is not needed.
//-----------------------------------------------------------------------------
HRESULT CSound::RestoreBuffer( LPDIRECTSOUNDBUFFER pDSB, BOOL* pbWasRestored )
{
    HRESULT hr;

    if( pDSB == NULL )
        return CO_E_NOTINITIALIZED;
    if( pbWasRestored )
        *pbWasRestored = FALSE;

    DWORD dwStatus;
    if( FAILED( hr = pDSB->GetStatus( &dwStatus ) ) )
        return S_FALSE;

    if( dwStatus & DSBSTATUS_BUFFERLOST )
    {
        // Since the app could have just been activated, then
        // DirectSound may not be giving us control yet, so 
        // the restoring the buffer may fail.  
        // If it does, sleep until DirectSound gives us control.
        do 
        {
            hr = pDSB->Restore();
            if( hr == DSERR_BUFFERLOST )
                Sleep( 10 );
        }

        // IS THIS CORRECT? MSVC WARNS ABOUT IT!
        // SUPPOSED TO BE = OR ==? 
        // MAYBE SHOULD ADD () AROUND IT...
        // - jpkokkon
        while( hr = pDSB->Restore() );

        if( pbWasRestored != NULL )
            *pbWasRestored = true;

        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

//-----------------------------------------------------------------------------
// Name: CSound::GetFreeBuffer()
// Desc: Checks to see if a buffer is playing and returns true if it is.
//-----------------------------------------------------------------------------
LPDIRECTSOUNDBUFFER CSound::GetFreeBuffer()
{
    BOOL bIsPlaying = FALSE;

    if( m_apDSBuffer == NULL )
        return FALSE; 

    for( DWORD i=0; i<m_dwNumBuffers; i++ )
    {
        if( m_apDSBuffer[i] )
        {  
            DWORD dwStatus = 0;
            m_apDSBuffer[i]->GetStatus( &dwStatus );
            if ( ( dwStatus & DSBSTATUS_PLAYING ) == 0 )
                break;
        }
    }

    if( i != m_dwNumBuffers )
        return m_apDSBuffer[ i ];
    else
        return m_apDSBuffer[ rand() % m_dwNumBuffers ];
}

//-----------------------------------------------------------------------------
// Name: CSound::GetBuffer()
// Desc: 
//-----------------------------------------------------------------------------
LPDIRECTSOUNDBUFFER CSound::GetBuffer( DWORD dwIndex )
{
    if( m_apDSBuffer == NULL )
        return NULL;
    if( dwIndex >= m_dwNumBuffers )
        return NULL;

    return m_apDSBuffer[dwIndex];
}

//-----------------------------------------------------------------------------
// Name: CSound::Get3DBufferInterface()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CSound::Get3DBufferInterface( DWORD dwIndex, LPDIRECTSOUND3DBUFFER* ppDS3DBuffer )
{
    if( m_apDSBuffer == NULL )
        return CO_E_NOTINITIALIZED;
    if( dwIndex >= m_dwNumBuffers )
        return E_INVALIDARG;

    *ppDS3DBuffer = NULL;

    return m_apDSBuffer[dwIndex]->QueryInterface( IID_IDirectSound3DBuffer, 
                                                  (VOID**)ppDS3DBuffer );
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::CWaveFile()
// Desc: Constructs the class.  Call Open() to open a wave file for reading.  
//       Then call Read() as needed.  Calling the destructor or Close() 
//       will close the file.  
//-----------------------------------------------------------------------------
CWaveFile::CWaveFile()
{
    m_pwfx    = NULL;
    m_hmmio   = NULL;
    m_dwSize  = 0;
    m_bIsReadingFromMemory = FALSE;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::~CWaveFile()
// Desc: Destructs the class
//-----------------------------------------------------------------------------
CWaveFile::~CWaveFile()
{
    Close();

    if( !m_bIsReadingFromMemory )
        SLIB_SAFE_DELETE_ARRAY( m_pwfx );
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::Open()
// Desc: Opens a wave file for reading
//-----------------------------------------------------------------------------
HRESULT CWaveFile::Open( const char *strFileName, WAVEFORMATEX* pwfx, DWORD dwFlags )
{
    HRESULT hr;

    m_dwFlags = dwFlags;
    m_bIsReadingFromMemory = FALSE;

    if( m_dwFlags == WAVEFILE_READ )
    {
        if( strFileName == NULL )
            return E_INVALIDARG;
        SLIB_SAFE_DELETE_ARRAY( m_pwfx );

        m_hmmio = mmioOpen( const_cast<char *> (strFileName), NULL, MMIO_ALLOCBUF | MMIO_READ );

        if( NULL == m_hmmio )
        {
            HRSRC   hResInfo;
            HGLOBAL hResData;
            DWORD   dwSize;
            VOID*   pvRes;

            // Loading it as a file failed, so try it as a resource
            if( NULL == ( hResInfo = FindResource( NULL, strFileName, TEXT("WAVE") ) ) )
            {
                if( NULL == ( hResInfo = FindResource( NULL, strFileName, TEXT("WAV") ) ) )
                    return S_FALSE;
            }

            if( NULL == ( hResData = LoadResource( NULL, hResInfo ) ) )
                return S_FALSE;

            if( 0 == ( dwSize = SizeofResource( NULL, hResInfo ) ) ) 
                return S_FALSE;

            if( NULL == ( pvRes = LockResource( hResData ) ) )
                return S_FALSE;

            CHAR* pData = new CHAR[ dwSize ];
            memcpy( pData, pvRes, dwSize );

            MMIOINFO mmioInfo;
            ZeroMemory( &mmioInfo, sizeof(mmioInfo) );
            mmioInfo.fccIOProc = FOURCC_MEM;
            mmioInfo.cchBuffer = dwSize;
            mmioInfo.pchBuffer = (CHAR*) pData;

            m_hmmio = mmioOpen( NULL, &mmioInfo, MMIO_ALLOCBUF | MMIO_READ );
        }

        if( FAILED( hr = ReadMMIO() ) )
        {
            // ReadMMIO will fail if its an not a wave file
            mmioClose( m_hmmio, 0 );
            return S_FALSE;
        }

        if( FAILED( hr = ResetFile() ) )
            return S_FALSE;

        // After the reset, the size of the wav file is m_ck.cksize so store it now
        m_dwSize = m_ck.cksize;
    }
    else
    {
        m_hmmio = mmioOpen( const_cast<char *> (strFileName), NULL, MMIO_ALLOCBUF  | 
                                                  MMIO_READWRITE | 
                                                  MMIO_CREATE );
        if( NULL == m_hmmio )
            return S_FALSE;

        if( FAILED( hr = WriteMMIO( pwfx ) ) )
        {
            mmioClose( m_hmmio, 0 );
            return S_FALSE;
        }
                        
        if( FAILED( hr = ResetFile() ) )
            return S_FALSE;
    }

    return hr;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::OpenFromMemory()
// Desc: copy data to CWaveFile member variable from memory
//-----------------------------------------------------------------------------
HRESULT CWaveFile::OpenFromMemory( BYTE* pbData, ULONG ulDataSize, 
                                   WAVEFORMATEX* pwfx, DWORD dwFlags )
{
    m_pwfx       = pwfx;
    m_ulDataSize = ulDataSize;
    m_pbData     = pbData;
    m_pbDataCur  = m_pbData;
    m_bIsReadingFromMemory = true;
    
    if( dwFlags != WAVEFILE_READ )
        return E_NOTIMPL;       
    
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::ReadMMIO()
// Desc: Support function for reading from a multimedia I/O stream.
//       m_hmmio must be valid before calling.  This function uses it to
//       update m_ckRiff, and m_pwfx. 
//-----------------------------------------------------------------------------
HRESULT CWaveFile::ReadMMIO()
{
    MMCKINFO        ckIn;           // chunk info. for general use.
    PCMWAVEFORMAT   pcmWaveFormat;  // Temp PCM structure to load in.       

    m_pwfx = NULL;

    if( ( 0 != mmioDescend( m_hmmio, &m_ckRiff, NULL, 0 ) ) )
        return S_FALSE;

    // Check to make sure this is a valid wave file
    if( (m_ckRiff.ckid != FOURCC_RIFF) ||
        (m_ckRiff.fccType != mmioFOURCC('W', 'A', 'V', 'E') ) )
        return S_FALSE;

    // Search the input file for for the 'fmt ' chunk.
    ckIn.ckid = mmioFOURCC('f', 'm', 't', ' ');
    if( 0 != mmioDescend( m_hmmio, &ckIn, &m_ckRiff, MMIO_FINDCHUNK ) )
        return S_FALSE;

    // Expect the 'fmt' chunk to be at least as large as <PCMWAVEFORMAT>;
    // if there are extra parameters at the end, we'll ignore them
       if( ckIn.cksize < (LONG) sizeof(PCMWAVEFORMAT) )
           return S_FALSE;

    // Read the 'fmt ' chunk into <pcmWaveFormat>.
    if( mmioRead( m_hmmio, (HPSTR) &pcmWaveFormat, 
                  sizeof(pcmWaveFormat)) != sizeof(pcmWaveFormat) )
        return S_FALSE;

    // Allocate the waveformatex, but if its not pcm format, read the next
    // word, and thats how many extra bytes to allocate.
    if( pcmWaveFormat.wf.wFormatTag == WAVE_FORMAT_PCM )
    {
        m_pwfx = (WAVEFORMATEX*)(new CHAR[ sizeof(WAVEFORMATEX) ]);
        if( NULL == m_pwfx )
            return S_FALSE;

        // Copy the bytes from the pcm structure to the waveformatex structure
        memcpy( m_pwfx, &pcmWaveFormat, sizeof(pcmWaveFormat) );
        m_pwfx->cbSize = 0;
    }
    else
    {
        // Read in length of extra bytes.
        WORD cbExtraBytes = 0L;
        if( mmioRead( m_hmmio, (CHAR*)&cbExtraBytes, sizeof(WORD)) != sizeof(WORD) )
            return S_FALSE;

        m_pwfx = (WAVEFORMATEX*)(new CHAR[ sizeof(WAVEFORMATEX) + cbExtraBytes ]);
        if( NULL == m_pwfx )
            return S_FALSE;

        // Copy the bytes from the pcm structure to the waveformatex structure
        memcpy( m_pwfx, &pcmWaveFormat, sizeof(pcmWaveFormat) );
        m_pwfx->cbSize = cbExtraBytes;

        // Now, read those extra bytes into the structure, if cbExtraAlloc != 0.
        if( mmioRead( m_hmmio, (CHAR*)(((BYTE*)&(m_pwfx->cbSize))+sizeof(WORD)),
                      cbExtraBytes ) != cbExtraBytes )
        {
            SLIB_SAFE_DELETE( m_pwfx );
            return S_FALSE;
        }
    }

    // Ascend the input file out of the 'fmt ' chunk.
    if( 0 != mmioAscend( m_hmmio, &ckIn, 0 ) )
    {
        SLIB_SAFE_DELETE( m_pwfx );
        return S_FALSE;
    }

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::GetSize()
// Desc: Retuns the size of the read access wave file 
//-----------------------------------------------------------------------------
DWORD CWaveFile::GetSize()
{
    return m_dwSize;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::ResetFile()
// Desc: Resets the internal m_ck pointer so reading starts from the 
//       beginning of the file again 
//-----------------------------------------------------------------------------
HRESULT CWaveFile::ResetFile()
{
    if( m_bIsReadingFromMemory )
    {
        m_pbDataCur = m_pbData;
    }
    else 
    {
        if( m_hmmio == NULL )
            return CO_E_NOTINITIALIZED;

        if( m_dwFlags == WAVEFILE_READ )
        {
            // Seek to the data
            if( -1 == mmioSeek( m_hmmio, m_ckRiff.dwDataOffset + sizeof(FOURCC),
                            SEEK_SET ) )
                return S_FALSE;

            // Search the input file for the 'data' chunk.
            m_ck.ckid = mmioFOURCC('d', 'a', 't', 'a');
            if( 0 != mmioDescend( m_hmmio, &m_ck, &m_ckRiff, MMIO_FINDCHUNK ) )
              return S_FALSE;
        }
        else
        {
            // Create the 'data' chunk that holds the waveform samples.  
            m_ck.ckid = mmioFOURCC('d', 'a', 't', 'a');
            m_ck.cksize = 0;

            if( 0 != mmioCreateChunk( m_hmmio, &m_ck, 0 ) ) 
                return S_FALSE;

            if( 0 != mmioGetInfo( m_hmmio, &m_mmioinfoOut, 0 ) )
                return S_FALSE;
        }
    }
    
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::Read()
// Desc: Reads section of data from a wave file into pBuffer and returns 
//       how much read in pdwSizeRead, reading not more than dwSizeToRead.
//       This uses m_ck to determine where to start reading from.  So 
//       subsequent calls will be continue where the last left off unless 
//       Reset() is called.
//-----------------------------------------------------------------------------
HRESULT CWaveFile::Read( BYTE* pBuffer, DWORD dwSizeToRead, DWORD* pdwSizeRead )
{
    if( m_bIsReadingFromMemory )
    {
        if( m_pbDataCur == NULL )
            return CO_E_NOTINITIALIZED;
        if( pdwSizeRead != NULL )
            *pdwSizeRead = 0;

        if( (BYTE*)(m_pbDataCur + dwSizeToRead) > 
            (BYTE*)(m_pbData + m_ulDataSize) )
        {
            dwSizeToRead = m_ulDataSize - (DWORD)(m_pbDataCur - m_pbData);
        }
        
        CopyMemory( pBuffer, m_pbDataCur, dwSizeToRead );
        
        if( pdwSizeRead != NULL )
            *pdwSizeRead = dwSizeToRead;

        return S_OK;
    }
    else 
    {
        MMIOINFO mmioinfoIn; // current status of m_hmmio

        if( m_hmmio == NULL )
            return CO_E_NOTINITIALIZED;
        if( pBuffer == NULL || pdwSizeRead == NULL )
            return E_INVALIDARG;

        if( pdwSizeRead != NULL )
            *pdwSizeRead = 0;

        if( 0 != mmioGetInfo( m_hmmio, &mmioinfoIn, 0 ) )
            return S_FALSE;
                
        UINT cbDataIn = dwSizeToRead;
        if( cbDataIn > m_ck.cksize ) 
            cbDataIn = m_ck.cksize;       

        m_ck.cksize -= cbDataIn;
    
        for( DWORD cT = 0; cT < cbDataIn; cT++ )
        {
            // Copy the bytes from the io to the buffer.
            if( mmioinfoIn.pchNext == mmioinfoIn.pchEndRead )
            {
                if( 0 != mmioAdvance( m_hmmio, &mmioinfoIn, MMIO_READ ) )
                    return S_FALSE;

                if( mmioinfoIn.pchNext == mmioinfoIn.pchEndRead )
                    return S_FALSE;
            }

            // Actual copy.
            *((BYTE*)pBuffer+cT) = *((BYTE*)mmioinfoIn.pchNext);
            mmioinfoIn.pchNext++;
        }

        if( 0 != mmioSetInfo( m_hmmio, &mmioinfoIn, 0 ) )
            return S_FALSE;

        if( pdwSizeRead != NULL )
            *pdwSizeRead = cbDataIn;

        return S_OK;
    }
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::Close()
// Desc: Closes the wave file 
//-----------------------------------------------------------------------------
HRESULT CWaveFile::Close()
{
    if( m_dwFlags == WAVEFILE_READ )
    {
        mmioClose( m_hmmio, 0 );
        m_hmmio = NULL;
    }
    else
    {
        m_mmioinfoOut.dwFlags |= MMIO_DIRTY;

        if( m_hmmio == NULL )
            return CO_E_NOTINITIALIZED;

        if( 0 != mmioSetInfo( m_hmmio, &m_mmioinfoOut, 0 ) )
            return S_FALSE;
    
        // Ascend the output file out of the 'data' chunk -- this will cause
        // the chunk size of the 'data' chunk to be written.
        if( 0 != mmioAscend( m_hmmio, &m_ck, 0 ) )
            return S_FALSE;
    
        // Do this here instead...
        if( 0 != mmioAscend( m_hmmio, &m_ckRiff, 0 ) )
            return S_FALSE;
        
        mmioSeek( m_hmmio, 0, SEEK_SET );

        if( 0 != (INT)mmioDescend( m_hmmio, &m_ckRiff, NULL, 0 ) )
            return S_FALSE;
    
        m_ck.ckid = mmioFOURCC('f', 'a', 'c', 't');

        if( 0 == mmioDescend( m_hmmio, &m_ck, &m_ckRiff, MMIO_FINDCHUNK ) ) 
        {
            DWORD dwSamples = 0;
            mmioWrite( m_hmmio, (HPSTR)&dwSamples, sizeof(DWORD) );
            mmioAscend( m_hmmio, &m_ck, 0 ); 
        }
    
        // Ascend the output file out of the 'RIFF' chunk -- this will cause
        // the chunk size of the 'RIFF' chunk to be written.
        if( 0 != mmioAscend( m_hmmio, &m_ckRiff, 0 ) )
            return S_FALSE;
    
        mmioClose( m_hmmio, 0 );
        m_hmmio = NULL;
    }

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::WriteMMIO()
// Desc: Support function for reading from a multimedia I/O stream
//       pwfxDest is the WAVEFORMATEX for this new wave file.  
//       m_hmmio must be valid before calling.  This function uses it to
//       update m_ckRiff, and m_ck.  
//-----------------------------------------------------------------------------
HRESULT CWaveFile::WriteMMIO( WAVEFORMATEX *pwfxDest )
{
    DWORD    dwFactChunk; // Contains the actual fact chunk. Garbage until WaveCloseWriteFile.
    MMCKINFO ckOut1;
    
    dwFactChunk = (DWORD)-1;

    // Create the output file RIFF chunk of form type 'WAVE'.
    m_ckRiff.fccType = mmioFOURCC('W', 'A', 'V', 'E');       
    m_ckRiff.cksize = 0;

    if( 0 != mmioCreateChunk( m_hmmio, &m_ckRiff, MMIO_CREATERIFF ) )
        return S_FALSE;
    
    // We are now descended into the 'RIFF' chunk we just created.
    // Now create the 'fmt ' chunk. Since we know the size of this chunk,
    // specify it in the MMCKINFO structure so MMIO doesn't have to seek
    // back and set the chunk size after ascending from the chunk.
    m_ck.ckid = mmioFOURCC('f', 'm', 't', ' ');
    m_ck.cksize = sizeof(PCMWAVEFORMAT);   

    if( 0 != mmioCreateChunk( m_hmmio, &m_ck, 0 ) )
        return S_FALSE;
    
    // Write the PCMWAVEFORMAT structure to the 'fmt ' chunk if its that type. 
    if( pwfxDest->wFormatTag == WAVE_FORMAT_PCM )
    {
        if( mmioWrite( m_hmmio, (HPSTR) pwfxDest, 
                       sizeof(PCMWAVEFORMAT)) != sizeof(PCMWAVEFORMAT))
            return S_FALSE;
    }   
    else 
    {
        // Write the variable length size.
        if( (UINT)mmioWrite( m_hmmio, (HPSTR) pwfxDest, 
                             sizeof(*pwfxDest) + pwfxDest->cbSize ) != 
                             ( sizeof(*pwfxDest) + pwfxDest->cbSize ) )
            return S_FALSE;
    }  
    
    // Ascend out of the 'fmt ' chunk, back into the 'RIFF' chunk.
    if( 0 != mmioAscend( m_hmmio, &m_ck, 0 ) )
        return S_FALSE;
    
    // Now create the fact chunk, not required for PCM but nice to have.  This is filled
    // in when the close routine is called.
    ckOut1.ckid = mmioFOURCC('f', 'a', 'c', 't');
    ckOut1.cksize = 0;

    if( 0 != mmioCreateChunk( m_hmmio, &ckOut1, 0 ) )
        return S_FALSE;
    
    if( mmioWrite( m_hmmio, (HPSTR)&dwFactChunk, sizeof(dwFactChunk)) != 
                    sizeof(dwFactChunk) )
         return S_FALSE;
    
    // Now ascend out of the fact chunk...
    if( 0 != mmioAscend( m_hmmio, &ckOut1, 0 ) )
        return S_FALSE;
       
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::Write()
// Desc: Writes data to the open wave file
//-----------------------------------------------------------------------------
HRESULT CWaveFile::Write( UINT nSizeToWrite, BYTE* pbSrcData, UINT* pnSizeWrote )
{
    UINT cT;

    if( m_bIsReadingFromMemory )
        return E_NOTIMPL;
    if( m_hmmio == NULL )
        return CO_E_NOTINITIALIZED;
    if( pnSizeWrote == NULL || pbSrcData == NULL )
        return E_INVALIDARG;

    *pnSizeWrote = 0;
    
    for( cT = 0; cT < nSizeToWrite; cT++ )
    {       
        if( m_mmioinfoOut.pchNext == m_mmioinfoOut.pchEndWrite )
        {
            m_mmioinfoOut.dwFlags |= MMIO_DIRTY;
            if( 0 != mmioAdvance( m_hmmio, &m_mmioinfoOut, MMIO_WRITE ) )
                return S_FALSE;
        }

        *((BYTE*)m_mmioinfoOut.pchNext) = *((BYTE*)pbSrcData+cT);
        (BYTE*)m_mmioinfoOut.pchNext++;

        (*pnSizeWrote)++;
    }

    return S_OK;
}

//-----------------------------------------------------------------------------
// Stream object for playing any sound streams that DirectShow supports
// (MP3,WAV,AVI for example)
// Uses DirectShow
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// CSoundStream* CSoundLib::CreateMP3Stream(char *Filename)
// Creates mp3 stream from a file
//-----------------------------------------------------------------------------
CSoundStream* CSoundLib::CreateMP3Stream(const char *Filename)
{
	CSoundStream *Stream=new CSoundStream();
  
  // i dunno what this does, but does not seem to work without it ;)
  CoInitialize(NULL);

  HRESULT hr=CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&Stream->pGB);

  if(SUCCEEDED(hr)) 
  {
		WCHAR st[128];
		MultiByteToWideChar(CP_ACP, 0, Filename, -1, st, 128);
		
		// Have the graph construct its the appropriate graph automatically
		hr=Stream->pGB->RenderFile(st, NULL);
		
		// QueryInterface for DirectShow interfaces
		Stream->pGB->QueryInterface(IID_IMediaControl, (void **)&Stream->pMC);
		Stream->pGB->QueryInterface(IID_IMediaEventEx, (void **)&Stream->pME);
		Stream->pGB->QueryInterface(IID_IMediaSeeking, (void **)&Stream->pMS);
		
		// Query for audio interface
		Stream->pGB->QueryInterface(IID_IBasicAudio, (void **)&Stream->pBA);

    Stream->initOk = true;

    return Stream;
  } else {
    // changed to return null on failure 
    // (so we can detect it and not crash because of it)
    delete Stream;
    return NULL;
  }
}

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CSoundStream::CSoundStream()
{
  initOk = false;

	pGB=0;
	pMC=0;
	pME=0;
	pBA=0;
	pMS=0;
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
CSoundStream::~CSoundStream()
{
  if (initOk)
  {
    Stop();
  }

  SAFE_RELEASE(pME);
  SAFE_RELEASE(pMS);
  SAFE_RELEASE(pMC);
  SAFE_RELEASE(pBA);
  SAFE_RELEASE(pGB);
}

void CSoundStream::Play()
{
	pMC->Run();
}

void CSoundStream::Stop()
{
	LONGLONG pos = 0;
	pMC->Stop();
    pMS->SetPositions(&pos, AM_SEEKING_AbsolutePositioning ,NULL, AM_SEEKING_NoPositioning);
	pMC->Pause();
}

void CSoundStream::Pause()
{
	pMC->Stop();
	pMC->Pause();
}

void CSoundStream::SetVolume(int Volume)
{
	pBA->put_Volume(Volume);
}

int CSoundStream::IsComplete()
{
    LONG evCode, evParam1, evParam2;
    HRESULT hr=S_OK;

    while(SUCCEEDED(pME->GetEvent(&evCode, &evParam1, &evParam2, 0)))
    {
        // Spin through the events
        hr = pME->FreeEventParams(evCode, evParam1, evParam2);

        if(EC_COMPLETE == evCode)
        {
			return 1;
		}
	}
	return 0;
}

#elif SOUND_NULL

namespace sfx {


void SoundStream::setVolume(float value) {}
void SoundStream::play() {}
bool SoundStream::hasEnded() const { return true; }
void SoundStream::stop() {}
void SoundLib::stopSound(int sound) {}
void SoundLib::setSound3D(int sound, const VC3 &position, const VC3 &velocity) {}
void SoundLib::setSoundVolume(int sound, float value) {}
bool SoundLib::isSoundPlaying(int sound) const { return false; }
SoundStream::~SoundStream() {}
void SoundStream::setBaseVolume(float value) {}
SoundStream *SoundLib::createStream(const char *file) { return NULL; }
void SoundLib::setSoundLoop(int sound, bool loop) {}
void SoundLib::setSoundPaused(int sound, bool pause) {}
int SoundLib::createSound(Sound *sound, int priority) { return -1; }
void SoundLib::setSoundArea(const std::string &name) {}
void SoundLib::update() {}
void SoundStream::setLooping(bool loop) {}
Sound *SoundLib::loadSample(const char *file) { return NULL; }
void SoundLib::playSound(int sound) {}
void SoundLib::setSoundFrequency(int sound, int value) {}
void SoundLib::setFrequencyFactor(float scalar) {}
int SoundLib::getSoundFrequency(int sound) const { return 0; }
void SoundLib::setListener(const VC3 &position, const VC3 &velocity, const VC3 &forwardDirection, const VC3 &upDirection) {}
Sound::~Sound() {}
SoundLib::~SoundLib() {}
void SoundStream::setPanning(float value) {}
bool SoundLib::initialize() { return true; }
void SoundLib::setAcceleration(bool useHW, bool useEax_, int minHardwareChannels_, int maxHardwareChannels_) {}
void SoundLib::setProperties(int mixrate_, int softwareChannels_) {}
int Sound::getLength() const { return 1; }
SoundLib::SoundLib() {}


}
#else // fmod

#include "SoundLib.h"
#include <assert.h>
#include <string>
#include <fmod.h>
#include "../util/Debug_MemoryManager.h"
#include "../system/Logger.h"
#include "../editor/parser.h"
#include "../editor/string_conversions.h"
#include <boost/lexical_cast.hpp>
//#include <fstream>
#include "../filesystem/file_package_manager.h"
#include "../filesystem/input_stream.h"

using namespace std;
using namespace boost;
using namespace frozenbyte::editor;
using namespace frozenbyte;

namespace sfx {

/*
  Sound
*/

void Sound::updateProperties()
{
	assert(sample);
	//FSOUND_Sample_SetDefaults(sample, -1, volume, -1, priority);
}

Sound::Sound(const char *file, int flags)
:	sample(0),
	volume(255),
	priority(0),
	length(0),
	fileNotFound(false)
{
	if(!file)
		return;

	filesystem::InputStream stream = filesystem::FilePackageManager::getInstance().getFile(file);
	if(stream.isEof())
	{
		fileNotFound = true;
		return;
	}

	//sample = FSOUND_Sample_Load(FSOUND_UNMANAGED, file, flags, 0, 0);
	vector<char> buffer(stream.getSize());
	stream.read(&buffer[0], buffer.size());
	sample = FSOUND_Sample_Load(FSOUND_UNMANAGED, &buffer[0], flags | FSOUND_LOADMEMORY, 0, buffer.size());

	if(sample)
	{
		setDefaultVolume(255);
		setPriority(1);

		int frequency = 0;
		FSOUND_Sample_GetDefaults(sample, &frequency, 0, 0, 0);
		//length = frequency * FSOUND_Sample_GetLength(sample);
		length = 1000 * FSOUND_Sample_GetLength(sample) / frequency;

		filename = file;
	}
}

Sound::~Sound()
{
	if(sample)
		FSOUND_Sample_Free(sample);
}

void Sound::setDefaultVolume(float value)
{
	assert(sample);

	volume = int(value * 255.f);
	if(volume < 0)
		volume = 0;
	if(volume > 255)
		volume = 255;

	updateProperties();
}

void Sound::setPriority(int value)
{
	assert(sample);
	priority = value;

	updateProperties();
}

int Sound::getLength() const
{
	return length;
}

const std::string &Sound::getFileName() const
{
	return filename;
}

/*
  SoundStream
*/

SoundStream::SoundStream(const char *file)
:	stream(0),
	channel(-1),
	baseVolume(1.f),
	volume(1.f)
{
	filename = file;

	filesystem::InputStream fileStream = filesystem::FilePackageManager::getInstance().getFile(file);
	if(fileStream.isEof())
		return;

	//stream = FSOUND_Stream_Open(file, 0, 0, 0);

	buffer.reset(new char[fileStream.getSize()]);
	fileStream.read(buffer.get(), fileStream.getSize());
	stream = FSOUND_Stream_Open(buffer.get(), FSOUND_LOADMEMORY, 0, fileStream.getSize());

	if(stream)
		channel = FSOUND_Stream_PlayEx(FSOUND_FREE, stream, 0, TRUE);

	type = 0;
}

SoundStream::~SoundStream()
{
	if(stream)
		FSOUND_Stream_Close(stream);
}

void SoundStream::setBaseVolume(float value)
{
	baseVolume = value;

	int vol = int(baseVolume * volume * 255.f);
	if(vol < 0)
		vol = 0;
	if(vol > 255)
		vol = 255;

	if(channel >= 0)
		FSOUND_SetVolume(channel, vol);
}

void SoundStream::setVolume(float value)
{
	volume = value;

	int vol = int(baseVolume * volume * 255.f);
	if(vol < 0)
		vol = 0;
	if(vol > 255)
		vol = 255;

	if(channel >= 0)
		FSOUND_SetVolume(channel, vol);
}

void SoundStream::setPanning(float value)
{
	int panning = int(value * 255.f);
	if(panning < 0)
		panning = 0;
	if(panning > 255)
		panning = 255;

	if(channel >= 0)
		FSOUND_SetPan(channel, panning);
}

void SoundStream::setLooping(bool loop)
{
	if(stream)
	{
		if(loop)
			FSOUND_Stream_SetMode(stream, FSOUND_LOOP_NORMAL);
		else
			FSOUND_Stream_SetMode(stream, FSOUND_LOOP_OFF);
	}
}

void SoundStream::play()
{
	if(channel >= 0)
		FSOUND_SetPaused(channel, false);
}

void SoundStream::pause()
{
	if(channel >= 0)
		FSOUND_SetPaused(channel, TRUE);
}

void SoundStream::stop()
{
	//FSOUND_Stream_Stop
	if(channel >= 0)
	{
		FSOUND_SetPaused(channel, TRUE);
		FSOUND_Stream_SetTime(stream, 0);
	}
}

bool SoundStream::hasEnded() const
{
	if(!stream || channel < 0)
		return true;

	//if(int(FSOUND_Stream_GetPosition(stream)) >= FSOUND_Stream_GetLength(stream))
	//	return true;
	//return false;

	if(FSOUND_IsPlaying(channel))
		return false;

	return true;
}

const std::string &SoundStream::getFileName() const
{
	return filename;
}

/*
  SoundLib
*/

SoundLib::SoundLib()
:	initialized(false),
	useHardware(false),
	useEax(false),
	mixrate(44100),
	softwareChannels(32),
	minHardwareChannels(16),
	maxHardwareChannels(32),
	speakerType(StereoSpeakers),
	frequencyFactor(1.f)
{
}

SoundLib::~SoundLib()
{
	if(initialized)
		FSOUND_Close();
}

void SoundLib::setProperties(int mixrate_, int softwareChannels_)
{
	mixrate = mixrate_;
	softwareChannels = softwareChannels_;
}

void SoundLib::setAcceleration(bool useHW, bool useEax_, int minHardwareChannels_, int maxHardwareChannels_)
{
	useHardware = useHW;
	useEax = useEax_;
	minHardwareChannels = minHardwareChannels_;
	maxHardwareChannels = maxHardwareChannels_;
}

void SoundLib::setSpeakers(SpeakerType speakerType_)
{
	speakerType = speakerType_;
}

bool SoundLib::initialize()
{
	assert(!initialized);

	//if(useEax && useHardware)
	{
#ifdef _WIN32
		FSOUND_SetOutput(FSOUND_OUTPUT_DSOUND);
#else
		FSOUND_SetOutput(FSOUND_OUTPUT_ALSA);
#endif

		int driverAmount = FSOUND_GetNumDrivers();
		for(int i = 0; i < driverAmount; ++i)
		{
			unsigned int caps = 0;
			bool eax = false;
			FSOUND_GetDriverCaps(i, &caps);
			{
				if(caps & FSOUND_CAPS_EAX2)
					eax = true;
				if(caps & FSOUND_CAPS_EAX3)
					eax = true;
			}

			string foo = "Driver ";
			foo += lexical_cast<string> (i);
			foo += "(";

			string name = "Null";
			const char *ptr = FSOUND_GetDriverName(i);
			if(ptr)
				name = ptr;

			foo += name;
			foo += ")";
			foo += " -- ";

			if(eax)
				foo += "supports EAX";
			else
				foo += "does NOT support EAX";

			Logger::getInstance()->debug(foo.c_str());
		}

#ifdef _WIN32
		FSOUND_SetOutput(-1);
#else
		FSOUND_SetOutput(FSOUND_OUTPUT_OSS);
#endif
	}

	if(!useHardware)
		maxHardwareChannels = 0;

	FSOUND_SetMinHardwareChannels(minHardwareChannels);
	FSOUND_SetMaxHardwareChannels(maxHardwareChannels);
	FSOUND_SetSpeakerMode(speakerType);

	char result = FSOUND_Init(mixrate, softwareChannels, 0);
	if(result)
	{
		initialized = true;
		FSOUND_3D_SetRolloffFactor(1);
		FSOUND_3D_SetRolloffFactor(0.f);

		int hwchans = 0;
		FSOUND_GetNumHWChannels(0, &hwchans, 0);
		string foo = "Hardware sound channels initialized: ";
		foo += lexical_cast<string> (hwchans);
		Logger::getInstance()->debug(foo.c_str());

		if(hwchans && useEax)
		{
			int output = FSOUND_GetOutput();
			if(output != FSOUND_OUTPUT_DSOUND)
				Logger::getInstance()->debug("Does not use DSOUND!");

			int driver = FSOUND_GetDriver();
			string foo = "Selected sound driver ";
			foo += lexical_cast<string> (driver);
			Logger::getInstance()->debug(foo.c_str());

			{
				EditorParser parser;
#ifdef LEGACY_FILES
				filesystem::InputStream env_file = filesystem::FilePackageManager::getInstance().getFile("Data/Ambient/environment.txt");
#else
				filesystem::InputStream env_file = filesystem::FilePackageManager::getInstance().getFile("data/audio/environment/environment.txt");
#endif
				env_file >> parser;

				ParserGroup &root = parser.getGlobals();
				for(int i = 0; i < root.getSubGroupAmount(); ++i)
				{
					const std::string &name = root.getSubGroupName(i);
					const ParserGroup &group = root.getSubGroup(name);

					FSOUND_REVERB_PROPERTIES p = FSOUND_PRESET_OFF;
					p.Environment = convertFromString<int> (group.getValue("environment"), 0);
					p.EnvSize = convertFromString<float> (group.getValue("env_size"), 7.5f);
					p.EnvDiffusion = convertFromString<float> (group.getValue("env_diffusion"), 1.f);
					p.Room = convertFromString<int> (group.getValue("room"), -1000);
					p.RoomHF = convertFromString<int> (group.getValue("room_hf"), -100);
					p.RoomLF = convertFromString<int> (group.getValue("room_lf"), 0);
					p.DecayTime = convertFromString<float> (group.getValue("decay_time"), 1.49f);
					p.Reflections = convertFromString<int> (group.getValue("reflections"), -2602);
					p.ReflectionsDelay = convertFromString<float> (group.getValue("reflections_delay"), 0.007f);
					p.Reverb = convertFromString<int> (group.getValue("reverb"), 200);
					p.ReverbDelay = convertFromString<float> (group.getValue("reverb_delay"), 0.011f);
					p.EchoTime = convertFromString<float> (group.getValue("echo_time"), 0.25f);
					p.EchoDepth = convertFromString<float> (group.getValue("echo_depth"), 0.f);
					p.ModulationTime = convertFromString<float> (group.getValue("modulation_time"), 0.25f);
					p.ModulationDepth = convertFromString<float> (group.getValue("modulation_depth"), 0.f);
					p.AirAbsorptionHF = convertFromString<float> (group.getValue("air_absorption_hf"), -5.0f);
					p.HFReference = convertFromString<float> (group.getValue("hf_reference"), 5000.f);
					p.LFReference = convertFromString<float> (group.getValue("hf_reference"), 250.f);
					p.RoomRolloffFactor = convertFromString<float> (group.getValue("room_rolloff_factor"), 0.f);
					p.Diffusion = convertFromString<float> (group.getValue("diffusion"), 100.f);
					p.Density = convertFromString<float> (group.getValue("density"), 100.f);

					soundAreas[name] = p;
					areaAlias[name] = name;
				}
			}

			{
				EditorParser parser;
#ifdef LEGACY_FILES
				filesystem::InputStream alias_file = filesystem::FilePackageManager::getInstance().getFile("Data/Ambient/environment_aliases.txt");
#else
				filesystem::InputStream alias_file = filesystem::FilePackageManager::getInstance().getFile("data/audio/environment/environment_aliases.txt");
#endif
				alias_file >> parser;

				ParserGroup &root = parser.getGlobals();
				for(int i = 0; i < root.getValueAmount(); ++i)
				{
					const std::string &key = root.getValueKey(i);
					const std::string &value = root.getValue(key);

					areaAlias[key] = value;
				}
			}
		}
	}

	return initialized;
}

void SoundLib::setFrequencyFactor(float scalar)
{	
	frequencyFactor = scalar;
}

Sound *SoundLib::loadSample(const char *file)
{
	if(!initialized)
		return 0;

	if (file == NULL)
	{
		string message = "SoundLib::loadSample - Null filename parameter.";
		Logger::getInstance()->warning(message.c_str());
		return 0;
	}

	// failsafe check for fmod memory leak on failure.
	// (without this, the game will choke on some extensive leaks with bad (stereo?) ambient sounds)
	static std::string lastFailedSampleLoad;
	if (!lastFailedSampleLoad.empty())
	{
		if (lastFailedSampleLoad == file)
		{
			string message = "SoundLib::loadSample - Failsafe, this sample needs to be fixed: ";
			message += file;
			Logger::getInstance()->error(message.c_str());

#ifndef PROJECT_VIEWER
			if (isThisDeveloper())
			{
				// you ain't gonna get past this just ignoring it! --jpk :)

				Logger::getInstance()->error("SoundLib::loadSample - DON'T IGNORE THIS - FIX IT!!!");

				// this should get their attention...
				abort();
			}
#endif

			return 0;
		}
	}

	int flags = FSOUND_LOOP_OFF;
	if(useHardware)
		flags |= FSOUND_HW3D;

	// test
	flags |= FSOUND_MONO;

	Sound *result = new Sound(file, flags);
	if(result->sample)
	{
		lastFailedSampleLoad = "";
		return result;
	}

	if(result->fileNotFound)
	{
#ifndef PROJECT_VIEWER
		if (game::SimpleOptions::getBool(DH_OPT_B_SOUND_MISSING_WARNING))
		{
			string message = "SoundLib::loadSample - Sound sample not found: ";
			message += file;
			Logger::getInstance()->warning(message.c_str());
		}
#endif
	} else {
		lastFailedSampleLoad = file;

		string message = "SoundLib::loadSample - Sound sample loading failure: ";
		message += file;
		Logger::getInstance()->warning(message.c_str());
	}

	delete result;
	return 0;
}

SoundStream *SoundLib::createStream(const char *file)
{
	if(!initialized)
		return 0;

	SoundStream *result = new SoundStream(file);
	if(result->stream && result->channel >= 0)
		return result;

//	string message = "Music stream not found: ";
//	message += file;
//	Logger::getInstance()->warning(message.c_str());

	delete result;
	return 0;
}

int SoundLib::createSound(Sound *sound, int priority)
{
	if(!sound)
		return -1;

	int id = FSOUND_PlaySoundEx(FSOUND_FREE, sound->sample, 0, TRUE);
	if(id >= 0)
		FSOUND_SetPriority(id, priority);

	return id;
}

void SoundLib::playSound(int sound)
{
	if(sound < 0)
	{
		assert(!"Invalid sound handle");
		return;
	}

	if(FSOUND_GetPaused(sound) == TRUE)
	{
		float frequency = getSoundFrequency(sound) * frequencyFactor;
		setSoundFrequency(sound, int(frequency + .5f));

		FSOUND_SetPaused(sound, false);
	}
}

void SoundLib::setSoundLoop(int sound, bool loop)
{
	if(sound < 0)
	{
		assert(!"Invalid sound handle");
		return;
	}

	int flag = loop ? FSOUND_LOOP_NORMAL : FSOUND_LOOP_OFF;
	FSOUND_SetLoopMode(sound, flag);
}

void SoundLib::setSoundPaused(int sound, bool pause)
{
	if(sound < 0)
	{
		assert(!"Invalid sound handle");
		return;
	}

	FSOUND_SetPaused(sound, pause ? TRUE : FALSE);
}

void SoundLib::setSoundVolume(int sound, float value)
{
	if(sound < 0)
	{
		//assert(!"Invalid sound handle");
		return;
	}

	int volume = int(value * 255.f);
	if(volume < 0)
		volume = 0;
	if(volume > 255)
		volume = 255;

	FSOUND_SetVolume(sound, volume);
}

void SoundLib::setSoundFrequency(int sound, int value)
{
	if(sound < 0)
	{
		assert(!"Invalid sound handle");
		return;
	}

	FSOUND_SetFrequency(sound, value);
}

void SoundLib::setSound3D(int sound, const VC3 &position, const VC3 &velocity)
{
	if(sound < 0)
	{
		assert(!"Invalid sound handle");
		return;
	}

	float p[3] = { position.x, position.y, position.z };
	float v[3] = { 0 }; //{ velocity.x, velocity.y, velocity.z };

	FSOUND_3D_SetAttributes(sound, p, v);
}

void SoundLib::stopSound(int sound)
{
	if(sound < 0)
	{
		assert(!"Invalid sound handle");
		return;
	}

	FSOUND_StopSound(sound);
}

float SoundLib::getSoundVolume(int sound) const
{
	if(sound < 0)
	{
		assert(!"Invalid sound handle");
		return 0;
	}

	int volume = FSOUND_GetVolume(sound);
	return volume / 255.f;
}

int SoundLib::getSoundFrequency(int sound) const
{
	if(sound < 0)
	{
		assert(!"Invalid sound handle");
		return 0;
	}

	return FSOUND_GetFrequency(sound);
}

int SoundLib::getSoundTime(int sound) const
{
	if(sound < 0)
	{
		assert(!"Invalid sound handle");
		return 0;
	}

	int freq = FSOUND_GetFrequency(sound);
	int pos = FSOUND_GetCurrentPosition(sound);

	Uint64 temp = pos * 1000;
	temp /= freq;

	return int(temp);
}

bool SoundLib::isSoundPlaying(int sound) const
{
	if(sound < 0)
	{
		//assert(!"Invalid sound handle");
		return false;
	}

	if(FSOUND_IsPlaying(sound))
		return true;

	return false;
}

void SoundLib::setGlobalVolume(float value)
{
	int volume = int(value * 255.f);
	if(volume < 0)
		volume = 0;
	if(volume > 255)
		volume = 255;

	FSOUND_SetSFXMasterVolume(volume);
}

void SoundLib::setListener(const VC3 &position, const VC3 &velocity, const VC3 &forwardDirection, const VC3 &upDirection)
{
	float p[3] = { position.x, position.y, position.z };
	float v[3] = { 0 }; //{ velocity.x, velocity.y, velocity.z };

	FSOUND_3D_Listener_SetAttributes(p, v, 
		forwardDirection.x, forwardDirection.y, forwardDirection.z,
		upDirection.x, upDirection.y, upDirection.z);
}

void SoundLib::setSoundArea(const std::string &name)
{
	if(!useEax)
		return;

	const std::string &alias = areaAlias[name];
	map<string, FSOUND_REVERB_PROPERTIES>::iterator it = soundAreas.find(alias);
	if(it == soundAreas.end())
	{
		string foo = "Sound reverb area not found: ";
		foo += name;

		Logger::getInstance()->warning(foo.c_str());
		return;
	}

	FSOUND_Reverb_SetProperties(&it->second);
}

void SoundLib::update()
{
	FSOUND_Update();
}

} // sfx

#endif
