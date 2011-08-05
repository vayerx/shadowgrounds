
#include "precompiled.h"

#include "CircleAreaTracker.h"
#include "ClippedCircle.h"
#include "ITrackable.h"
#include "ITriggerListener.h"
#include "assert.h"
#include <boost/shared_ptr.hpp>
#include <c2_qtree.h>
#include <vector>

using namespace boost;
using namespace std;

namespace util {

	struct Trackable
	{
		ITrackable *ptr;
		int triggers;

		Trackable()
		:	ptr(0),
			triggers(0)
		{
		}
	};

	typedef vector<shared_ptr<Trackable> > TrackableList;

	struct Trigger
	{
		ClippedCircle circle;
		ITriggerListener *listener;
		TrackableList trackables;

		Quadtree<Trigger>::Entity *entity;
		int id;

		void *triggerData;

		Trigger()
		:	listener(0),
			entity(0),
			id(-1),
			triggerData(0)
		{
		}

		void SphereCollision(const VC3 &position, float radius, Storm3D_CollisionInfo &info, bool) const
		{
			// FIXME:
			// Doing only point vs clipped circle here

			if(circle.isInsideArea(position))
				info.hit = true;
		}

		bool fits(const AABB &area) const
		{
			return false;
		}
	};

	typedef vector<shared_ptr<Trigger> > TriggerList;
	typedef Quadtree<Trigger> QTree;

	static const int UPDATE_INTERVAL = 200;
	static const float HEIGHT = 15.f;

	struct ITrackableComparator
	{
		ITrackable *given;

		explicit ITrackableComparator(ITrackable *given_)
		:	given(given_) {}

		bool operator () (const shared_ptr<Trackable> &ptr) const
		{
			return given == ptr->ptr;
		}
	};

	struct TrackableComparator
	{
		Trackable *given;

		explicit TrackableComparator(Trackable *given_)
		:	given(given_) {}

		bool operator () (const shared_ptr<Trackable> &ptr) const
		{
			return given == ptr.get();
		}
	};


struct CircleAreaTracker::Data
{
	scoped_ptr<QTree> tree;
	TriggerList triggers;
	TrackableList trackables;

	int time;

	Data()
	:	time(0)
	{
	}

	void init(const VC2 &size)
	{
		VC2 mmin(-size.x, -size.y);
		VC2 mmax( size.x,  size.y);

		tree.reset(new QTree(mmin, mmax));
	}

	int insert(const ClippedCircle &circle, ITriggerListener *listener, void *triggerData)
	{
		shared_ptr<Trigger> trigger;
		int index = -1;

		for(unsigned int i = 0; i < triggers.size(); ++i)
		{
			FB_ASSERT(triggers[i]);

			if(!triggers[i]->listener || !triggers[i]->entity)
			{
				index = i;
				break;
			}
		}

		if(index == -1)
		{
			index = triggers.size();
			triggers.resize(index + 1);

			triggers[index].reset(new Trigger());
		}

		VC3 position = circle.getPosition();
		position.y = HEIGHT;

		trigger = triggers[index];
		trigger->id = index;
		trigger->listener = listener;
		trigger->circle = circle;
		trigger->entity = tree->insert(trigger.get(), position, circle.getRadius());
		trigger->triggerData = triggerData;

		FB_ASSERT(trigger->entity);
		return index;
	}

	void addTrackable(int id, ITrackable *ptr)
	{
		// Note:
		// Should we check if trying to insert same trackable multiple times?

		FB_ASSERT(id >= 0 && id < int(triggers.size()) && triggers[id]);
		Trigger &trigger = *triggers[id];

		int index = -1;
		TrackableList::iterator it = find_if(trackables.begin(), trackables.end(), ITrackableComparator(ptr));
		if(it != trackables.end())
			index = it - trackables.begin();
		if(index == -1)
		{
			for(unsigned int i = 0; i < trackables.size(); ++i)
			{
				if(!trackables[i]->triggers)
				{
					index = i;
					break;
				}
			}
		}
		if(index == -1)
		{
			index = trackables.size();
			trackables.resize(index + 1);
			trackables[index].reset(new Trackable());
		}

		shared_ptr<Trackable> &trackable = trackables[index];
		FB_ASSERT(trackable);

		trackable->ptr = ptr;
		++trackable->triggers;

		trigger.trackables.push_back(trackable);
	}

	void erase(int id)
	{
		FB_ASSERT(id >= 0 && id < int(triggers.size()) && triggers[id]);
		Trigger &trigger = *triggers[id];
		FB_ASSERT(trigger.entity);

		TrackableList::iterator it = trigger.trackables.begin();
		for(; it != trigger.trackables.end(); ++it)
		{
			FB_ASSERT(*it);
			--it->get()->triggers;
		}

		tree->erase(trigger.entity);
		trigger.entity = 0;
		trigger.listener = 0;
	}

	void update(int ms)
	{
		time += ms;
		if(time <= UPDATE_INTERVAL)
			return;
		time -= UPDATE_INTERVAL;

		vector<Trigger *> activated;

		// Find triggers under trackables
		{
			TrackableList::iterator it = trackables.begin();
			for(; it != trackables.end(); ++it)
			{
				Trackable *t = it->get();
				VC3 position = t->ptr->getTrackablePosition();
				position.y = HEIGHT;

				vector<Trigger *> foundTriggers;
				tree->collectSphere(foundTriggers, position, t->ptr->getTrackableRadius2d());

				// Test if those really track given entity
				for(unsigned int i = 0; i < foundTriggers.size(); ++i)
				{
					Trigger *trigger = foundTriggers[i];
					TrackableList::iterator it = find_if(trigger->trackables.begin(), trigger->trackables.end(), TrackableComparator(t));
					if(it != trigger->trackables.end())
					{
						// Insert if and only if not on activated list already
						bool found = false;
						for(unsigned int j = 0; j < activated.size(); ++j)
						{
							if(activated[j] == trigger)
							{
								found = true;
								break;
							}
						}

						if(!found)
							activated.push_back(trigger);
					}
				}
			}
		}

		vector<Trigger *>::iterator it = activated.begin();
		for(; it != activated.end(); ++it)
			(*it)->listener->activate((*it)->id, (*it)->triggerData);
	}
};

CircleAreaTracker::CircleAreaTracker(const VC2 &size)
{
	scoped_ptr<Data> tempData(new Data());
	tempData->init(size);

	data.swap(tempData);
}

CircleAreaTracker::~CircleAreaTracker()
{
}

int CircleAreaTracker::addCircleTrigger(const ClippedCircle &circle, ITriggerListener *listener, void *triggerData)
{
	FB_ASSERT(listener);
	return data->insert(circle, listener, triggerData);
}

void CircleAreaTracker::addTrackable(int circleId, ITrackable *ptr)
{
	if(!ptr)
	{
		FB_ASSERT(ptr);
		return;
	}

	data->addTrackable(circleId, ptr);
}

void CircleAreaTracker::removeCircleTrigger(int circleId)
{
	data->erase(circleId);
}

void CircleAreaTracker::update(int ms)
{
	data->update(ms);
}

} // util
