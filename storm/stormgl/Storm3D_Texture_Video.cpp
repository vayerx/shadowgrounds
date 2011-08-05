// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d.h"
#include "storm3d_texture.h"

namespace {
	int id;
}


//! Constructor
Storm3D_Texture_Video::Storm3D_Texture_Video(Storm3D *s2,const char *_filename,DWORD texloadcaps) :
	Storm3D_Texture(s2),
	frame_amount(1),
	frame(0),
	framechangetime(0),
	framechangecounter(0),
	loop_params(VIDEOLOOP_DEFAULT),
	last_time(SDL_GetTicks())
{
	// Create filename
	int len = strlen(_filename);
	filename=new char[len+1];
	strcpy(filename,_filename);
	filename[len - 3] = 'o';
	filename[len - 2] = 'g';
	filename[len - 1] = 'g';

	reader = boost::shared_ptr<TReader>(new TReader());

	// Allocate memory for frame array
	buf = NULL;
	width = 0;
	height = 0;

	// Get frame_amount etc. from file
	ReadAVIVideoInfo();

	// Create textures for videoframes

	// Get first frame's textureformat:
	// 3d-card may not support 565, and DX can change format if that happens.

	// Fill textures
	LoadAVIVideoFrames();
}

//! Destructor
Storm3D_Texture_Video::~Storm3D_Texture_Video()
{
	// Release frame array
	vector<GLuint>::const_iterator it = frames.begin();
	for(; it != frames.end(); ++it){
		const GLuint temp = *it;
		glDeleteTextures(1, &temp);
	}

	delete[] buf;
	buf = NULL;

	//delete reader;
}

//! Animate video
void Storm3D_Texture_Video::AnimateVideo()
{
	// Calculate time difference
	DWORD time_now=SDL_GetTicks();
	DWORD time_dif=time_now-last_time;
	// added use of timing factor
	// --jpk
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
						texhandle=frames[frame];
						break;

					case VIDEOLOOP_PINGPONG:
					case VIDEOLOOP_PP_STOP_AT_BEGINNING:
						frame=frame_amount-1;
						framechangetime=-framechangetime;
						framechangecounter=0;
						texhandle=frames[frame];
						return;

					case VIDEOLOOP_STOP_AT_END:
						frame=frame_amount-1;
						framechangetime=0;
						framechangecounter=0;
						texhandle=frames[frame];
						return;
				}
			}
			texhandle=frames[frame];
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
						texhandle=frames[frame];
						break;

					case VIDEOLOOP_PINGPONG:
						frame=0;
						framechangetime=-framechangetime;
						framechangecounter=0;
						texhandle=frames[frame];
						return;

					case VIDEOLOOP_STOP_AT_END:
					case VIDEOLOOP_PP_STOP_AT_BEGINNING:
						frame=0;
						framechangetime=0;
						framechangecounter=0;
						texhandle=frames[frame];
						return;
				}
			}
			texhandle=frames[frame];
		}
	}
}

//! Set frame
/*!
	\param num frame number
*/
void Storm3D_Texture_Video::VideoSetFrame(int num)
{
	// Test param
	if (num<0) num=0;
	else if (num>=frame_amount) num=frame_amount-1;

	// Set frame
	frame=num;
	texhandle=frames[frame];
}

//! Set frame change speed
/*
	\param millisecs change speed
*/
void Storm3D_Texture_Video::VideoSetFrameChangeSpeed(int millisecs)
{
	framechangetime=millisecs;
}

//! Set video looping parameters
/*
	\param params parameters
*/
void Storm3D_Texture_Video::VideoSetLoopingParameters(VIDEOLOOP params)
{
	loop_params=params;
}

//! Get frame amount
/*
	\return number of frames
*/
int Storm3D_Texture_Video::VideoGetFrameAmount()
{
	return frame_amount;
}

//! Get current frame
/*
	\return frame number
*/
int Storm3D_Texture_Video::VideoGetCurrentFrame()
{
	return frame;
}

//! Read video information
void Storm3D_Texture_Video::ReadAVIVideoInfo()
{
	if(reader->read_info(filename, 0))
		return;

	reader->init();

	// Get format/size from first frame
	//bpp=reader->ti.pixelformat;
	width = reader->frame_width;
	height = reader->frame_height;

	// Get video lenght
	igios_unimplemented();

	buf = new char[width * height * 4];
}

//! Load video frames
void Storm3D_Texture_Video::LoadAVIVideoFrames()
{
	if(width == 0 || height == 0)
		return;

	// Animation parameters
	float dwRate = (float)reader->fps_numerator;
	float dwScale = (float)reader->fps_denominator;
	frame=0;
	framechangecounter=0;
	framechangetime=(int)(1000.0f*(dwRate/dwScale));

	// Loop through frames and convert them into textures
	while(!reader->nextframe()){
		glGenTextures(1, &handle);
		frames.push_back(handle);
		glBindTexture(GL_TEXTURE_2D, handle);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		GLenum fmt = GL_RGBA;
		GLenum internalfmt = GL_RGBA;

		reader->read_pixels(buf, width, height);

		gluBuild2DMipmaps(GL_TEXTURE_2D, internalfmt, width, height, fmt, GL_UNSIGNED_BYTE, buf);
		frame_amount++;
	}

	if(frames.size() >= (unsigned int)frame)
		texhandle=frames[frame];

	reader->finish();
}

//! Class ID
void *Storm3D_Texture_Video::classId()
{
	return &id;
}

//! Get class ID
void *Storm3D_Texture_Video::getId() const
{
	return &id;
}

//! Swap textures
/*!
	\param otherI other texture
*/
void Storm3D_Texture_Video::swapTexture(IStorm3D_Texture *otherI)
{
	Storm3D_Texture *otherC = static_cast<Storm3D_Texture *> (otherI);
	if(!otherC)
	{
		assert(!"Whoops");
		return;
	}

	Storm3D_Texture::swapTexture(otherC);
	Storm3D_Texture_Video *other = static_cast<Storm3D_Texture_Video *> (otherC);

	std::swap(frames, other->frames);
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
