#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>

#include <istorm3d_streambuffer.h>
#include "storm3d_videostreamer.h"
#include "storm3d.h"
#include "storm3d_texture.h"
#include "storm3d_material.h"
#include "storm3d_scene.h"
#include "storm3d_string_util.h"
#include "treader.h"

#include "../../util/Debug_MemoryManager.h"

#ifdef WIN32

#include <windows.h>

#endif

#ifdef _MSC_VER
#if defined(DEBUG) || defined(_DEBUG)
	#pragma comment(lib, "storm_strmbasd.lib")
#else
	#pragma comment(lib, "storm_strmbase.lib")
#endif
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

	Uint32 *buffer;

	VC2 start;
	VC2 end;
	float alpha;
	bool finished;
	bool looping;

	bool useDynamicTextures;

	bool downscaled;
	int lastFrame;
	Uint32 time;
	unsigned int width, height; // texture w&h, pow2
	unsigned int videoWidth, videoHeight; // actual video w&h

	bool higherColorRange;

	Data(Storm3D &storm_, bool downscale_, bool higherColorRange_)
	:	storm(storm_),
		streamBuilder(0),
		buffer(0),
		alpha(1.f),
		finished(false),
		looping(false),
		useDynamicTextures(true),
		downscaled(downscale_),
		lastFrame(-1),
		time(0),
		width(0),
		height(0),
		higherColorRange(true)
	{
		// FIXME: do we need downscaled and higherColorRange ?
	}

	~Data()
	{
    delete[] buffer;
	}

	boost::shared_ptr<TReader> test;

	void initialize(const std::string &fileName)
	{
		// Create filename
		std::string file = fileName;
		std::string::size_type pos = file.find("wmv");
		if(pos != std::string::npos)
			file.replace(pos, 3, "ogg");
		// change to lowercase
		// change \ to /

		size_t l = file.length();
		for (size_t i = 0; i < l; i++) {
			if (isupper(file[i])) {
				file[i] = tolower(file[i]);
			} else if (file[i] == '\\') {
				file[i] = '/';
			}
		}

		test = boost::shared_ptr<TReader>(new TReader());

		if(test->read_info(file.c_str(), streamBuilder))
			return;

		if (test->init()) {
			igiosWarning("init failed\n");
			return;
		}

		videoWidth = test->frame_width;
		videoHeight = test->frame_height;
		int tempw = videoWidth, temph = videoHeight;
		toNearestPow(tempw); toNearestPow(temph);
		width = tempw; height = temph;
	}

	bool initStormResources()
	{
		if(width == 0 || height == 0)
			return false;

		if(useDynamicTextures)
		{
			/*
			if(downscaled)
			{
				width /= 2;
				height /= 2;
			}
			*/

			IStorm3D_Texture::TEXTYPE textype = IStorm3D_Texture::TEXTYPE_DYNAMIC_LOCKABLE;

			texture1.reset(static_cast<Storm3D_Texture *> (storm.CreateNewTexture(width, height, textype)), std::mem_fun(&Storm3D_Texture::Release));
			texture2.reset(static_cast<Storm3D_Texture *> (storm.CreateNewTexture(width, height, textype)), std::mem_fun(&Storm3D_Texture::Release));

			activeTexture = texture1;
			if(!texture1 || !texture2)
				return false;
		}
		else
		{
			ramTexture.reset(new Storm3D_Texture(&storm, width, height, IStorm3D_Texture::TEXTYPE_RAM), std::mem_fun(&Storm3D_Texture::Release));
			texture1.reset(static_cast<Storm3D_Texture *> (storm.CreateNewTexture(width, height, IStorm3D_Texture::TEXTYPE_DYNAMIC)), std::mem_fun(&Storm3D_Texture::Release));
			texture2.reset(static_cast<Storm3D_Texture *> (storm.CreateNewTexture(width, height, IStorm3D_Texture::TEXTYPE_DYNAMIC)), std::mem_fun(&Storm3D_Texture::Release));

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
			material->SetBaseTexture(activeTexture.get());
		}
		else
		{
			material->SetBaseTexture(activeTexture.get());
		}

		int x1 = 0;
		int y1 = 0;
		int x2 = windowSizeX;
		int y2 = windowSizeY;

		float textureRatio = float(videoWidth) / float(videoHeight);
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
	if(!data.get() || data->finished || data->width == 0 || data->height == 0)
		return;

	double frame_duration = ((double)data->test->fps_numerator / data->test->fps_denominator) * 1000;

	Uint32 current = SDL_GetTicks();
	Uint32 diff = current - data->time;

	if(diff < frame_duration)
		return;

	data->time = current;

	if(data->useDynamicTextures)
	{
		if(data->activeTexture == data->texture1)
			data->activeTexture = data->texture2;
		else
			data->activeTexture = data->texture1;

		data->material->SetBaseTexture(data->activeTexture.get());

		int height = data->height;
		int width = data->width;
		if(!data->test->nextframe())
		{
			if (!data->buffer) data->buffer = new Uint32[width * height];
			DWORD *buf = (DWORD *) data->buffer;
			data->test->read_pixels((char*)buf, width, height);
			data->activeTexture->Copy32BitSysMembufferToTexture(buf);
		}
		else if(data->looping)
		{
			//FIXME: restart video
			data->test->restart();
		}
		else
		{
			delete[] data->buffer;
			data->buffer = 0;
			data->finished = true;
			data->test->finish();
		}
	}
}

void Storm3D_VideoStreamer::render(IStorm3D_Scene *scene)
{
	if(!data || !scene || data->finished)
		return;

	update();
	float x, y;
	getTextureCoords(x, y);
	scene->Render2D_Picture(data->material.get(), data->start, data->end, data->alpha, 0, 0,0,x,y);
}

void Storm3D_VideoStreamer::getTextureCoords(float &x, float &y)
{
	x = (float)data->videoWidth / data->width;
	y = (float)data->videoHeight / data->height;
}
