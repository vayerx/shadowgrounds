#include <vector>
#include <string>

#include <istorm3d_streambuffer.h>
#include "storm3d_videostreamer.h"
#include "storm3d.h"
#include "storm3d_texture.h"
#include "storm3d_material.h"
#include "storm3d_scene.h"
#include "storm3d_string_util.h"

#include "../../util/Debug_MemoryManager.h"
//#pragma pack(8)

#include <windows.h>
#include <atlbase.h>
#include <boost/shared_ptr.hpp>

#ifndef __GNUC__
#include <wmsdkidl.h>
#include "wmvsync/rostream.h"
#include "wmvsync/reader.h"
#pragma comment(lib, "wmvcore.lib")

#if defined(DEBUG) || defined(_DEBUG)
	#pragma comment(lib, "storm_strmbasd.lib")
#else
	#pragma comment(lib, "storm_strmbase.lib")
#endif

namespace {

	static const int INTERLACE = 2;

} // unnamed


struct Storm3D_VideoStreamer::Data
{
	Storm3D &storm;
	IStorm3D_StreamBuilder *streamBuilder;

	boost::shared_ptr<Storm3D_Texture> ramTexture;
	boost::shared_ptr<Storm3D_Texture> texture1;
	boost::shared_ptr<Storm3D_Texture> texture2;

	boost::shared_ptr<Storm3D_Texture> activeTexture;
	boost::shared_ptr<Storm3D_Material> material;

	VC2 start;
	VC2 end;
	float alpha;
	bool finished;
	bool looping;

	//CRITICAL_SECTION colorBufferLock;
	bool useDynamicTextures;

	bool downscaled;
	int lastFrame;
	int time;

	bool higherColorRange;

	Data(Storm3D &storm_, bool downscale_, bool higherColorRange_)
	:	storm(storm_),
		streamBuilder(0),
		finished(false),
		looping(false),
		alpha(1.f),
		useDynamicTextures(true),
		downscaled(downscale_),
		//useDynamicTextures(false),
		lastFrame(-1),
		time(0),
		higherColorRange(higherColorRange_)
	{
		CoInitialize(0);
	}

	~Data()
	{
		if(test.playing)
			test.Stop();
		test.Close();
	}

	CReader test;

	void initialize(const std::string &fileName)
	{
		//std::wstring wideFileName = Storm3D_MakeWideString(fileName);

		test.higherColorRange = higherColorRange;
		test.streamBuilder = streamBuilder;
		HRESULT hr = test.Open(fileName.c_str());
		if(SUCCEEDED(hr))
			test.Start();
	}

	bool initStormResources()
	{
		if(!test.width || !test.height)
			return false;

		if(useDynamicTextures)
		{
			int width = test.width;
			int height = test.height;

			if(downscaled)
			{
				width /= 2;
				height /= 2;
			}

			IStorm3D_Texture::TEXTYPE textype = IStorm3D_Texture::TEXTYPE_DYNAMIC_LOCKABLE;
			if(higherColorRange)
				textype = IStorm3D_Texture::TEXTYPE_DYNAMIC_LOCKABLE_32;

			texture1.reset(static_cast<Storm3D_Texture *> (storm.CreateNewTexture(width, height, textype)), std::mem_fun(&Storm3D_Texture::Release));
			texture2.reset(static_cast<Storm3D_Texture *> (storm.CreateNewTexture(width, height, textype)), std::mem_fun(&Storm3D_Texture::Release));

			activeTexture = texture1;
			if(!texture1 || !texture2)
				return false;
		}
		else
		{
			ramTexture.reset(new Storm3D_Texture(&storm, test.width, test.height, IStorm3D_Texture::TEXTYPE_RAM), std::mem_fun(&Storm3D_Texture::Release));
			texture1.reset(static_cast<Storm3D_Texture *> (storm.CreateNewTexture(test.width, test.height, IStorm3D_Texture::TEXTYPE_DYNAMIC)), std::mem_fun(&Storm3D_Texture::Release));
			texture2.reset(static_cast<Storm3D_Texture *> (storm.CreateNewTexture(test.width, test.height, IStorm3D_Texture::TEXTYPE_DYNAMIC)), std::mem_fun(&Storm3D_Texture::Release));

			activeTexture = texture1;
			if(!texture1 || !texture2 || !ramTexture)
				return false;
		}

		material.reset(new Storm3D_Material(&storm, "video material"));
		if(!material)
			return false;

		Storm3D_SurfaceInfo info = storm.GetScreenSize();
		int windowSizeX = info.width;
		int windowSizeY = info.height;

		if(useDynamicTextures)
		{
			int textureWidth = texture1->getWidth();
			int textureHeight = texture2->getHeight();
			material->SetBaseTexture(activeTexture.get());
		}
		else
		{
			int textureWidth = ramTexture->getWidth();
			int textureHeight = ramTexture->getHeight();
			material->SetBaseTexture(activeTexture.get());
		}

		int x1 = 0;
		int y1 = 0;
		int x2 = windowSizeX;
		int y2 = windowSizeY;

		float textureRatio = float(test.width) / float(test.height);
		y2 = int(windowSizeX / textureRatio);
		y1 = (windowSizeY - y2) / 2;
		y2 += y1;

		start = VC2(float(x1), float(y1));
		end = VC2(float(x2-x1), float(y2-y1));

		return true;
	}

	bool hasVideo() const
	{
		if(!texture1 || !texture2)
			return false;

		return true;
	}
};

Storm3D_VideoStreamer::Storm3D_VideoStreamer(Storm3D &storm, const char *fileName, IStorm3D_StreamBuilder *streamBuilder, bool loop, bool downscale, bool higherColorRange)
{
	boost::scoped_ptr<Data> tempData(new Data(storm, downscale, higherColorRange));
	tempData->streamBuilder = streamBuilder;
	tempData->initialize(fileName);
	data.swap(tempData);

	if(!data->initStormResources())
		return;

	data->looping = loop;
}

Storm3D_VideoStreamer::~Storm3D_VideoStreamer()
{
}

bool Storm3D_VideoStreamer::hasVideo() const
{
	if(!data)
		return false;

	return data->hasVideo();
}

bool Storm3D_VideoStreamer::hasEnded() const
{
	if(!data || data->finished)
		return true;

	return false;
}

int Storm3D_VideoStreamer::getTime() const
{
	if(!data)
		return 0;

	return data->time;
}

bool Storm3D_VideoStreamer::isPlaying() const
{
	return !hasEnded();
}

IStorm3D_Material *Storm3D_VideoStreamer::getMaterial()
{
	return data->material.get();
}

void Storm3D_VideoStreamer::setPosition(const VC2 &position, const VC2 &size)
{
	data->start = position;
	data->end = size;
}

void Storm3D_VideoStreamer::setAlpha(float alpha)
{
	data->alpha = alpha;
}

void Storm3D_VideoStreamer::update()
{
	if(!data || data->finished)
		return;

	EnterCriticalSection(&data->test.lock);

	int frame = data->test.frameIndex;
	if(data->test.streamBuffer)
	{
		QWORD soundTime = data->test.streamBuffer->getCurrentTime();

		{
			// Find closest to current time
			__int64 ref = 0;
			for(int i = 0; i < FRAME_BUFFER_AMOUNT; ++i)
			{
				const CReader::Frame &f = data->test.frames[i];
				if(i == 0)
				{
					frame = i;
					ref = f.start - soundTime;
					if(ref < 0)
						ref = -ref;
				}

				__int64 delta = f.start - soundTime;
				if(delta < 0)
					delta = -delta;
				if(delta < ref)
				{
					frame = i;
					ref = delta;
				}
			}
		}
	}

	const CReader::Frame &f = data->test.frames[frame];
	data->time = int(f.start / 10000);

	if(data->useDynamicTextures)
	{
		if(data->lastFrame != frame)
		{
			data->lastFrame = frame;
			if(data->activeTexture == data->texture1)
				data->activeTexture = data->texture2;
			else
				data->activeTexture = data->texture1;

			data->material->SetBaseTexture(data->activeTexture.get());

			D3DLOCKED_RECT rect;
			if(data->activeTexture->lock(rect))
			{
				int height = data->test.height;
				int width = data->test.width;

				if(data->higherColorRange)
				{
					DWORD *src = &data->test.frames[frame].colorBuffer32[0];
					DWORD *dst = reinterpret_cast<DWORD *> (rect.pBits);
					int pitch = rect.Pitch / 4;


					if(data->downscaled)
					{
						std::vector<DWORD> buff(width / 2);
						for(int j = 0; j < height / 2; ++j)
						{
							int line = (j * 2 * width);

							for(int i = 0; i < width / 2; ++i)
							{
								int index[4] = 
								{
									line + i * 2,
									line + i * 2 + 1,
									line + i * 2 + width,
									line + i * 2 + width + 1
								};

								int r = 0;
								int g = 0;
								int b = 0;
								for(int k = 0; k < 4; ++k)
								{
									DWORD value = src[index[k]];
									r += (value >> 16) & 255;
									g += (value >> 8) & 255;
									b += value & 255;
								}

								r /= 4;
								g /= 4;
								b /= 4;

								buff[i] = (255 << 24) | (r << 16) | (g << 8) | (b);
							}

							memcpy(dst, &buff[0], width * 2);
							dst += pitch;
						}
					}
					else
					{
						for(int j = 0; j < height; ++j)
						{
							memcpy(dst, src, width * 4);
							src += width;
							dst += pitch;
						}
					}
				}
				else
				{
					WORD *src = &data->test.frames[frame].colorBuffer[0];
					WORD *dst = reinterpret_cast<WORD *> (rect.pBits);
					int pitch = rect.Pitch / 2;

					if(data->downscaled)
					{
						std::vector<WORD> buff(width / 2);
						for(int j = 0; j < height / 2; ++j)
						{
							int line = (j * 2 * width);

							for(int i = 0; i < width / 2; ++i)
							{
								int index[4] = 
								{
									line + i * 2,
									line + i * 2 + 1,
									line + i * 2 + width,
									line + i * 2 + width + 1
								};

								int r = 0;
								int g = 0;
								int b = 0;
								for(int k = 0; k < 4; ++k)
								{
									WORD value = src[index[k]];
									r += (value >> 11) & 31;
									g += (value >> 5) & 63;
									b += value & 31;
								}

								r /= 4;
								g /= 4;
								b /= 4;

								buff[i] = r << 11 | g << 5 | b;
							}

							memcpy(dst, &buff[0], width);
							dst += pitch;
						}
					}
					else
					{
						for(int j = 0; j < height; ++j)
						{
							memcpy(dst, src, width * 2);
							src += width;
							dst += pitch;
						}
					}
				}

				data->activeTexture->unlock();
			}
		}
	}

	int testFrame = data->test.frameIndex;
	LeaveCriticalSection(&data->test.lock);

	if(!data->test.playing && data->lastFrame == testFrame)
	{
		if(data->test.streamBuffer)
		{
			if(data->test.streamBuffer)
				data->test.streamBuffer->deactivate();
		}

		if(!data->looping)
		{
			data->test.Stop();
			data->finished = true;
		}
		else
			data->test.Start();
	}

}

void Storm3D_VideoStreamer::render(IStorm3D_Scene *scene)
{
	if(!data || !scene || data->finished)
		return;

	update();
	scene->Render2D_Picture(data->material.get(), data->start, data->end, data->alpha, 0, 0,0,1,1);
}


void Storm3D_VideoStreamer::getTextureCoords(float &x, float &y)
{
	x = 1.0f; y = 1.0f;
}

#else


// FIXME: placeholders


struct Storm3D_VideoStreamer::Data
{
	Storm3D &storm;
	boost::shared_ptr<Storm3D_Material> material;

	Data(Storm3D &storm_, bool downscale_, bool higherColorRange_)
	:	storm(storm_)
	{
	}

	bool initStormResources()
	{
		material.reset(new Storm3D_Material(&storm, "video material"));
		return false;
	}
};


Storm3D_VideoStreamer::Storm3D_VideoStreamer(Storm3D &storm, const char *fileName, IStorm3D_StreamBuilder *streamBuilder, bool loop, bool downscale, bool higherColorRange)
{
	boost::scoped_ptr<Data> tempData(new Data(storm, downscale, higherColorRange));
	data.swap(tempData);

	if(!data->initStormResources())
		return;
}


Storm3D_VideoStreamer::~Storm3D_VideoStreamer()
{
}



bool Storm3D_VideoStreamer::hasEnded() const
{
	return true;
}


bool Storm3D_VideoStreamer::hasVideo() const
{
	return false;
}


void Storm3D_VideoStreamer::render(IStorm3D_Scene *scene)
{
	return;
}


void Storm3D_VideoStreamer::update()
{
	return;
}


int Storm3D_VideoStreamer::getTime() const
{
	return 0;
}


bool Storm3D_VideoStreamer::isPlaying() const
{
	return !hasEnded();
}


void Storm3D_VideoStreamer::setPosition(const VC2 &position, const VC2 &size)
{
    return;
}


void Storm3D_VideoStreamer::setAlpha(float alpha)
{
}


IStorm3D_Material *Storm3D_VideoStreamer::getMaterial()
{
	return data->material.get();
}

void Storm3D_VideoStreamer::getTextureCoords(float &x, float &y)
{
	x = 1.0f; y = 1.0f;
}

#endif // __GNUC__

