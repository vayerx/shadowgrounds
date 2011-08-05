// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "helper_visualization.h"
#include "terrain_units.h"
#include "storm.h"

#include <istorm3d.h>
#include <istorm3d_line.h>
#include <istorm3d_terrain.h>
#include <vector>
#include <boost/shared_ptr.hpp>

namespace frozenbyte {
namespace editor {
namespace {

template<class T>
struct LineDeleter
{
	Storm &storm;

	LineDeleter(Storm &storm_)
	:	storm(storm_)
	{
	}

	void operator()(T *pointer)
	{
		storm.scene->RemoveLine(pointer);
		delete pointer;
	}
};

} // unnamed

struct HelperVisualizationData
{
	Storm &storm;
	std::vector<boost::shared_ptr<IStorm3D_Line> > lines;

	HelperVisualizationData(Storm &storm_)
	:	storm(storm_)
	{
	}

	boost::shared_ptr<IStorm3D_Line> createNewLine(DWORD color)
	{
		boost::shared_ptr<IStorm3D_Line> line(storm.storm->CreateNewLine(), LineDeleter<IStorm3D_Line>(storm));
		line->SetThickness(.25f);
		line->SetColor(color);

		storm.scene->AddLine(line.get(), true);
		lines.push_back(line);
		
		return line;
	}

	VC3 getPosition(const VC2 &pointPosition)
	{
		//float height = storm.terrain->getHeight(pointPosition);
		float height = storm.getHeight(pointPosition);
		return VC3(pointPosition.x, height + .2f, pointPosition.y);
	}

	void createCross(const VC2 &pointPosition, float size, DWORD color)
	{
		//float height = storm.terrain->GetHeightAt(pointPosition);
		//VC3 position(pointPosition.x, height + .2f, pointPosition.y);
		VC3 position = getPosition(pointPosition);

		boost::shared_ptr<IStorm3D_Line> line1 = createNewLine(color);
		VC3 delta1 = VC3(size, 0, 0);
		line1->AddPoint(position + delta1);
		line1->AddPoint(position - delta1);

		boost::shared_ptr<IStorm3D_Line> line2 = createNewLine(color);
		VC3 delta2 = VC3(0, 0, size);
		line2->AddPoint(position + delta2);
		line2->AddPoint(position - delta2);
	}
};

HelperVisualization::HelperVisualization(Storm &storm)
{
	boost::scoped_ptr<HelperVisualizationData> tempData(new HelperVisualizationData(storm));
	data.swap(tempData);
}

HelperVisualization::~HelperVisualization()
{
}

void HelperVisualization::clear()
{
	data->lines.clear();
}

void HelperVisualization::visualize(const UnitHelpers &helpers, int activeIndex, const VC2 &unitPosition)
{
	data->lines.clear();

	for(int i = 0; i < helpers.getHelperAmount(); ++i)
	{
		DWORD color = (i == activeIndex) ? 0xFFFF0000 : 0xFFAA0000;

		VC2 startPoint = unitPosition;

		for(int j = 0; j < helpers.getPointAmount(i); ++j)
		{
			if(j > 0)
				startPoint = helpers.getPoint(i, j-1);

			boost::shared_ptr<IStorm3D_Line> line = data->createNewLine(color);
			line->SetThickness(.15f);

			line->AddPoint(data->getPosition(startPoint));
			line->AddPoint(data->getPosition(helpers.getPoint(i, j)));

			data->createCross(helpers.getPoint(i, j), .5f, color);
		}
	}
}

} // end of namespace editor
} // end of namespace frozenbyte
