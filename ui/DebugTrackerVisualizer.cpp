
#include "precompiled.h"

#include "DebugTrackerVisualizer.h"

#include "../game/tracking/ObjectTracker.h"
#include "../game/tracking/ITrackerObject.h"
#include "../game/tracking/ITrackerObjectType.h"
#include <vector>
#include <Storm3D_UI.h>

using namespace game::tracking;

extern IStorm3D_Scene *disposable_scene;

namespace ui
{

	void DebugTrackerVisualizer::visualizeTrackers(game::tracking::ObjectTracker *objectTracker)
	{
		assert(objectTracker != NULL);

		std::vector<ITrackerObject *> trackerList = objectTracker->getAllTrackers();

		for (int i = 0; i < (int)trackerList.size(); i++)
		{
			ITrackerObject *tracker = trackerList[i];

			std::string trackerName = tracker->getType()->getTrackerTypeName();
			
			COL col = COL(0,0,1);

			VC3 pos = tracker->getTrackerPosition();
			VC3 sizes = VC3(0.1f, 0.1f, 0.1f);

			if (trackerName == "projectile")
			{
				col = COL(1.0f,0.7f,0.0f);
				sizes *= 1.5f;
			}

			VC3 c1 = VC3(pos.x - sizes.x, pos.y + sizes.y, pos.z - sizes.z);
			VC3 c2 = VC3(pos.x + sizes.x, pos.y + sizes.y, pos.z - sizes.z);
			VC3 c3 = VC3(pos.x + sizes.x, pos.y + sizes.y, pos.z + sizes.z);
			VC3 c4 = VC3(pos.x - sizes.x, pos.y + sizes.y, pos.z + sizes.z);
			VC3 cb1 = VC3(pos.x - sizes.x, pos.y - sizes.y, pos.z - sizes.z);
			VC3 cb2 = VC3(pos.x + sizes.x, pos.y - sizes.y, pos.z - sizes.z);
			VC3 cb3 = VC3(pos.x + sizes.x, pos.y - sizes.y, pos.z + sizes.z);
			VC3 cb4 = VC3(pos.x - sizes.x, pos.y - sizes.y, pos.z + sizes.z);

			float extra_offset = 0.0f + (i / 200.0f);
			VC3 cb4_extra = VC3(pos.x - sizes.x - extra_offset, pos.y - sizes.y, pos.z + sizes.z);

			disposable_scene->AddLine(c1, c2, col);
			disposable_scene->AddLine(c2, c3, col);
			disposable_scene->AddLine(c3, c4, col);
			disposable_scene->AddLine(c4, c1, col);
			disposable_scene->AddLine(cb1, cb2, col);
			disposable_scene->AddLine(cb2, cb3, col);
			disposable_scene->AddLine(cb3, cb4, col);
			disposable_scene->AddLine(cb4, cb1, col);
			disposable_scene->AddLine(c1, cb1, col);
			disposable_scene->AddLine(c2, cb2, col);
			disposable_scene->AddLine(c3, cb3, col);
			disposable_scene->AddLine(c4, cb4, col);
			disposable_scene->AddLine(c4, cb4, col);

			COL col2 = COL(0,1,1);
			disposable_scene->AddLine(c4, cb4_extra, col2);

		}
	}

}
