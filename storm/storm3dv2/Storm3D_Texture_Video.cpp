// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d.h"
#include "storm3d_texture.h"
#include <vfw.h>		// Video for Windows header (to load AVIs)

#include "../../util/Debug_MemoryManager.h"


namespace {
	int id;
}

#ifndef __GNUC__

//------------------------------------------------------------------
// Storm3D_Texture_Video::Storm3D_Texture_Video
//------------------------------------------------------------------
Storm3D_Texture_Video::Storm3D_Texture_Video(Storm3D *s2,const char *_filename,DWORD texloadcaps) :
	Storm3D_Texture(s2),
	frames(NULL),
	frame_amount(1),
	frame(0),
	framechangetime(0),
	framechangecounter(0),
	loop_params(VIDEOLOOP_DEFAULT),
	last_time(timeGetTime())
{
	// Create filename
	filename=new char[strlen(_filename)+1];
	strcpy(filename,_filename);

	// Get frame_amount etc. from AVI file
	ReadAVIVideoInfo();

	// Allocate memory for frame array
	frames=new LPDIRECT3DTEXTURE9[frame_amount];

	// Create textures for videoframes
	for (int i=0;i<frame_amount;i++)
	{
		// Create (empty) texture (without mipmaps)
		// Prefer 16bit textures in (565 format) if possible
		D3DXCreateTexture(Storm3D2->GetD3DDevice(),width,height,D3DX_DEFAULT,//D3DX_DEFAULT,
			0,D3DFMT_R5G6B5,D3DPOOL_MANAGED,&frames[i]);
	}

	// Get first frame's textureformat:
	// 3d-card may not support 565, and DX can change format if that happens.
    D3DSURFACE_DESC ddsd;
    frames[0]->GetLevelDesc(0,&ddsd);
    dx_texformat=ddsd.Format;

	// Fill textures
	LoadAVIVideoFrames();

	for(int i=0;i<frame_amount;i++)
	{
		//D3DXCreateTexture(Storm3D2->GetD3DDevice(),width,height,1,//D3DX_DEFAULT,
		//	0,D3DFMT_R5G6B5,D3DPOOL_MANAGED,&frames[i]);

		IDirect3DBaseTexture9 *base = frames[i];
		D3DXFilterTexture(base, 0, D3DX_DEFAULT, D3DX_DEFAULT);
		frames[i]->AddDirtyRect(0);
	}
	
}



//------------------------------------------------------------------
// Storm3D_Texture_Video::~Storm3D_Texture_Video
//------------------------------------------------------------------
Storm3D_Texture_Video::~Storm3D_Texture_Video()
{
	// Release frames
	for (int i=0;i<frame_amount;i++) SAFE_RELEASE(frames[i]);

	// Release frame array
	delete[] frames;

	// Set DX8-handle to NULL (otherwise texture tries to delete it, and programs crashes)
	dx_handle=NULL;
}



//------------------------------------------------------------------
// Storm3D_Texture_Video::AnimateVideo
//------------------------------------------------------------------
void Storm3D_Texture_Video::AnimateVideo()
{
	// Calculate time difference
	//static DWORD last_time=timeGetTime();
	DWORD time_now=timeGetTime();
	DWORD time_dif=time_now-last_time;
  // added use of timing factor
  // --jpk
	//last_time=time_now;
  if (this->Storm3D2->timeFactor != 1.0f)
  {
    // FIXME: may have a small error on some values
    // should work just fine for factor values like 0.5 though.
    time_dif = (int)(float(time_dif) * this->Storm3D2->timeFactor);
    last_time+=(int)(float(time_dif) / this->Storm3D2->timeFactor);
  } else {
    last_time+=time_dif;
  }

	// Increase counter
	framechangecounter+=time_dif;

	// Change frame if needed
	if (framechangetime>0)
	{
		while (framechangecounter>framechangetime)
		{
			// Decrease counter
			framechangecounter-=framechangetime;

			// Add frame
			frame++;
			if (frame>=frame_amount)
			{
				switch (loop_params)
				{
					case VIDEOLOOP_DEFAULT: 
						frame=0;
						dx_handle=frames[frame];
						break;

					case VIDEOLOOP_PINGPONG:
					case VIDEOLOOP_PP_STOP_AT_BEGINNING:
						frame=frame_amount-1;
						framechangetime=-framechangetime;
						framechangecounter=0;
						dx_handle=frames[frame];
						return;

					case VIDEOLOOP_STOP_AT_END:
						frame=frame_amount-1;
						framechangetime=0;
						framechangecounter=0;
						dx_handle=frames[frame];
						return;
				}
			}
			dx_handle=frames[frame];
		}
	}
	else
	if (framechangetime<0)
	{
		while (framechangecounter>-framechangetime)
		{
			// Decrease counter
			framechangecounter+=framechangetime;

			// Decrease frame
			frame--;
			if (frame<0)
			{
				switch (loop_params)
				{
					case VIDEOLOOP_DEFAULT: 
						frame=frame_amount-1;
						dx_handle=frames[frame];
						break;

					case VIDEOLOOP_PINGPONG:
						frame=0;
						framechangetime=-framechangetime;
						framechangecounter=0;
						dx_handle=frames[frame];
						return;

					case VIDEOLOOP_STOP_AT_END:
					case VIDEOLOOP_PP_STOP_AT_BEGINNING:
						frame=0;
						framechangetime=0;
						framechangecounter=0;
						dx_handle=frames[frame];
						return;
				}
			}
			dx_handle=frames[frame];
		}
	}
}



//------------------------------------------------------------------
// Storm3D_Texture_Video::VideoSetFrame
//------------------------------------------------------------------
void Storm3D_Texture_Video::VideoSetFrame(int num)
{
	// Test param
	if (num<0) num=0;
	else if (num>=frame_amount) num=frame_amount-1;

	// Set frame
	frame=num;
	dx_handle=frames[frame];
}



//------------------------------------------------------------------
// Storm3D_Texture_Video::VideoSetFrameChangeSpeed
//------------------------------------------------------------------
void Storm3D_Texture_Video::VideoSetFrameChangeSpeed(int millisecs)
{
	framechangetime=millisecs;
}



//------------------------------------------------------------------
// Storm3D_Texture_Video::VideoSetLoopingParameters
//------------------------------------------------------------------
void Storm3D_Texture_Video::VideoSetLoopingParameters(VIDEOLOOP params)
{
	loop_params=params;
}



//------------------------------------------------------------------
// Storm3D_Texture_Video::VideoGetFrameAmount
//------------------------------------------------------------------
int Storm3D_Texture_Video::VideoGetFrameAmount()
{
	return frame_amount;
}



//------------------------------------------------------------------
// Storm3D_Texture_Video::VideoGetCurrentFrame
//------------------------------------------------------------------
int Storm3D_Texture_Video::VideoGetCurrentFrame()
{
	return frame;
}



//------------------------------------------------------------------
// Storm3D_Texture_Video::ReadAVIVideoInfo
//------------------------------------------------------------------
void Storm3D_Texture_Video::ReadAVIVideoInfo()
{
	PAVISTREAM    AVIStream;
	PGETFRAME     AVIFrame;
	AVISTREAMINFO AVIInfo;

	// Init AVI lib
    AVIFileInit();

	// Open file
	if(AVIStreamOpenFromFile(&AVIStream,filename,streamtypeVIDEO,0,OF_READ,NULL) != 0)
		return;

	// Get info
    AVIFrame=AVIStreamGetFrameOpen(AVIStream,NULL);
    AVIStreamInfo(AVIStream,&AVIInfo,sizeof(AVISTREAMINFO));

	// Get first frame
    BITMAPINFO* pbmi;
    pbmi=(BITMAPINFO*)AVIStreamGetFrame(AVIFrame,0);

	// Get format/size from first frame
	bpp=pbmi->bmiHeader.biBitCount;
	width=pbmi->bmiHeader.biWidth;
	height=pbmi->bmiHeader.biHeight;

	// Get video lenght
	frame_amount=AVIInfo.dwLength;

    // AVI lib exit
    AVIFileExit();
}



//------------------------------------------------------------------
// Storm3D_Texture_Video::LoadAVIVideoFrames
//------------------------------------------------------------------
void Storm3D_Texture_Video::LoadAVIVideoFrames()
{
	PAVISTREAM    AVIStream;
	PGETFRAME     AVIFrame;
	AVISTREAMINFO AVIInfo;

	// Init AVI lib
    AVIFileInit();

	// Open file
	if(AVIStreamOpenFromFile(&AVIStream,filename,streamtypeVIDEO,0,OF_READ,NULL) != 0)
		return;

	// Get info
    AVIFrame=AVIStreamGetFrameOpen(AVIStream,NULL);
    AVIStreamInfo(AVIStream,&AVIInfo,sizeof(AVISTREAMINFO));

	// Get first frame
    BITMAPINFO* pbmi;
    pbmi=(BITMAPINFO*)AVIStreamGetFrame(AVIFrame,0);

	// Animation parameters
	frame=0;
	dx_handle=frames[frame];
	framechangecounter=0;
	framechangetime=(int)(1000.0f/(((float)AVIInfo.dwRate)/(float)AVIInfo.dwScale));

	// Loop through AVI's frames and convert them into textures
	for (UINT fra=0;fra<AVIInfo.dwLength;fra++)
	{
		// Get the AVI frame
		BITMAPINFO* pbmi;
		pbmi=(BITMAPINFO*)AVIStreamGetFrame(AVIFrame,fra);

		// Lock the DX8 texture
		D3DLOCKED_RECT lrect;
		frames[fra]->LockRect(0,&lrect,NULL,0);
		BYTE *datapointer=(BYTE*)lrect.pBits;

		// Fill depending on DX8 texture format
		if (dx_texformat==D3DFMT_R5G6B5)	// 565 format (most common)
		{
			// 16 bit AVI
			if (pbmi->bmiHeader.biBitCount==16)
			{
				//WORD* pSrc=(WORD*)(sizeof(BITMAPINFO)+(BYTE*)pbmi);
				WORD* pSrc=(WORD*)(pbmi->bmiColors);
				WORD* pDest=(WORD*)datapointer;

				// Fill upside-down, because AVI is saved so (microsoft rulez;)
				pDest+=pbmi->bmiHeader.biWidth*(pbmi->bmiHeader.biHeight-1);
				int dualrow=pbmi->bmiHeader.biWidth*2;

				for(int yy=0;yy<pbmi->bmiHeader.biHeight;yy++)
				{
					for(int xx=0;xx<pbmi->bmiHeader.biWidth;xx++)
					{
						WORD color=*pSrc++;
						*pDest++=((color&0x1F)|((color&0xFFE0)<<1));
						//*pDest++ = *pSrc++;
					}
					pDest-=dualrow;
				}
			}

			// 24 bit AVI
			if (pbmi->bmiHeader.biBitCount==24)
			{
				//BYTE* pSrc=(BYTE*)(sizeof(BITMAPINFO)+(BYTE*)pbmi);
				BYTE* pSrc=(BYTE*)(pbmi->bmiColors);
				WORD* pDest=(WORD*)datapointer;
				
				// Fill upside-down, because AVI is saved so (microsoft rulez;)
				pDest+=pbmi->bmiHeader.biWidth*(pbmi->bmiHeader.biHeight-1);
				int dualrow=pbmi->bmiHeader.biWidth*2;

				for(int yy=0;yy<pbmi->bmiHeader.biHeight;yy++)
				{
					for(int xx=0;xx<pbmi->bmiHeader.biWidth;xx++)
					{
						//BYTE g=(*pSrc++)>>2;
						//BYTE r=(*pSrc++)>>3;
						//BYTE b=(*pSrc++)>>3;
						BYTE b=(*pSrc++)>>3;
						BYTE g=(*pSrc++)>>2;
						BYTE r=(*pSrc++)>>3;
						*pDest++=((r<<11)|(g<<5)|(b));
					}
					pDest-=dualrow;
				}
			}
		}
		else
		if ((dx_texformat==D3DFMT_X1R5G5B5)||	// 555 or 1555 format (some cards do not support 565)
			(dx_texformat==D3DFMT_A1R5G5B5))
		{
			// 16 bit AVI
			if (pbmi->bmiHeader.biBitCount==16)
			{
				//WORD* pSrc=(WORD*)(sizeof(BITMAPINFO)+(BYTE*)pbmi);
				WORD* pSrc=(WORD*)(pbmi->bmiColors);
				WORD* pDest=(WORD*)datapointer;

				// Fill upside-down, because AVI is saved so (microsoft rulez;)
				pDest+=pbmi->bmiHeader.biWidth*(pbmi->bmiHeader.biHeight-1);
				int dualrow=pbmi->bmiHeader.biWidth*2;

				for(int yy=0;yy<pbmi->bmiHeader.biHeight;yy++)
				{
					for(int xx=0;xx<pbmi->bmiHeader.biWidth;xx++)
					{
						*pDest++=*pSrc++;
					}
					pDest-=dualrow;
				}				
			}

			// 24 bit AVI
			if (pbmi->bmiHeader.biBitCount==24)
			{
				//BYTE* pSrc=(BYTE*)(sizeof(BITMAPINFO)+(BYTE*)pbmi);
				BYTE* pSrc=(BYTE*)(pbmi->bmiColors);
				WORD* pDest=(WORD*)datapointer;
				
				// Fill upside-down, because AVI is saved so (microsoft rulez;)
				pDest+=pbmi->bmiHeader.biWidth*(pbmi->bmiHeader.biHeight-1);
				int dualrow=pbmi->bmiHeader.biWidth*2;

				for(int yy=0;yy<pbmi->bmiHeader.biHeight;yy++)
				{
					for(int xx=0;xx<pbmi->bmiHeader.biWidth;xx++)
					{
						BYTE g=(*pSrc++)>>3;
						BYTE r=(*pSrc++)>>3;
						BYTE b=(*pSrc++)>>3;
						*pDest++=((r<<10)|(g<<5)|(b));
					}
					pDest-=dualrow;
				}			
			}
		}
		/*else
		if ((dx_texformat==D3DFMT_A8R8G8B8)||	// 888 or 8888 format (some cards do not support 565)
			(dx_texformat==D3DFMT_X8R8G8B8))
		{
			// 24 bit AVI
			if (pbmi->bmiHeader.biBitCount==24)
			{
				BYTE* pSrc=(BYTE*)(sizeof(BITMAPINFO)+(BYTE*)pbmi);
				DWORD* pDest=(DWORD*)datapointer;

				for(int i=0;i<pbmi->bmiHeader.biWidth*pbmi->bmiHeader.biHeight;i++)
				{
					BYTE *col=(unsigned char*)pDest;
					col[1]=*pSrc++;
					col[2]=*pSrc++;
					col[0]=*pSrc++;
					pDest++;
				}
			}
		}*/

		// Unlock the DX8 texture
		frames[fra]->UnlockRect(0);
	}

    // AVI lib exit
    AVIFileExit();
}

void *Storm3D_Texture_Video::classId()
{
	return &id;
}

void *Storm3D_Texture_Video::getId() const
{
	return &id;
}

void Storm3D_Texture_Video::swapTexture(IStorm3D_Texture *otherI)
{
	Storm3D_Texture *otherC = static_cast<Storm3D_Texture *> (otherI);
	if(!otherC)
	{
		assert(!"Whoops");
		return;
	}

	if(otherC->getId() != classId())
	{
		assert(!"Whoops");
		return;
	}

	Storm3D_Texture::swapTexture(otherC);
	Storm3D_Texture_Video *other = static_cast<Storm3D_Texture_Video *> (otherC);

	std::swap(frames, other->frames);
	std::swap(dx_texformat, other->dx_texformat);
	std::swap(frame_amount, other->frame_amount);
	std::swap(frame, other->frame);
	std::swap(framechangetime, other->framechangetime);
	std::swap(framechangecounter, other->framechangecounter);
	std::swap(loop_params, other->loop_params);
	std::swap(width, other->width);
	std::swap(height, other->height);
	std::swap(bpp, other->bpp);
	std::swap(last_time, other->last_time);
}

#else

// FIXME: placeholders

Storm3D_Texture_Video::Storm3D_Texture_Video(Storm3D *s2,const char *_filename,DWORD texloadcaps) :
	Storm3D_Texture(s2),
	frames(NULL),
	frame_amount(1),
	frame(0),
	framechangetime(0),
	framechangecounter(0),
	loop_params(VIDEOLOOP_DEFAULT),
	last_time(timeGetTime())
{
}

Storm3D_Texture_Video::~Storm3D_Texture_Video()
{
}

void Storm3D_Texture_Video::AnimateVideo()
{
}

void Storm3D_Texture_Video::VideoSetFrame(int num)
{
}


void Storm3D_Texture_Video::VideoSetFrameChangeSpeed(int millisecs)
{
}


void Storm3D_Texture_Video::VideoSetLoopingParameters(VIDEOLOOP params)
{
}


int Storm3D_Texture_Video::VideoGetFrameAmount()
{
	return 0;
}

int Storm3D_Texture_Video::VideoGetCurrentFrame()
{
	return 0;
}


void Storm3D_Texture_Video::ReadAVIVideoInfo()
{
}


void Storm3D_Texture_Video::LoadAVIVideoFrames()
{
}


void *Storm3D_Texture_Video::classId()
{
	return &id;
}


void *Storm3D_Texture_Video::getId() const
{
	return &id;
}


void Storm3D_Texture_Video::swapTexture(IStorm3D_Texture *otherI)
{
}


#endif // __GNUC__
