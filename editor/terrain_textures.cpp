// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "terrain_textures.h"
#include "storm.h"
#include "storm_texture.h"
#include "exporter.h"
#include "exporter_scene.h"
#include "../ui/terrain_legacy.h"
#include "../filesystem/output_stream.h"
#include "../filesystem/input_stream.h"
#include "../filesystem/input_file_stream.h"
#include "../filesystem/output_file_stream.h"
#include "../filesystem/memory_stream.h"

#include <boost/shared_ptr.hpp>
#include <map>
#include <cassert>
#include <istorm3d_texture.h>
#include <istorm3d.h>

namespace frozenbyte {
namespace editor {
namespace {
	static const int BLOCK_SIZE = IStorm3D_Terrain::BLOCK_SIZE;
	static const int MAX_OPTIMIZE_LAYERS = 4;

	struct OptimizeLayer
	{
		int index;
		int order;

		OptimizeLayer()
		:	index(0),
			order(0)
		{
		}
	};

	struct OptimizeLayerSorter
	{
		bool operator () (const OptimizeLayer &a, const OptimizeLayer &b) const
		{
			return a.order > b.order;
		}
	};

	struct BlendPass
	{
		boost::shared_ptr<IStorm3D_Texture> weightTexture;
		std::vector<unsigned char> buffer;

		int textureA;
		int textureB;

		bool needCopy;
		bool setTerrain;

		BlendPass()
		:	textureA(-1),
			textureB(-1),
			needCopy(false),
			setTerrain(false)
		{
			buffer.resize(BLOCK_SIZE * BLOCK_SIZE * 4);
		}

		void setPixel(int x, int y, int textureIndex, int weight)
		{
			int position = (y * BLOCK_SIZE + x) * 4;
			needCopy = true;

			if(textureIndex == textureA)
			{
				buffer[position] = weight;
				buffer[position + 1] = weight;
				buffer[position + 2] = weight;
			}
			else
				buffer[position + 3] = weight;
		}

		void update(Storm &storm, int blockIndex)
		{
			if(!weightTexture)
				weightTexture = createTexture(BLOCK_SIZE, BLOCK_SIZE, storm);

			if(needCopy)
			{
				weightTexture->Copy32BitSysMembufferToTexture(reinterpret_cast<DWORD *> (&buffer[0]));
				needCopy = false;
			}

			if(!setTerrain)
			{
				assert(weightTexture);

				storm.terrain->setBlendMap(blockIndex, *weightTexture.get(), textureA, textureB);
				setTerrain = true;
			}
		}
	};

	struct BlendStorage
	{
		std::vector<unsigned char> weights;
	};

	struct BlendBlock
	{
		std::vector<BlendPass> passes;
		std::vector<int> textureList;

		bool updateBlock;

		BlendBlock()
		:	updateBlock(true)
		{
		}

		void setPixel(int textureIndex, int weight, int x, int y, bool forceSingleTexture)
		{
			assert(textureIndex >= 0);

			int texturePass = -1;

			if(!forceSingleTexture)
			{
				for(unsigned int t = 0; t < textureList.size(); ++t)
				{
					if(textureList[t] == textureIndex)
					{
						texturePass = t / 2;
						break;
					}
				}

				if(texturePass == -1)
				{
					if(textureList.size() % 2 == 1)
					{
						int index = textureList.size() / 2;
						passes[index].textureB = textureIndex;
						texturePass = index;
					}
					else
					{
						passes.resize(passes.size() + 1);
						texturePass = passes.size() - 1;

						passes[texturePass].textureA = textureIndex;
					}

					textureList.push_back(textureIndex);
					updateBlock = true;
				}
			}
			else
			{
				for(unsigned int t = 0; t < textureList.size(); ++t)
				{
					if(textureList[t] == textureIndex)
					{
						texturePass = t;
						break;
					}
				}

				if(texturePass == -1)
				{
					passes.resize(passes.size() + 1);
					texturePass = passes.size() - 1;
					
					passes[texturePass].textureA = textureIndex;
					textureList.push_back(textureIndex);

					updateBlock = true;
				}
			}

			assert(texturePass != -1);

			BlendPass &pass = passes[texturePass];
			pass.setPixel(x, y, textureIndex, weight);
		}

		void insertTexture(int index)
		{
			for(unsigned int i = 0; i < passes.size(); ++i)
			{
				BlendPass &pass = passes[i];

				if(pass.textureA >= index)
					++pass.textureA;
				if(pass.textureB >= index)
					++pass.textureB;
			}

			for(unsigned int j = 0; j < textureList.size(); ++j)
				if(textureList[j] >= index)
					++textureList[j];

			updateBlock = true;
		}

		void removeTexture(int index)
		{
			for(unsigned int i = 0; i < passes.size(); ++i)
			{
				BlendPass &pass = passes[i];

				if(pass.textureA >= index)
					--pass.textureA;
				if(pass.textureB >= index)
					--pass.textureB;
			}

			for(unsigned int j = 0; j < textureList.size(); ++j)
				if(textureList[j] >= index)
					--textureList[j];

			updateBlock = true;
		}

		void update(Storm &storm, int blockIndex)
		{
			if(!storm.terrain)
				return;

			if(updateBlock)
				storm.terrain->resetBlends(blockIndex);

			for(unsigned int i = 0; i < passes.size(); ++i)
			{
				BlendPass &pass = passes[i];

				if(updateBlock)
					pass.setTerrain = false;

				pass.update(storm, blockIndex);
			}
		}
	};

	struct BlendMap
	{
		// [textureIndex] -> blends
		std::vector<BlendStorage> blendings;
		//std::vector<unsigned char> pixelCount;
		VC2I size;

		std::vector<BlendBlock> blocks;
		VC2I blockAmount;

		bool forceSingleTexture;

		void init(int textureAmount)
		{
			blocks.resize(blockAmount.x * blockAmount.y);
			blendings.resize(textureAmount);

			for(unsigned int i = 0; i < blendings.size(); ++i)
				blendings[i].weights.resize(size.x * size.y);
		}

		void setPixel(const VC2I &blockIndex, const VC2I &blockPosition, int textureIndex, int weight)
		{
			int finalBlockIndex = blockIndex.y * blockAmount.x + blockIndex.x;
			blocks[finalBlockIndex].setPixel(textureIndex, weight, blockPosition.x, blockPosition.y, forceSingleTexture);

			// Keep blocks in sync
			if(blockPosition.x == 0 && blockIndex.x > 0)
				blocks[finalBlockIndex - 1].setPixel(textureIndex, weight, BLOCK_SIZE - 1, blockPosition.y, forceSingleTexture);
			if(blockPosition.y == 0 && blockIndex.y > 0)
				blocks[finalBlockIndex - blockAmount.x].setPixel(textureIndex, weight, blockPosition.x, BLOCK_SIZE - 1, forceSingleTexture);

			if(blockPosition.y == 0 && blockIndex.y > 0)
			if(blockPosition.x == 0 && blockIndex.x > 0)
				blocks[finalBlockIndex - blockAmount.x - 1].setPixel(textureIndex, weight, BLOCK_SIZE - 1, BLOCK_SIZE - 1, forceSingleTexture);


			assert(blockPosition.x < BLOCK_SIZE - 1);
			assert(blockPosition.y < BLOCK_SIZE - 1);
		}

		BlendMap()
		:	forceSingleTexture(false)
		{
		}

		void create(const VC2I &size_, int textureAmount)
		{
			clear();
			size = size_;
			
			blockAmount = size / (BLOCK_SIZE - 1);
			init(textureAmount);
		}

		void setSingleTexturing(bool forceSingleTexture_)
		{
			forceSingleTexture = forceSingleTexture_;
		}

		void addSplat(int textureIndex, const TextureSplat &splat)
		{
			if(blendings.empty())
				return;

			BlendStorage &storage = blendings[textureIndex];
			if(storage.weights.empty())
				storage.weights.resize(size.x * size.y);

			int limitY = splat.size.y;
			int limitX = splat.size.x;

			if(limitX + splat.position.x >= size.x - 2)
				limitX = size.x - 2 - splat.position.x;
			if(limitY + splat.position.y >= size.y - 2)
				limitY = size.y - 2 - splat.position.y;

			for(int j = 0; j < limitY; ++j)
			for(int i = 0; i < limitX; ++i)
			{
				int positionX = splat.position.x + i;
				int positionY = j + splat.position.y;
				int position = positionY * size.x + positionX;

				if(positionX < 0 || positionX >= size.x)
					continue;
				if(positionY < 0 || positionX >= size.x)
					continue;

				int oldWeight = storage.weights[position];
				int newWeight = splat.weights[j * splat.size.x + i];
				if(newWeight < oldWeight)
					continue;

				VC2I blockIndex(positionX / (BLOCK_SIZE - 1), positionY / (BLOCK_SIZE - 1));
				VC2I blockPosition(positionX - blockIndex.x * (BLOCK_SIZE - 1), positionY - blockIndex.y * (BLOCK_SIZE - 1));
				
				//if(blockPosition.x < 0 || blockPosition.y < 0)
				//	continue;
				//if(blockPosition.x >= blockAmount.x || blockPosition.y >= blockAmount.y)
				//	continue;

				// Normalize old weights
				{
					int sum = 255 - newWeight;
					for(unsigned int k = 0; k < blendings.size(); ++k)
					{
						int original = blendings[k].weights[position];
						blendings[k].weights[position] = sum * blendings[k].weights[position] / 255;

						if(original)
							setPixel(blockIndex, blockPosition, k, blendings[k].weights[position]);
					}

					int newSum = 0;
					for(unsigned int k = 0; k < blendings.size(); ++k)
					{
						if(int(k) != textureIndex)
							newSum += blendings[k].weights[position];
					}

					int diff = sum - newSum;
					newWeight += diff;
				}

				storage.weights[position] = newWeight;
				setPixel(blockIndex, blockPosition, textureIndex, newWeight);
			}
		}

		void update(Storm &storm)
		{
			if(blocks.empty())
				return;

			for(int j = 0; j < blockAmount.y; ++j)
			for(int i = 0; i < blockAmount.x; ++i)
			{
				int index = j * blockAmount.x + i;
				blocks[index].update(storm, index);
			}
		}

		void optimize()
		{
			if(size.x == 0 || size.y == 0)
				return;

			boost::shared_ptr<filesystem::MemoryStreamBuffer> streamBuffer(new filesystem::MemoryStreamBuffer());
			filesystem::OutputStream outputStream;
			outputStream.setBuffer(streamBuffer);
			
			filesystem::InputStream inputStream;
			inputStream.setBuffer(streamBuffer);

			for(int yb = 0; yb < blockAmount.y; ++yb)
			for(int xb = 0; xb < blockAmount.x; ++xb)
			{
				int xo = xb * (BLOCK_SIZE - 1);
				int yo = yb * (BLOCK_SIZE - 1);

				// Collect texture influences
				std::vector<OptimizeLayer> amounts(blendings.size());
				{
					for(unsigned int i = 0; i < blendings.size(); ++i)
					{
						amounts[i].index = i;
						BlendStorage &blend = blendings[i];

						for(int y = 0; y < BLOCK_SIZE; ++y)
						for(int x = 0; x < BLOCK_SIZE; ++x)
						{
							int index = (yo + y) * size.x + xo + x;
							int value = blend.weights[index];
							/*
							if(value < 50)
							{
								value = 0;
								blend.weights[index] = 0;
							}
							*/

							amounts[i].order += value;
						}
					}

					std::sort(amounts.begin(), amounts.end(), OptimizeLayerSorter());
				}

				// Normalize out all others
				{
					for(int y = 0; y < BLOCK_SIZE; ++y)
					for(int x = 0; x < BLOCK_SIZE; ++x)
					{
						int xp = xo + x;
						int yp = yo + y;
						if(xp < 0 || yp < 0 || xp >= size.x || yp >= size.y)
							continue;

						int index = yp * size.x + xp;
						int layerAmount = min(MAX_OPTIMIZE_LAYERS, int(amounts.size()));
						if(!layerAmount)
							continue;

						// Find highest amount of used textures (default 0) and normalize by
						// adding missing blend weights to it
						int highest = amounts[0].index;
						int highestAmount = blendings[highest].weights[index];
						int combined = 0;

						if(highestAmount < 255)
							int a = 0;

						int i = 0;
						for(i = 0; i < layerAmount; ++i)
						{
							int current = blendings[amounts[i].index].weights[index];
							combined += current;

							if(current > highestAmount)
							{
								highest = amounts[i].index;
								highestAmount = current;
							}
						}

						for(i = layerAmount; i < int(amounts.size()); ++i)
							blendings[amounts[i].index].weights[index] = 0;

						blendings[highest].weights[index] += 255 - combined;
					}
				}


			}

			/*
			// highest weight, texture index pairs for normalization
			std::vector<std::pair<int, int> > weights;
			// Amount of weights removed from given pixel
			std::vector<unsigned char> weightSub;

			weights.resize(size.x * size.y, std::pair<int, int> (-1, 0));
			weightSub.resize(size.x * size.y);

			for(unsigned int i = 0; i < blendings.size(); ++i)
			{
				BlendStorage &blend = blendings[i];

				for(unsigned int j = 0; j < blend.weights.size(); ++j)
				{
					std::pair<int, int> currentWeight(blend.weights[j], i);
					weights[j] = std::max(weights[j], currentWeight);

					if(blend.weights[j] < 75)
					{
						weightSub[j] += blend.weights[j];
						blend.weights[j] = 0;
					}
				}
			}

			// Normalize
			for(unsigned int j = 0; j < weightSub.size(); ++j)
			{
				std::pair<int, int> &currentWeight = weights[j];
				if(currentWeight.second < 0)
					continue;

				blendings[currentWeight.second].weights[j] += weightSub[j];
			}
			*/

			write(outputStream);
			read(inputStream);
		}

		void clear()
		{
			blendings = std::vector<BlendStorage>();
			blocks = std::vector<BlendBlock> ();
		}

		void insertTexture(int index)
		{
			blendings.resize(blendings.size() + 1);
			for(int i = blendings.size() - 1; i > index; --i)
				blendings[i] = blendings[i - 1];

			blendings[index].weights.clear();
			blendings[index].weights.resize(size.x * size.y);

			for(unsigned int j = 0; j < blocks.size(); ++j)
				blocks[j].insertTexture(index);
		}

		void removeTexture(int index)
		{
			blendings.erase(blendings.begin() + index);

			for(unsigned int j = 0; j < blocks.size(); ++j)
				blocks[j].removeTexture(index);
		}

		void read(filesystem::InputStream &stream)
		{
			clear();

			int version = 0;
			stream >> version;
			if(!version)
				return;

			int blendAmount = 0;
			if(version == 1)
			{
				// Old version, double sized blocks

				stream >> size.x >> size.y;
				stream >> blockAmount.x >> blockAmount.y;

				VC2I oldSize = size;
				//size.x += blockAmount.x;
				//size.y += blockAmount.y;
				blockAmount.x *= 2;
				blockAmount.y *= 2;
				size.x = blockAmount.x * (BLOCK_SIZE - 1) + 1;
				size.y = blockAmount.y * (BLOCK_SIZE - 1) + 1;

				stream >> blendAmount;

				init(blendAmount);
				std::vector<unsigned char> buffer(oldSize.x * oldSize.y);
				for(int i = 0; i < blendAmount; ++i)
				{
					/*
					std::vector<unsigned char> &weights = blendings[i].weights;
					assert(!weights.empty());

					for(int x = 0; x < oldSize.x; ++x)
					for(int y = 0; y < oldSize.y; ++y)
					{
						unsigned char value = 0;
						stream >> value;

						if(x < size.x && y < size.y)
							weights[y * size.x + x] = value;
					}
					*/

					for(unsigned int j = 0; j < buffer.size(); ++j)
						stream >> buffer[j];

					std::vector<unsigned char> &weights = blendings[i].weights;
					assert(!weights.empty());

					for(int x = 0; x < size.x; ++x)
					for(int y = 0; y < size.y; ++y)
					{
						int destIndex = y * size.x + x;

						int xo = x * oldSize.x / size.x;
						int yo = y * oldSize.y / size.y;
						float xf = float(x) * oldSize.x / size.x;
						float yf = float(y) * oldSize.y / size.y;
						float xd = xf - xo;
						float yd = yf - yo;

						int val1 = buffer[yo * oldSize.x + xo];
						int val2 = buffer[yo * oldSize.x + (xo + 1)];
						int val3 = buffer[(yo + 1) * oldSize.x + xo];
						int val4 = buffer[(yo + 1) * oldSize.x + (xo + 1)];
						int value = 0;

						if(xd + yd <= 1.f)
							value = int(val1 + (val2 - val1) * xd + (val3 - val1) * yd);
						else
							value = int(val4 + (val2 - val4) * (1.f - xd) + (val3 - val4) * (1.f - yd));

						//int sourceIndex = (y * oldSize.y / size.y) * oldSize.x + (x * oldSize.x / size.x);
						//weights[destIndex] = buffer[sourceIndex];

						weights[destIndex] = value;
					}
				}
			}
			else
			{
				stream >> size.x >> size.y;
				stream >> blockAmount.x >> blockAmount.y;

				stream >> blendAmount;

				init(blendAmount);
				for(int i = 0; i < blendAmount; ++i)
				{
					std::vector<unsigned char> &weights = blendings[i].weights;
					assert(!weights.empty());

					//for(unsigned int j = 0; j  < weights.size(); ++j)
					//	stream >> weights[j];
					stream.read(&weights[0], weights.size());
				}
			}

			if(!blendAmount)
				return;

			for(int j = 0; j < size.y - 2; ++j)
			for(int i = 0; i < size.x - 2; ++i)
			{
				int position = j * size.x + i;

				VC2I blockIndex(i / (BLOCK_SIZE - 1), j / (BLOCK_SIZE - 1));
				//VC2I blockPosition(i - blockIndex.x * (BLOCK_SIZE - 1), j - blockIndex.y * (BLOCK_SIZE - 1));
				VC2I blockPosition(i % (BLOCK_SIZE - 1), j % (BLOCK_SIZE - 1));

				for(int k = 0; k < blendAmount; ++k)
				{
					int weight = blendings[k].weights[position];
					if(weight)
						setPixel(blockIndex, blockPosition, k, weight);
				}
			}
		}

		void write(filesystem::OutputStream &stream) const
		{
			stream << int(2);

			stream << size.x << size.y;
			stream << blockAmount.x << blockAmount.y;
			stream << int(blendings.size());

			for(unsigned int i = 0; i < blendings.size(); ++i)
			{
				const std::vector<unsigned char> &weights = blendings[i].weights;
				assert(!weights.empty());

				for(unsigned int j = 0; j  < weights.size(); ++j)
					stream << weights[j];
			}
		}

		const VC2I &getSize() const
		{
			return size;
		}

		bool canRemove(int index) const
		{
			if(blendings.empty())
				return true;

			const BlendStorage &blend = blendings[index];

			for(unsigned int j = 0; j < blend.weights.size(); ++j)
			{
				if(blend.weights[j] > 0)
					return false;
			}

			return true;
		}

		void doExport(ExporterScene &scene)
		{
			for(unsigned int k = 0; k < blendings.size(); ++k)
			{
				for(int j = 0; j < blockAmount.y; ++j)
				for(int i = 0; i < blockAmount.x; ++i)
				{
					std::vector<unsigned char> weights(BLOCK_SIZE * BLOCK_SIZE);
					
					for(int y = 0; y < BLOCK_SIZE; ++y)
					for(int x = 0; x < BLOCK_SIZE; ++x)
					{
						int positionX = i * (BLOCK_SIZE - 1) + x;
						int positionY = j * (BLOCK_SIZE - 1) + y;

						int position = positionY * size.x + positionX;
						weights[y * BLOCK_SIZE + x] = blendings[k].weights[position];
					}

					scene.setBlock(j * blockAmount.x + i, k, weights);
				}
			}
		}
	};
} // unnamed

struct TerrainTexturesData
{
	Storm &storm;

	typedef std::map<std::string, boost::shared_ptr<IStorm3D_Texture> > TextureContainer;
	TextureContainer textures;

	std::string terrainTexture[2];
	std::string bottomTexture;

	BlendMap blendMap;

	TerrainTexturesData(Storm &storm_)
	:	storm(storm_),
		blendMap()
	{
	}

	void clear()
	{
		textures.clear();
		blendMap.clear();

		terrainTexture[0].clear();
		terrainTexture[1].clear();
		bottomTexture.clear();
	}

	void applyToTerrain()
	{
		//blendMap.setSingleTexturing(storm.terrain->legacyTexturing());
		resetTextures();
		blendMap.update(storm);
	}

	void writeStream(filesystem::OutputStream &stream) const
	{
		stream << int(1);
		stream << int(textures.size());

		TextureContainer::const_iterator it = textures.begin();
		for(; it != textures.end(); ++it)
			stream << it->first;

		stream << terrainTexture[0] << terrainTexture[1] << bottomTexture;
		blendMap.write(stream);	
	}

	void readStream(filesystem::InputStream &stream)
	{
		int version = 0;
		stream >> version;

		if(version == 0)
		{
			for(int i = 0; i < 5; ++i)
			for(int j = 0; j < 3; ++j)
			{
				std::string foo;
				stream >> foo;
			}

			return;
		}

		int textureAmount = 0;
		stream >> textureAmount;

		for(int i = 0; i < textureAmount; ++i)
		{
			std::string fileName;
			stream >> fileName;

			textures[fileName] = loadTexture(fileName, storm);
		}

		stream >> terrainTexture[0] >> terrainTexture[1] >> bottomTexture;
		blendMap.read(stream);	
	}

	void resetTextures()
	{
		if(!storm.terrain)
			return;

		storm.terrain->removeTerrainTextures();

		TextureContainer::iterator it = textures.begin();
		for(; it != textures.end(); ++it)
			storm.terrain->addTerrainTexture(*it->second.get());
	}
};

TerrainTextures::TerrainTextures(Storm &storm)
{
	boost::scoped_ptr<TerrainTexturesData> tempData(new TerrainTexturesData(storm));
	data.swap(tempData);
}

TerrainTextures::~TerrainTextures()
{
}

void TerrainTextures::addTexture(const std::string &fileName)
{
	if(data->textures.find(fileName) != data->textures.end())
		return;

	data->textures[fileName] = loadTexture(fileName, data->storm);

	int textureIndex = 0;
	for(TerrainTexturesData::TextureContainer::iterator it = data->textures.begin(); it != data->textures.end(); ++it)
	{
		if(it->first == fileName)
		{
			data->blendMap.insertTexture(textureIndex);
			break;
		}

		++textureIndex;
	}

	data->resetTextures();
	data->blendMap.update(data->storm);
}

void TerrainTextures::removeTexture(int index)
{
	assert((index >= 0) && (index < int(data->textures.size())));
	int textureIndex = 0;

	if(data->textures.size() < 2)
		return;

	// Should really remove & renormalize 
	// -> currently just do nothing if set to terrain
	if(!data->blendMap.canRemove(index))
		return;

	for(TerrainTexturesData::TextureContainer::iterator it = data->textures.begin(); it != data->textures.end(); ++it)
	{
		if(textureIndex++ == index)
		{
			data->textures.erase(it);
			break;
		}
	}

	data->blendMap.removeTexture(index);
	data->resetTextures();
	data->blendMap.update(data->storm);
}

void TerrainTextures::clear()
{
	data->clear();
}

void TerrainTextures::applyToTerrain()
{
	data->applyToTerrain();
}

int TerrainTextures::getTextureCount() const
{
	return data->textures.size();
}

std::string TerrainTextures::getTexture(int index) const
{
	assert((index >= 0) && (index < int(data->textures.size())));
	int textureIndex = 0;

	for(TerrainTexturesData::TextureContainer::iterator it = data->textures.begin(); it != data->textures.end(); ++it)
		if(textureIndex++ == index)
			return (*it).first;

	return "";
}

void TerrainTextures::optimize()
{
	if(data->blendMap.blendings.empty() || data->textures.empty())
		return;

	data->blendMap.optimize();
	data->blendMap.update(data->storm);
}

void TerrainTextures::resetBlendMap(const VC2I &size)
{
	data->blendMap.create(size, data->textures.size());
}

void TerrainTextures::addSplat(int textureIndex, const TextureSplat &splat)
{
	data->blendMap.addSplat(textureIndex, splat);
	data->blendMap.update(data->storm);
}

const VC2I &TerrainTextures::getMapSize() const
{
	return data->blendMap.getSize();
}

void TerrainTextures::doExport(Exporter &exporter) const
{
	ExporterScene &scene = exporter.getScene();

	for(TerrainTexturesData::TextureContainer::iterator it = data->textures.begin(); it != data->textures.end(); ++it)
		scene.addTexture(it->first);

	data->blendMap.doExport(scene);
}

filesystem::OutputStream &TerrainTextures::writeStream(filesystem::OutputStream &stream) const
{
	data->writeStream(stream);
	return stream;
}

filesystem::InputStream &TerrainTextures::readStream(filesystem::InputStream &stream)
{
	data->clear();
	data->readStream(stream);

	return stream;
}

} // end of namespace editor
} // end of namespace frozenbyte
