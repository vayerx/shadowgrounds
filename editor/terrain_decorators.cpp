// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "terrain_decorators.h"
#include "storm.h"
#include "../filesystem/output_stream.h"
#include "../filesystem/input_stream.h"
#include "../ui/decoration.h"
#include "../ui/decorationmanager.h"
#include "../ui/visualobjectmodel.h"
#include <cassert>

namespace frozenbyte {
namespace editor {
namespace {
	class DecorationInstance
	{
		ui::DecorationManager &manager;

		// this won't do, as DecorationManager owns the Decoration, and it will
		// delete it. changed to traditional pointer. -jpk
		//boost::scoped_ptr<ui::Decoration> decoration;
		ui::Decoration *decoration;

	public:
		DecorationInstance(ui::DecorationManager &manager_)
		:	manager(manager_),
			decoration(manager.createDecoration())
		{
		}

		~DecorationInstance()
		{
			//manager.deleteDecoration(decoration.get());
			manager.deleteDecoration(decoration);
		}

		ui::Decoration &get()
		{
			assert(decoration);
			return *decoration;
		}
	};

} // unnamed

struct TerrainDecoratorsData
{
	Storm &storm;

	std::string waterName[2];
	float waterHeight[2];

	ui::DecorationManager decorationManager;
	boost::scoped_ptr<DecorationInstance> waterDecoration[2];

	TerrainDecoratorsData(Storm &storm_)
	:	storm(storm_)
	{
		reset();
	}

	void createWater(int index)
	{
		if(waterName[index].empty())
			return;

		boost::scoped_ptr<DecorationInstance> decorationInstance(new DecorationInstance(decorationManager));
		ui::Decoration &decoration = decorationInstance->get();
		
		ui::VisualObjectModel::setVisualStorm(storm.storm, storm.scene);

		decoration.setHeight(waterHeight[index]);
		decoration.loadModel(waterName[index].c_str());

		if(index == 0)
			decoration.setEffect(ui::Decoration::DECORATION_EFFECT_WAVE_X, true);
		if(index == 1)
			decoration.setEffect(ui::Decoration::DECORATION_EFFECT_WAVE_Z, true);

		waterDecoration[index].swap(decorationInstance);
	}

	void reset()
	{
		for(int i = 0; i < 2; ++i)
		{
			waterName[i] = "";
			waterHeight[i] = 0;
			waterDecoration[i].reset();
		}
	}
};

TerrainDecorators::TerrainDecorators(Storm &storm)
{
	boost::scoped_ptr<TerrainDecoratorsData> tempData(new TerrainDecoratorsData(storm));
	data.swap(tempData);

	timeBeginPeriod(1);
}

TerrainDecorators::~TerrainDecorators()
{
	timeEndPeriod(1);
}

void TerrainDecorators::reset()
{
	data->reset();
}

void TerrainDecorators::setWaterModel(int index, const std::string &fileName)
{
	assert(index >= 0 && index < 2);
	data->waterName[index] = fileName;
	
	data->createWater(index);
}

void TerrainDecorators::setWaterHeight(int index, float height)
{
	assert(index >= 0 && index < 2);
	data->waterHeight[index] = height;

	data->createWater(index);
}

const std::string &TerrainDecorators::getWaterName(int index) const
{
	assert(index >= 0 && index < 2);
	return data->waterName[index];
}

float TerrainDecorators::getWaterHeight(int index) const
{
	assert(index >= 0 && index < 2);
	return data->waterHeight[index];
}

void TerrainDecorators::tick()
{
	// Hackety hack
	static unsigned int lastTime = timeGetTime();
	static int timeSinceLastUpdate = 0;
	unsigned int currentTime = timeGetTime();

	timeSinceLastUpdate += (currentTime - lastTime);

	for(int i = 0; i < timeSinceLastUpdate / 10; ++i)
		data->decorationManager.run();

	timeSinceLastUpdate %= 10;
	lastTime = currentTime;
}

void TerrainDecorators::doExport(Exporter &exporter) const
{
}

filesystem::OutputStream &TerrainDecorators::writeStream(filesystem::OutputStream &stream) const
{
	stream << int(1);
	for(int i = 0; i < 2; ++i)
		stream << data->waterName[i] << data->waterHeight[i];

	return stream;
}

filesystem::InputStream &TerrainDecorators::readStream(filesystem::InputStream &stream)
{
	int version = 0;
	stream >> version;

	if(version == 0)
		return stream;

	for(int i = 0; i < 2; ++i)
	{
		stream >> data->waterName[i] >> data->waterHeight[i];
		data->createWater(i);
	}

	return stream;
}

} // end of namespace editor
} // end of namespace frozenbyte
