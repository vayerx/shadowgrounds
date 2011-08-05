// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "exporter_scene.h"
#include "export_options.h"
#include "../filesystem/output_file_stream.h"
#include <map>

#ifdef _MSC_EXTENSIONS
#define for if(false) {} else for
#endif

namespace frozenbyte {
namespace editor {
namespace {
	filesystem::OutputStream &operator << (filesystem::OutputStream &stream, const VC2I &vector)
	{
		return stream << vector.x << vector.y;
	}

	filesystem::OutputStream &operator << (filesystem::OutputStream &stream, const VC3 &vector)
	{
		return stream << vector.x << vector.y << vector.z;
	}

	filesystem::OutputStream &operator << (filesystem::OutputStream &stream, const TColor<unsigned char> &color)
	{
		return stream << color.r << color.g << color.b;
	}

	struct TexturePass
	{
		int textureIndex;
		std::vector<unsigned char> weights;

		TexturePass()
		:	textureIndex(-1)
		{
		}

		TexturePass(int textureIndex_, const std::vector<unsigned char> weights_)
		:	textureIndex(textureIndex_),
			weights(weights_)
		{
		}

		int getOrder() const
		{
			int result = 0;
			for(unsigned int i = 0; i < weights.size(); ++i)
				result += weights[i];

			return result;
		}
	};

	struct TexturePassSorter
	{
		bool operator() (const TexturePass &a, const TexturePass &b) const
		{
			return a.getOrder() > b.getOrder(); 
		}
	};

	struct Fog
	{
		std::string id;
		bool enabled;
		bool cameraCentric;
		TColor<unsigned char> color;
		float start;
		float end;

		Fog()
		:	enabled(false),
			cameraCentric(false),
			start(0),
			end(0)
		{
		}
	};

	using namespace std;
	typedef vector<TColor<unsigned char> > ColorList;
	typedef vector<Fog> FogList;
}

struct ExporterSceneData
{
	std::vector<unsigned short> heightMap;
	std::vector<unsigned short> obstacleMap; // 16x resolution (4x * 4x)
	
	VC2I mapSize;
	VC3 realSize;
	int textureRepeat;

	TColor<unsigned char> ambientColor;
	TColor<unsigned char> sunColor;
	TColor<unsigned char> fogColor;

	VC3 sunDirection;

	float cameraRange;
	float fogStart;	
	float fogEnd;

	bool fogEnabled;
	std::string backgroundModel;

	vector<string> textures;
	map<int, vector<TexturePass> > blends;
	map<int, vector<unsigned char> > lightmaps;
	int lightmapSize;

	VC2I colormapSize;
	ColorList colorMap;

	FogList fogList;

	ExporterSceneData()
	{
		lightmapSize = 0;
		textureRepeat = 5;
		cameraRange = 0;
		fogStart = 0;
		fogEnd = 0;

		fogEnabled = false;
	}

	void exportColorMap(const string &baseFileName)
	{
		std::string fileName = baseFileName + "\\bin\\color.bin";
		filesystem::OutputStream stream = filesystem::createOutputFileStream(fileName);

		stream << int(1);
		stream << colormapSize.x << colormapSize.y;

		ColorList::iterator it = colorMap.begin();
		for(; it != colorMap.end(); ++it)
			stream << *it;
	}
};

ExporterScene::ExporterScene()
{
	boost::scoped_ptr<ExporterSceneData> tempData(new ExporterSceneData());
	data.swap(tempData);
}

ExporterScene::~ExporterScene()
{
}

void ExporterScene::setHeightmap(const std::vector<unsigned short> &heightMap, const VC2I &mapSize, const VC3 &realSize)
{
	data->heightMap = heightMap;
	data->mapSize = mapSize;
	data->realSize = realSize;
}

void ExporterScene::setTextureRepeat(int value)
{
	data->textureRepeat = value;
}

void ExporterScene::setObstaclemap(const std::vector<unsigned short> &obstacleMap)
{
	data->obstacleMap = obstacleMap;
}

void ExporterScene::setAmbient(const TColor<unsigned char> &color)
{
	data->ambientColor = color;
}

void ExporterScene::setSun(const TColor<unsigned char> &color, const VC3 &direction)
{
	data->sunColor = color;
	data->sunDirection = direction;
}
/*
void ExporterScene::setFog(bool enabled, const TColor<unsigned char> &color, float start, float end)
{
	data->fogEnabled = enabled;
	data->fogColor = color;
	data->fogStart = start;
	data->fogEnd = end;
}
*/
void ExporterScene::addFog(const std::string &id, bool enabled, bool cameraCentric, const TColor<unsigned char> &color, float start, float end)
{
	Fog f;
	f.id = id;
	f.enabled = enabled;
	f.cameraCentric = cameraCentric;
	f.color = color;
	f.start = start;
	f.end = end;

	data->fogList.push_back(f);
}

void ExporterScene::setCameraRange(float range)
{
	data->cameraRange = range;
}

void ExporterScene::setSun(const VC3 &direction)
{
	data->sunDirection = direction;
}

void ExporterScene::setBackground(const std::string &modelName)
{
	data->backgroundModel = modelName;
}

void ExporterScene::addTexture(const std::string &fileName)
{
	data->textures.push_back(fileName);
}

void ExporterScene::setLightmapSize(int size)
{
	data->lightmapSize = size;
}

void ExporterScene::setBlock(int blockIndex, int textureIndex, const std::vector<unsigned char> &weights)
{
	bool found = false;
	for(unsigned int i = 0; i < weights.size(); ++i)
	{
		if(weights[i])
		{
			found = true;
			break;
		}
	}

	if(found)
		data->blends[blockIndex].push_back(TexturePass(textureIndex, weights));
}

void ExporterScene::setBlockLightmap(int blockIndex, const std::vector<unsigned char> &weights)
{
	data->lightmaps[blockIndex] = weights;
}

void ExporterScene::setColorMap(const VC2I &size, const ColorList &buffer)
{
	data->colormapSize = size;
	data->colorMap = buffer;
}

void ExporterScene::addColorMap(const VC2I &size, const ColorList &buffer)
{
	if(data->colormapSize.x != size.x || data->colormapSize.y != size.y)
		return;

	for(unsigned int i = 0; i < buffer.size(); ++i)
	{
		//data->colorMap[i] += buffer[i];
		TColor<unsigned char> &color = data->colorMap[i];
		const TColor<unsigned char> &light = buffer[i];

		if(!color.r && !color.g && !color.b)
			color = light;
	}
}

void ExporterScene::save(const ExportOptions &options) const
{
	if(options.onlyScripts)
		return;

	std::string fileName = options.fileName + "\\bin\\scene.bin";
	filesystem::OutputStream stream = filesystem::createOutputFileStream(fileName);

	stream << int(8);
	stream << data->mapSize << data->realSize;

	for(int y = 0; y < data->mapSize.y; ++y)
	for(int x = 0; x < data->mapSize.x; ++x)
		stream << data->heightMap[y * data->mapSize.x + x];

	/*
	// removed since v8
	for(int y = 0; y < data->mapSize.y * 4; ++y)
	for(int x = 0; x < data->mapSize.x * 4; ++x)
		stream << data->obstacleMap[y * (data->mapSize.x * 4) + x];
	*/

	stream << data->textureRepeat;
	stream << data->ambientColor << data->sunColor << data->sunDirection;
	stream << data->fogEnabled << data->fogColor << data->fogStart << data->fogEnd;
	stream << data->backgroundModel;
	stream << data->cameraRange;

	{
		stream << int(data->fogList.size());
		for(FogList::const_iterator it = data->fogList.begin(); it != data->fogList.end(); ++it)
		{
			const Fog &f = *it;
			stream << f.id << f.enabled << f.cameraCentric << f.color << f.start << f.end;
		}
	}

	stream << int(data->textures.size());
	for(unsigned int i = 0; i < data->textures.size(); ++i)
		stream << data->textures[i];

	{
		stream << int(data->blends.size());
		std::map<int, std::vector<TexturePass> >::iterator it = data->blends.begin();
		for(; it != data->blends.end(); ++it)
		{
			int blockIndex = it->first;
			stream << int(blockIndex);

			std::vector<TexturePass> &passes = it->second;
			std::sort(passes.begin(), passes.end(), TexturePassSorter());

			stream << int(passes.size());

			for(unsigned int i = 0; i < passes.size(); i += 2)
			{
				stream << passes[i].textureIndex;

				if(i + 1 < passes.size())
					stream << passes[i + 1].textureIndex;
				else
					stream << int(-1);

				for(unsigned int j = 0; j < passes[i].weights.size(); ++j)
				{
					stream << passes[i].weights[j];

					if(i + 1 < passes.size())
						stream << passes[i + 1].weights[j];
				}
			}	
		}
	}

	{
		int maps = data->lightmaps.size();
		stream << maps;

		std::map<int, std::vector<unsigned char> >::iterator it = data->lightmaps.begin();
		for(; it != data->lightmaps.end(); ++it)
		{
			int blockIndex = it->first;
			stream << int(blockIndex);

			const std::vector<unsigned char> &map = it->second;
			stream << int(map.size());

			for(unsigned int i = 0; i < map.size(); ++i)
				stream << map[i];
		}
	}

	stream << data->lightmapSize;
	data->exportColorMap(options.fileName);
}

} // end of namespace editor
} // end of namespace frozenbyte
