// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "terrain_colormap.h"
#include "storm.h"
#include "exporter.h"
#include "exporter_scene.h"
#include "../filesystem/output_stream.h"
#include "../filesystem/input_stream.h"
#include <istorm3d_scene.h>
#include <istorm3d_texture.h>
#include <istorm3d.h>
#include <vector>
#include <fstream>
#include <boost/scoped_array.hpp>

using namespace boost;
using namespace std;

namespace frozenbyte {
namespace editor {
namespace {

	struct Pixel
	{
		TColor<unsigned char> color;
	};

	typedef vector<Pixel> ValueList;
}

struct TerrainColorMap::Data
{
	Storm &storm;

	ValueList values;
	VC2I size;

	IStorm3D_Texture *t;
	IStorm3D_Material *m;

	Data(Storm &storm_)
	:	storm(storm_),
		t(0),
		m(0)
	{
	}

	void reset()
	{
		t = 0;
		m = 0;

		values.clear();
		size = storm.heightmapResolution;
		size *= 4;

		values.resize(size.x * size.y);
	}

	void create()
	{
		reset();

		int xr = size.x / 2;
		int yr = size.y / 2;
		float xd = storm.heightmapResolution.x / storm.heightmapSize.x * 1.f;
		float yd = storm.heightmapResolution.y / storm.heightmapSize.z * 1.f;

		IStorm3D_Camera *camera = storm.scene->GetCamera();
		float fov = camera->GetFieldOfView();
		camera->SetFieldOfView(PI / 180.f * 60.f);
		camera->SetUpVec(VC3(0, 0, 1.f));

		for(int y = -yr; y < yr; ++y)
		for(int x = -xr; x < xr; ++x)
		{
			Storm3D_CollisionInfo info;
			VC3 pos(x * xd, 400.f, y * yd);

			storm.scene->RayTrace(pos, VC3(0, -1.f, 0), 500.f, info);
			if(!info.model || !info.object || !info.hit)
				continue;
			
			VC3 target = pos;
			target.y = info.position.y;
			pos = target + VC3(0, 1.5f, 0);

			camera->SetPosition(pos);
			camera->SetTarget(target);
			//storm.scene->RenderScene(false);

//storm.scene->RenderScene(true);
storm.scene->RenderScene(false);
DWORD colorValue = storm.storm->getScreenColorValue(VC2(.1f, .1f));
unsigned char r = unsigned char((colorValue & 0x00FF0000) >> 16);
unsigned char g = unsigned char((colorValue & 0x0000FF00) >> 8);
unsigned char b = unsigned char((colorValue & 0x000000FF));
Pixel &pixel = values[(y + yr) * size.x + (x + xr)];
pixel.color = TColor<unsigned char> (r, g, b);

/*
			scoped_ptr<IStorm3D_ScreenBuffer> buffer(storm.storm->TakeScreenshot(VC2(.1f, .1f)));
			if(!buffer)
				continue;

			Pixel &pixel = values[(y + yr) * size.x + (x + xr)];
			pixel.color = getColor(*buffer.get());
*/
			//Sleep(100);
		}

		camera->SetUpVec(VC3(0, 1.f, 0));
		camera->SetFieldOfView(fov);

		// Temp
		{
			ofstream stream("color.raw", ios::binary);

			for(int y = -yr; y < yr; ++y)
			for(int x = -xr; x < xr; ++x)
			{
				Pixel &pixel = values[(y + yr) * size.x + (x + xr)];
				stream << pixel.color.r << pixel.color.g << pixel.color.b;
			}
		}
	}

	TColor<unsigned char> getColor(IStorm3D_ScreenBuffer &screenBuffer) const
	{
		int r = 0;
		int g = 0;
		int b = 0;

		VC2I bufferSize = screenBuffer.getSize();
		const vector<DWORD> &buffer = screenBuffer.getBuffer();

		for(unsigned int i = 0; i < buffer.size(); ++i)
		{
			DWORD value = buffer[i];

			r += (value & 0x00FF0000) >> 16;
			g += (value & 0x0000FF00) >> 8;
			b += (value & 0x000000FF);
		}

		int count = buffer.size();
		TColor<unsigned char> result(r / count, g / count, b / count);
		return result;
	}

	TColor<unsigned char> getColor(const VC2 &position) const
	{
		if(!storm.terrain || values.empty())
			return TColor<unsigned char> ();

		const VC3 &hSize = storm.heightmapSize;
		VC2 pos(hSize.x * .5f, hSize.z * .5f);
		pos += position;
		pos.x /= hSize.x;
		pos.y /= hSize.z;

		//assert(pos.x >= 0 && pos.x <= 1.f);
		//assert(pos.y >= 0 && pos.y <= 1.f);
		pos.x = max(0.f, pos.x);
		pos.y = max(0.f, pos.y);
		pos.x = min(0.99f, pos.x);
		pos.y = min(0.99f, pos.y);

		int x = int(pos.x * size.x + 0.5f);
		int y = int(pos.y * size.y + 0.5f);

		const Pixel &pixel = values[y * size.x + x];
		return pixel.color;
	}

	void debugRender()
	{
		if(size.x == 0 || size.y == 0)
			return;

		if(!t)
			t = storm.storm->CreateNewTexture(size.x, size.y, IStorm3D_Texture::TEXTYPE_BASIC);
		if(!m)
			m = storm.storm->CreateNewMaterial("..");
		m->SetBaseTexture(t);

		scoped_array<DWORD> buffer(new DWORD[size.x * size.y]);
		for(int y = 0; y < size.y; ++y)
		for(int x = 0; x < size.x; ++x)
		{
			int index = y * size.x + x;
			Pixel &pixel = values[index];

			DWORD value = (pixel.color.r << 16) | (pixel.color.g << 8) | (pixel.color.b);
			buffer[index] = value;
		}

		t->Copy32BitSysMembufferToTexture(buffer.get());
		storm.scene->Render2D_Picture(m, VC2(1,1), VC2(1024, 512));
	}

	void write(filesystem::OutputStream &stream) const
	{
		stream << int(1);
		stream << size.x << size.y;

		for(int i = 0; i < size.x * size.y; ++i)
		{
			const Pixel &p = values[i];
			stream << p.color.r << p.color.g << p.color.b;
		}
	}

	void read(filesystem::InputStream &stream)
	{
		reset();

		int version = 0;
		stream >> version;

		stream >> size.x >> size.y;
		values.resize(size.x * size.y);

	assert(sizeof(Pixel) == 3);
	stream.read(&values[0].color.r, size.x * size.y * 3);
		/*
		for(int i = 0; i < size.x * size.y; ++i)
		{
			Pixel &p = values[i];
			int a = sizeof(Pixel);
			stream >> p.color.r >> p.color.g >> p.color.b;
		}
		*/
	}
};

TerrainColorMap::TerrainColorMap(Storm &storm)
{
	scoped_ptr<Data> tempData(new Data(storm));
	data.swap(tempData);
}

TerrainColorMap::~TerrainColorMap()
{
}

void TerrainColorMap::reset()
{
	data->reset();
}

void TerrainColorMap::create()
{
	data->create();
}

void TerrainColorMap::debugRender()
{
	data->debugRender();
}

COL TerrainColorMap::getColor(const VC2 &position) const
{
	TColor<unsigned char> color = data->getColor(position);

	float factor = .8f;
	return COL(factor * color.r / 255.f, factor * color.g / 255.f, factor * color.b / 255.f);
}

void TerrainColorMap::doExport(Exporter &exporter) const
{
	ExporterScene &scene = exporter.getScene();
	vector<TColor<unsigned char> > buffer;

	ValueList::iterator it = data->values.begin();
	for(; it != data->values.end(); ++it)
		buffer.push_back(it->color);

	scene.setColorMap(data->size, buffer);
}

filesystem::OutputStream &TerrainColorMap::writeStream(filesystem::OutputStream &stream) const
{
	data->write(stream);
	return stream;
}

filesystem::InputStream &TerrainColorMap::readStream(filesystem::InputStream &stream)
{
	data->read(stream);
	return stream;
}

} // editor
} // frozenbyte
