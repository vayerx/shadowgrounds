
#include "precompiled.h"

#include <map>
#include <string>
#include <fstream>
#include <stdio.h>

#include <istorm3D_terrain_renderer.h>
#include "Map.h"
#include "../game/Game.h"
#include "../game/GameMap.h"
#include "../util/assert.h"
#include "../filesystem/input_stream_wrapper.h"
#include "../filesystem/file_package_manager.h"
#include "../filesystem/input_stream.h"

#include "../game/GameUI.h"
#include "../game/GameScene.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_graphics.h"

#include "../game/scripting/GameScripting.h"



using namespace std;
using namespace boost;
using namespace game;
using namespace util;
using namespace frozenbyte;

namespace ui {
namespace {

	static const int UPDATE_INTERVAL = 100;
	static const unsigned int MAP_RESOLUTION = 1024;
	static const int RAW_SIZE = MAP_RESOLUTION * MAP_RESOLUTION * 3;
	static const int MAX_READ_CHUNK = 1024 * 30;
	static const float MAP_SMALLEST_BLOCK = 3.f;

	typedef vector<TColor<unsigned char> > ColorList;
	typedef vector<char> VisibilityList;

	void makeName(const string &dir, const string &id, string &result)
	{
		if(dir.empty())
			return;

		result = dir;
		char c = result[result.size() - 1];

		if(c != '\\' && c != '/')
			result += '/';

		result += "map_";
		result += id;
	}

	bool fileExists(const string &fileName)
	{
		filesystem::FB_FILE *fp = filesystem::fb_fopen(fileName.c_str(), "rb");
		if(!fp)
			return false;

		filesystem::fb_fclose(fp);
		return true;
	}

	enum FileType
	{
		Before,
		After
	};

	void generateFile(const string &fileName, const VC2 &start, const VC2 &size, const VC2 &start_, const VC2 &size_, Game &game, FileType type)
	{
		GameMap *map = game.gameMap;
		AreaMap *areaMap = map->getAreaMap();
		if(!map || !areaMap)
			return;

		game.gameScripting->runSingleSimpleStringCommand( "renderMapToFile", fileName.c_str());
		game.gameScripting->runSingleSimpleStringCommand( "setMapView", "0");

	}

	void loadFile(const string &fileName, ColorList &result)
	{
		/*
		ifstream stream(fileName.c_str(), std::ios::binary);
		if(!stream)
			return;

		int rawSize = MAP_RESOLUTION * MAP_RESOLUTION * 3;
		vector<unsigned char> buffer(rawSize);
		char *ptr = reinterpret_cast<char *> (&buffer[0]);
		stream.read(ptr, rawSize);
		*/

		filesystem::InputStream fileStream = filesystem::FilePackageManager::getInstance().getFile(fileName);
		if(!fileStream.isEof())
		{
			int rawSize = MAP_RESOLUTION * MAP_RESOLUTION * 3;
			vector<unsigned char> buffer(rawSize);
			char *ptr = reinterpret_cast<char *> (&buffer[0]);
			fileStream.read(ptr, rawSize);

			result.resize(MAP_RESOLUTION * MAP_RESOLUTION);
			for(unsigned int y = 0; y < MAP_RESOLUTION; ++y)
			for(unsigned int x = 0; x < MAP_RESOLUTION; ++x)
			{
				int index = y * MAP_RESOLUTION + x;
				int bufferIndex = index * 3;

				result[index].r = buffer[bufferIndex];
				result[index].g = buffer[bufferIndex + 1];
				result[index].b = buffer[bufferIndex + 2];
			}
		}
	}
} // unnamed

	struct FileReader
	{
		bool dataDone;
		bool asyncDone;

		vector<unsigned char> buffer;
		ColorList &result;
		string fileName;
		int offset;

		FileReader(const string &fileName_, ColorList &result_)
		:	dataDone(false),
			asyncDone(false),
			buffer(RAW_SIZE),
			result(result_),
			fileName(fileName_),
			offset(0)
		{
			/*
			OVERLAPPED foo = { 0 };
			overlapped = foo;

			result.resize(MAP_RESOLUTION * MAP_RESOLUTION);

			fileHandle = CreateFile(fileName.c_str(), GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
			update();
			*/
		}

		~FileReader()
		{
			//free();
		}

		void copyData()
		{
			/*
			for(unsigned int y = 0; y < MAP_RESOLUTION; ++y)
			for(unsigned int x = 0; x < MAP_RESOLUTION; ++x)
			{
				int index = y * MAP_RESOLUTION + x;
				int bufferIndex = index * 3;

				result[index].r = buffer[bufferIndex];
				result[index].g = buffer[bufferIndex + 1];
				result[index].b = buffer[bufferIndex + 2];
			}
			*/
		}

		void update()
		{
			/*
			if(asyncDone)
				return;

			if(!fileHandle)
			{
				asyncDone = true;
				return;
			}

			DWORD bytesRead = 0;
			DWORD requestAmount = RAW_SIZE - offset;
			if(requestAmount > MAX_READ_CHUNK)
				requestAmount = MAX_READ_CHUNK;

			overlapped.Offset = offset;
			BOOL readResult = ReadFile(fileHandle, &buffer[offset], requestAmount, &bytesRead, &overlapped);

			if(!readResult)
			{
				DWORD errorCode = GetLastError();

				if(errorCode == ERROR_HANDLE_EOF)
				{
					Logger::getInstance()->error("Reading file .. EOF");
				}

				if(errorCode != ERROR_IO_PENDING)
				{
					CancelIo(fileHandle);
					asyncDone = true;
					return;
				}

				readResult = GetOverlappedResult(fileHandle, &overlapped, &bytesRead, FALSE);
				if(!readResult && GetLastError() != ERROR_IO_INCOMPLETE)
				{
					CancelIo(fileHandle);
					asyncDone = true;
					return;
				}
			}

			offset += bytesRead;
			if(offset >= RAW_SIZE)
			{
				asyncDone = true;
				dataDone = true;
				copyData();
			}
			*/
		}

		void forceDataReady()
		{
			/*
			if(dataDone)
				return;

			if(!asyncDone)
			{
				CancelIo(fileHandle);
				asyncDone = true;
			}

			free();
			*/

			loadFile(fileName, result);
			dataDone = true;
		}

		void free()
		{
			/*
			if(fileHandle)
			{
				CancelIo(fileHandle);
				CloseHandle(fileHandle);
			}

			fileHandle = 0;
			*/
		}
	};

	/*
	void loadFileAsync(const string &fileName, ColorList &result)
	{
		int rawSize = MAP_RESOLUTION * MAP_RESOLUTION * 3;
		vector<unsigned char> buffer(rawSize);

		HANDLE fileHandle = CreateFile(fileName.c_str(), GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
		OVERLAPPED overlapped = { 0 };
		DWORD bytesRead = 0;

		for(;;)
		{
			BOOL readResult = ReadFile(fileHandle, &buffer[0], rawSize, &bytesRead, &overlapped);
			if(!readResult)
			{
				if(GetLastError() != ERROR_IO_PENDING)
					break;

				GetOverlappedResult(fileHandle, &overlapped, &bytesRead, FALSE);
				if(GetLastError() != ERROR_IO_INCOMPLETE)
					break;
			}
			else
				break;
		}

		CloseHandle(fileHandle);

		result.resize(MAP_RESOLUTION * MAP_RESOLUTION);
		for(unsigned int y = 0; y < MAP_RESOLUTION; ++y)
		for(unsigned int x = 0; x < MAP_RESOLUTION; ++x)
		{
			int index = y * MAP_RESOLUTION + x;
			int bufferIndex = index * 3;

			result[index].r = buffer[bufferIndex];
			result[index].g = buffer[bufferIndex + 1];
			result[index].b = buffer[bufferIndex + 2];
		}
	}
	*/

	static VC2I getVisibilityPosition(const VC2 &size, const VC2I &visibilitySize, const VC2 &position)
	{
		VC2I pos;
		pos.x = int(position.x * visibilitySize.x / size.x);
		pos.y = int(position.y * visibilitySize.y / size.y);

		return pos;
	}

	struct Layer
	{
		Layer() : drawAll( false ) { }

		VC2 start;
		VC2 end;
		VC2 size;
		VC2I visibilitySize;

		VC2 areaStart;
		VC2 areaEnd;
		VC2 areaSize;

		string layerFile1;
		string layerFile2;

		scoped_ptr<FileReader> fileReader1;
		scoped_ptr<FileReader> fileReader2;

		ColorList layer1;
		ColorList layer2;
		VisibilityList visibility;

		bool drawAll;


		void generate(Game &game, const string &dir, const string &id)
		{
			if(end.x < start.x)
				std::swap(end.x, start.x);
			if(end.y < start.y)
				std::swap(end.y, start.y);

			size = end - start;
//			FB_ASSERT(size.x >= 0 && size.y >= 0);

			areaStart = start;
			areaEnd = end;
			areaSize = size;

			{
				float ratio = areaSize.x / areaSize.y;
				if(ratio > 1.f)
				{
					float x1 = 0;
					float x2 = areaSize.x;
					float y2 = float(areaSize.y * ratio);
					float y1 = (areaSize.y - y2) / 2;
					y2 += y1;

					start = VC2(x1, y1) + areaStart;
					size = VC2(x2-x1, y2-y1);
				}
				else
				{
					float y1 = 0;
					float y2 = areaSize.y;
					float x2 = float(areaSize.x / ratio);
					float x1 = (areaSize.x - x2) / 2;
					x2 += x1;

					start = VC2(x1, y1) + areaStart;
					size = VC2(x2-x1, y2-y1);
				}

				areaEnd = start + size;
			}

			visibilitySize.x = int(size.x / MAP_SMALLEST_BLOCK + .5f);
			visibilitySize.y = int(size.y / MAP_SMALLEST_BLOCK + .5f);
			visibility.resize(visibilitySize.x * visibilitySize.y);

			makeName(dir, id, layerFile1);
			layerFile1 += "_before.raw";
			makeName(dir, id, layerFile2);
			layerFile2 += "_after.raw";

// --- DISABLED BECAUSE THIS CAUSES A SIGNIFICANT FPS DROP UNLESS RENDERER IS RESET AFTERWARDS. ---
// --jpk
//#pragma message("--- FIXME: map file image generation causes fps drop, therefore disabled. ---") 
//			if(!fileExists(layerFile1))
//				generateFile(layerFile1, start, size, areaStart, areaSize, game, Before);
// --- END OF DISABLED ---
//			if(!fileExists(layerFile2))
//				generateFile(layerFile2, start, size, areaStart, areaSize, game, After);

		}

		void update(const VC2 &position)
		{
			if(position.x < start.x || position.x >= end.x)
				return;
			if(position.y < start.y || position.y >= end.y)
				return;

			VC2 realPosition = position - start;
			VC2I pos = getVisibilityPosition(size, visibilitySize, realPosition);
			FB_ASSERT(pos.x >= 0 && pos.y >= 0 && pos.x < visibilitySize.x && pos.y < visibilitySize.y);

			// ToDo:
			//  - I guess we should do something more intelligent than this

			for(int j = -2; j <= 2; ++j)
			for(int i = -2; i <= 2; ++i)
			{
				int x = pos.x + i;
				int y = pos.y + j;

				if(x < 0 || x >= visibilitySize.x)
					continue;
				if(y < 0 || y >= visibilitySize.y)
					continue;

				if(i == -2 || i == 2)
				{
					if(j == -2 || j == 2)
						continue;
				}

				visibility[y * visibilitySize.x + x] = 1;
			}

			if(fileReader1)
				fileReader1->update();
			if(fileReader2)
				fileReader2->update();
		}

		void load(Game &game, const string &dir, const string &id)
		{
			if(layerFile1.empty() || layerFile2.empty())
				return;

			fileReader1.reset(new FileReader(layerFile1, layer1));
			fileReader2.reset(new FileReader(layerFile2, layer2));
		}

		void free()
		{
			layer1.clear();
			layer2.clear();
			fileReader1.reset();
			fileReader2.reset();
		}

		void drawTo(IStorm3D_Texture &t) const
		{
			if(fileReader1)
				fileReader1->forceDataReady();
			if(fileReader2)
				fileReader2->forceDataReady();

			Storm3D_SurfaceInfo info = t.GetSurfaceInfo();
			vector<DWORD> colorBuffer(info.height * info.width);

			if(!visibility.empty())
			{
				vector<float> buffer(info.height * info.width);
				vector<float> bufferH(info.height * info.width);

				// Create screen-size visibility buffer
				{
					for(int y = 0; y < info.height; ++y)
					for(int x = 0; x < info.width; ++x)
					{
						int index = y * info.width + x;
						buffer[index] = 0.f;

						VC2I visPos;
						visPos.x = x * visibilitySize.x / info.width;
						visPos.y = y * visibilitySize.y / info.height;
						if(visPos.x < 0 || visPos.y < 0 || (visPos.x >= visibilitySize.x && visPos.y >= visibilitySize.y))
							continue;

						if(visibility[visPos.y * visibilitySize.x + visPos.x])
							buffer[index] = 1.f;

						if( drawAll )
							buffer[index] = 1.f;
					}
				}

				int samples = 11;

				// Filter first horizontally
				{
					for(int y = 0; y < info.height; ++y)
					for(int x = 0; x < info.width; ++x)
					{
						int index = y * info.width + x;
						float factor = 0.f;

						for(int i = -samples/2; i <= samples/2; ++i)
						{
							int offset = i * 2;
							int nx = x + offset;
							if(nx < 0 || nx >= info.width)
								continue;

							factor += buffer[index + offset];
						}

						factor /= samples;
						bufferH[index] = factor;
					}
				}

				// And then vertically
				{
					for(int y = 0; y < info.height; ++y)
					for(int x = 0; x < info.width; ++x)
					{
						int index = y * info.width + x;
						float factor = 0.f;

						for(int j = -samples/2; j <= samples/2; ++j)
						{
							int offset = j * 2;
							int ny = y + offset;
							if(ny < 0 || ny >= info.height)
								continue;

							factor += bufferH[index + (offset * info.height)];
						}

						factor /= samples;
						buffer[index] = factor;
					}
				}

				if(!layer1.empty() && !layer2.empty())
				{
					FB_ASSERT(layer1.size() == layer2.size());

					for(int y = 0; y < info.height; ++y)
					for(int x = 0; x < info.width; ++x)
					{
						float factor = 1.f - buffer[y * info.width + x];
						FB_ASSERT(factor >= 0.f && factor <= 1.f);

						VC2I colPos;
						colPos.x = x * MAP_RESOLUTION / info.width;
						colPos.y = y * MAP_RESOLUTION / info.height;
						FB_ASSERT(colPos.x >= 0 && colPos.y >= 0 && colPos.x < MAP_RESOLUTION && colPos.y < MAP_RESOLUTION);

						int layerIndex = colPos.y * MAP_RESOLUTION + colPos.x;
						FB_ASSERT(layerIndex >= 0 && layerIndex < int(layer1.size()) && layerIndex < int(layer2.size()));
						const TColor<unsigned char> &c1 = layer1[layerIndex];
						const TColor<unsigned char> &c2 = layer2[layerIndex];

						//int a1 = 0; //255/6;
						//int a2 = 2 * 255 / 3;

						int ri = int(factor * c1.r + (1.f - factor) * c2.r);
						int gi = int(factor * c1.g + (1.f - factor) * c2.g);
						int bi = int(factor * c1.b + (1.f - factor) * c2.b);

						if(ri > 255) ri = 255;
						if(gi > 255) gi = 255;
						if(bi > 255) bi = 255;
						if(ri < 0) ri = 0;
						if(gi < 0) gi = 0;
						if(bi < 0) bi = 0;

						unsigned char r = ri;
						unsigned char g = gi;
						unsigned char b = bi;


						//DWORD value = r << 16 | g << 8 | b | (a << 24);
						DWORD alpha = 0xAA000000;
						DWORD value = r << 16 | g << 8 | b | alpha;
						colorBuffer[(info.height - y - 1) * info.width + x] = value;
					}
				}
			}
			t.Copy32BitSysMembufferToTexture(&colorBuffer[0]);
		}

		void getPlayerCoordinates(const VC2 &ppos, float prot, VC2 &pos, float &rot) const
		{
			if(ppos.x < start.x || ppos.x >= end.x)
				return;
			if(ppos.y < start.y || ppos.y >= end.y)
				return;

			VC2 realPosition = ppos - start;
			pos.x = realPosition.x / size.x;
			pos.y = realPosition.y / size.y;
			pos.y = 1.f - pos.y;

			rot = prot + PI;
		}
	};


typedef map<string, shared_ptr<Layer> > Layers;

struct Map::Data
{
	Game &game;
	string dir;

	string activeLayer;
	Layers layers;

	bool initialized;
	int time;

	Data(Game &game_)
	:	game(game_),
		initialized(false),
		time(0)
	{
	}

	void insertLayer(const string &id)
	{
		if(layers.find(id) == layers.end())
		{
			shared_ptr<Layer> l(new Layer());
			layers[id] = l;
		}
	}

	void loadLayer(const string &id)
	{
		if(!activeLayer.empty())
			layers[activeLayer]->free();

		if(layers.find(id) == layers.end())
		{
			string message = "Unable to find map layer ";
			message += id;
			Logger::getInstance()->error(message.c_str());
			return;
		}

		layers[id]->load(game, dir, id);
		activeLayer = id;
	}

	void update(const VC2 &position, int ms)
	{
		if(!initialized)
		{
			insertLayer("default"); // TMP HACK
			Layers::iterator it = layers.begin();
			for(; it != layers.end(); ++it)
				it->second->generate(game, dir, it->first);

			initialized = true;
		}

		time += ms;
		if(time < UPDATE_INTERVAL)
			return;
		time -= UPDATE_INTERVAL;

		Layers::iterator it = layers.begin();
		for(; it != layers.end(); ++it)
			it->second->update(position);
	}

	void getPlayerCoordinates(const VC2 &ppos, float prot, VC2 &pos, float &rot)
	{
		if(activeLayer.empty())
		{
			Logger::getInstance()->error("No map layer defined. Trying to use default.");
			// try default
			insertLayer ("default"); // TMP HACK
			loadLayer("default");
		}

		if(activeLayer.empty())
		{
			Logger::getInstance()->error("No map layer defined");
			return;
		}

		Layers::const_iterator it = layers.find(activeLayer);
		if(it != layers.end())
			it->second->getPlayerCoordinates(ppos, prot, pos, rot);
	}

	void drawTo(IStorm3D_Texture &texture)
	{
		if(activeLayer.empty())
		{
			Logger::getInstance()->error("No map layer defined. Trying to use default.");
			insertLayer ("default"); // TMP HACK
			//activeLayer = "default";
			loadLayer("default");
		}

		Layers::iterator it = layers.find(activeLayer);
		if(it == layers.end())
			return;

		it->second->drawTo(texture);
	}
};

Map::Map(Game &game)
{
	scoped_ptr<Data> tempData(new Data(game));
	data.swap(tempData);
}

Map::~Map()
{
}

void Map::setMission(const std::string &dir)
{
	FB_ASSERT(!dir.empty());
	data->dir = dir;
}

void Map::startLayer(const std::string &id, const VC2 &start)
{
	FB_ASSERT(!id.empty());
	data->insertLayer(id);
	data->layers[id]->start = start;
}

void Map::endLayer(const std::string &id, const VC2 &end)
{
	FB_ASSERT(!id.empty());
	data->insertLayer(id);
	data->layers[id]->end = end;
}

void Map::loadLayer(const std::string &id)
{
	FB_ASSERT(!id.empty());
	data->insertLayer(id);
	data->loadLayer(id);
}

void Map::update(const VC2 &player, int ms)
{
	data->update(player, ms);
}

void Map::drawTo(IStorm3D_Texture &texture)
{
	data->drawTo(texture);
}

const std::string &Map::getMission() const
{
	return data->dir;
}

void Map::getLayerCoordinates(VC2 &p1, VC2 &p2) const
{
	Layers::iterator it = data->layers.find(data->activeLayer);
	if(it != data->layers.end())
	{
		p1 = it->second->areaStart;
		p2 = it->second->areaEnd;
	}
}

void Map::getPlayerCoordinates(const VC2 &ppos, float prot, VC2 &pos, float &rot)
{
	data->getPlayerCoordinates(ppos, prot, pos, rot);
}

void Map::clearMapFog()
{
	Layers::iterator it = data->layers.begin();
	for(; it != data->layers.end(); ++it)
		it->second->drawAll = true;
}

} // ui
