
#ifndef DEBUGTRACKERVISUALIZER_H
#define DEBUGTRACKERVISUALIZER_H

namespace game
{
namespace tracking
{
	class ObjectTracker;
}
}

namespace ui
{
	class DebugTrackerVisualizer
	{
	public:
		static void visualizeTrackers(game::tracking::ObjectTracker *objectTracker);
	};
}

#endif
