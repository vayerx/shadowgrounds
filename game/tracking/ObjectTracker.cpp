
#include "precompiled.h"

#include "ObjectTracker.h"

#include "ITrackerObjectType.h"
#include "ITrackableObjectFactory.h"
#include "ITrackableObject.h"
#include "ITrackerObject.h"
#include "tracker_signals.h"
#include "../system/SystemRandom.h"
#include "../unified_handle.h"

// NOTE: bad dependecy...
#include "TrackableUnifiedHandleObject.h"

#define OBJECTTRACKER_COLLECT_PERF_STATS 1

// tracker, etc. max amounts
// -------------------------

#define OBJECTTRACKER_MAX_TRACKERS 256
#define OBJECTTRACKER_MAX_TRACKERTYPES 32

// Tracker tick load balancing...
// ------------------------------

// the "minimum" msec delay at which balancing causes to trackers when too many simultaneous trackers
// (in this case, 30 msec = 2 ticks, which should cause better balancing than a single tick delay)
// (having one low-duty tick between 2 high-duty ticks should be something that gets nicely smoothed by frame buffering)
#define OBJECTTRACKER_BALANCING_INITIAL_WAIT_TIME 30

// max 10% variation in tick interval per tracker because of balancing...
// (even a small variation percentage should allow perfect balancing in long time span, as the balance slowly creeps in)
// (however, smaller values mean that it may take longer to achieve the balance)
#define OBJECTTRACKER_BALANCING_MAX_TICK_INTERVAL_VARIATION_PERCENTAGE 10

// how many trackers per run call before balancing starts to kick in
// (2 is probably ok for most cases?, if not a whole lot of trackers)
#define OBJECTTRACKER_BALANCING_PREFERRED_SIMULTANEOUS_TRACKERS 3

// how many trackers allowed per call at maximum.
// (1 would cause very heavy balancing and will not be very successful unless amount of trackers is low enough)
// (even this limit is not absolute if there are too many trackers with high tick rate to successfully balance..)
//#define OBJECTTRACKER_BALANCING_MAX_ALLOWED_SIMULTANEOUS_TRACKERS 2

// -------------------------------------

namespace game
{
namespace tracking
{
	class InvalidTrackerType : public ITrackerObjectType
	{
	public:
		virtual std::string getTrackerTypeName() const { return std::string("_invalid_tracker_type"); }
		virtual void *getTypeId() const { return NULL; }
		virtual int getTickInterval() const { return 0; }
		virtual bool doesAllowTickBalancing() const { return true; }
		virtual bool doesGiveOwnershipToObjectTracker() const { return true; }
		virtual void trackerDeleted() const { }
		virtual float getAreaOfInterestRadius() const { return 0.0f; }
		virtual TRACKABLE_TYPEID_DATATYPE getTrackablesTypeOfInterest() const { return 0; }
		virtual ITrackerObject *createNewObjectInstance() { return NULL; }
	};
	InvalidTrackerType invalidTrackerType_instance;


	class ObjectTrackerImpl
	{
	private:
		ObjectTrackerImpl()
		{
			for (int i = 0; i < TRACKABLE_TYPEID_MAX_TYPES; i++)
			{
				factories[i] = NULL;
			}
			for (int i = 0; i < OBJECTTRACKER_MAX_TRACKERS; i++)
			{
				trackers[i] = NULL;
				trackerLastCallTimes[i] = 0;
				attachedTrackables[i] = NULL;
				typeNumbersForTrackers[i] = -1;
			}
			for (int i = 0; i < OBJECTTRACKER_MAX_TRACKERTYPES; i++)
			{
				trackerTypes[i] = NULL;
			}
			lastTracker = -1;
			currentTime = 0;

			nonTrackerRunsSinceTime = 0;
			trackerTicksAtLastRun = 0;
			balancingTickDelay = 0;

			collectBalancingStats = false;
			peakTrackerTicksPerRun = 0;
			trackerTicksSinceLastDump = 0;
			runsSinceLastDump = 0;

#ifdef OBJECTTRACKER_COLLECT_PERF_STATS 
			runTimePeak = 0;
			runTimeAvg = 0;
#endif

			// reserving 0 tracker type number...
			addTrackerType(&invalidTrackerType_instance);
		}

		ITrackableObjectFactory *factories[TRACKABLE_TYPEID_MAX_TYPES];
		ITrackerObject *trackers[OBJECTTRACKER_MAX_TRACKERS];
		ITrackableObject *attachedTrackables[OBJECTTRACKER_MAX_TRACKERS];
		int trackerLastCallTimes[OBJECTTRACKER_MAX_TRACKERS];
		TrackerTypeNumber typeNumbersForTrackers[OBJECTTRACKER_MAX_TRACKERS];
		ITrackerObjectType *trackerTypes[OBJECTTRACKER_MAX_TRACKERTYPES];

		int lastTracker;
		int currentTime;

		int nonTrackerRunsSinceTime;
		int trackerTicksAtLastRun;
		int balancingTickDelay;

		bool collectBalancingStats;
		int peakTrackerTicksPerRun;
		int trackerTicksSinceLastDump;
		int runsSinceLastDump;

#ifdef OBJECTTRACKER_COLLECT_PERF_STATS 
		int runTimePeak;
		int runTimeAvg;
#endif

		friend class ObjectTracker;


		TrackerTypeNumber addTrackerType(ITrackerObjectType *trackerType)
		{
			for (int i = 0; i < OBJECTTRACKER_MAX_TRACKERTYPES; i++)
			{
				if (trackerTypes[i] == NULL)
				{
					trackerTypes[i] = trackerType;
					return i;
				}
			}

			LOG_ERROR("ObjectTracker::addTrackerType - Max tracker type amount exceeded.");
			return -1;
		}


		TrackerTypeNumber getTrackerTypeNumberByName(const std::string &typeName) const
		{
			for (int i = 0; i < OBJECTTRACKER_MAX_TRACKERTYPES; i++)
			{
				if (trackerTypes[i] != NULL)
				{
					if (trackerTypes[i]->getTrackerTypeName() == typeName)
						return i;
				}
			}
			return -1;
		}


		void removeTrackerType(ITrackerObjectType *trackerType)
		{
			deleteAllTrackersOfType(trackerType);

			for (int i = 0; i < OBJECTTRACKER_MAX_TRACKERTYPES; i++)
			{
				if (trackerTypes[i] == trackerType)
				{
					trackerTypes[i] = NULL;
					return;
				}
			}

			LOG_WARNING("ObjectTracker::removeTrackerType - Given tracker type not found in list.");
		}


		void removeTrackerType(TrackerTypeNumber trackerTypeNumber)
		{
			if (trackerTypeNumber < 0 || trackerTypeNumber >= OBJECTTRACKER_MAX_TRACKERTYPES)
			{
				LOG_ERROR("ObjectTracker::removeTrackerType - Parameter out of range.");
				assert(!"ObjectTracker::removeTrackerType - Parameter out of range.");
				return;
			}

			if (trackerTypes[trackerTypeNumber] != NULL)
			{
				deleteAllTrackersOfType(trackerTypes[trackerTypeNumber]);
				trackerTypes[trackerTypeNumber] = NULL;
			} else {
				LOG_WARNING("ObjectTracker::removeTrackerType - Given tracker type by number is null.");
			}
		}


		void removeAllTrackerTypes()
		{
			deleteAllTrackers();

			for (int i = 0; i < OBJECTTRACKER_MAX_TRACKERTYPES; i++)
			{
				trackerTypes[i] = NULL;
			}
		}


		void addTrackableObjectFactory(ITrackableObjectFactory *trackableObjectFactory)
		{
			assert(trackableObjectFactory != NULL);
			int mask = trackableObjectFactory->getTrackableObjectType();
			for (int i = 0; i < TRACKABLE_TYPEID_MAX_TYPES; i++)
			{
				if (mask == (1<<i))
				{
					assert(factories[i] == NULL);
					factories[i] = trackableObjectFactory;
					return;
				}
			}

			LOG_ERROR("ObjectTracker::addTrackerType - Factory had invalid trackable object type value (expected single bit value).");
			assert(!"ObjectTracker::addTrackerType - Factory had invalid trackable object type value (expected single bit value).");

			/*
			for (int i = 0; i < TRACKABLE_TYPEID_MAX_TYPES; i++)
			{
				if (factories[i] == NULL)
				{
					factories[i] = trackableObjectFactory;
					return;
				}
			}

			LOG_ERROR("ObjectTracker::addTrackerType - Max trackable object factory amount exceeded.");
			*/
		}


		UnifiedHandle createTracker(TrackerTypeNumber trackerTypeNumber)
		{ 
			if (trackerTypeNumber < 0 || trackerTypeNumber >= OBJECTTRACKER_MAX_TRACKERTYPES)
			{
				LOG_ERROR("ObjectTracker::createTracker - Parameter out of range.");
				assert(!"ObjectTracker::createTracker - Parameter out of range.");
				return UNIFIED_HANDLE_NONE;
			}
			
			if (trackerTypes[trackerTypeNumber] != NULL)
			{
				for (int i = 0; i < OBJECTTRACKER_MAX_TRACKERS; i++)
				{
					if (trackers[i] == NULL)
					{
						if (lastTracker < i)
							lastTracker = i;

						ITrackerObject *t = trackerTypes[trackerTypeNumber]->createNewObjectInstance();
						if (t == NULL)
						{
							LOG_WARNING("ObjectTracker::addTracker - Factory failed to create tracker.");
							return UNIFIED_HANDLE_NONE;
						}
						trackers[i] = t;
						typeNumbersForTrackers[i] = trackerTypeNumber;

						// do first tick immediately...
						//trackerLastCallTimes[i] = currentTime;
						trackerLastCallTimes[i] = currentTime - trackerTypes[trackerTypeNumber]->getTickInterval();

						assert(attachedTrackables[i] == NULL);

						UnifiedHandle uh = UNIFIED_HANDLE_FIRST_TRACKER + i;
						return uh;
					}
				}
				LOG_WARNING("ObjectTracker::addTracker - Too many trackers.");

			} else {
				LOG_WARNING("ObjectTracker::addTracker - Given tracker type by number is null.");
			}

			// heck.. this deals with trackables, not trackers...
			/*
			// TODO: optimize, some quicker way to solve type bit mask value to index.
			// (a single cache variable would be simple and sufficient?)
			for (int i = 0; i < TRACKABLE_TYPEID_MAX_TYPES; i++)
			{
				if (type == (1<<i))
				{
					if (factories[i] != NULL)
					{
						ITrackableObject factories[i]->
						return ret;
					} else {
						return 0;
					}
				}
			}
			Logger::getInstance()->error("ObjectTracker::createTracker - given type id is a value with multiple bits set (expected single bit type id value).");
			*/

			return UNIFIED_HANDLE_NONE;
		}


		void deleteAllTrackers()
		{
			for (int i = 0; i < lastTracker + 1; i++)
			{
				if (trackers[i] != NULL)
				{
					if (trackers[i]->getType()->doesGiveOwnershipToObjectTracker())
						delete trackers[i];
					trackers[i] = NULL;
					if (attachedTrackables[i] != NULL)
					{
						if (attachedTrackables[i]->getTypeId() == TrackableUnifiedHandleObject::typeId)
						{
							// WARNING: unsafe cast! (based on check above)
							((TrackableUnifiedHandleObject *)attachedTrackables[i])->removeTrackedBy(UNIFIED_HANDLE_FIRST_TRACKER + i);
						}
						attachedTrackables[i]->release();
						attachedTrackables[i] = NULL;
					}
				}
			}
			lastTracker = -1;
		}


		void deleteAllTrackersOfType(ITrackerObjectType *trackerType)
		{
			for (int i = 0; i < lastTracker + 1; i++)
			{
				if (trackers[i] != NULL
					&& trackers[i]->getType() == trackerType)
				{
					if (trackers[i]->getType()->doesGiveOwnershipToObjectTracker())
						delete trackers[i];
					trackers[i] = NULL;
					if (attachedTrackables[i] != NULL)
					{
						if (attachedTrackables[i]->getTypeId() == TrackableUnifiedHandleObject::typeId)
						{
							// WARNING: unsafe cast! (based on check above)
							((TrackableUnifiedHandleObject *)attachedTrackables[i])->removeTrackedBy(UNIFIED_HANDLE_FIRST_TRACKER + i);
						}
						attachedTrackables[i]->release();
						attachedTrackables[i] = NULL;
					}
				}
			}
			optimizeLastTrackerValue();
		}


		void optimizeLastTrackerValue()
		{
			while (lastTracker >= 0
				&& trackers[lastTracker] == NULL)
			{
				lastTracker--;
			}
		}


		void deleteTracker(ITrackerObject *tracker)
		{
			assert(tracker != NULL);

			if (!tracker->getType()->doesGiveOwnershipToObjectTracker())
			{
				tracker->trackerDeleted();
				// the deleted tracker should now have called the releaseTracker to actually get it removed from ObjectTracker...
				// nope, at least projectiles just set their lifetime to 1 and therefore do the release in next tick
				/*
				for (int i = 0; i < lastTracker + 1; i++)
				{
					if (trackers[i] == tracker)
					{
						assert(!"ObjectTracker::deleteTracker - the tracker is still in the list, releaseTracker was not properly called by the tracker when deleted?");
					}
				}
				*/
				return;
			}

			bool found = false;
			for (int i = 0; i < lastTracker + 1; i++)
			{
				if (trackers[i] == tracker)
				{
					if (trackers[i]->getType()->doesGiveOwnershipToObjectTracker())
						delete tracker;
					trackers[i] = NULL;
					typeNumbersForTrackers[i] = -1;
					if (attachedTrackables[i] != NULL)
					{
						if (attachedTrackables[i]->getTypeId() == TrackableUnifiedHandleObject::typeId)
						{
							// WARNING: unsafe cast! (based on check above)
							((TrackableUnifiedHandleObject *)attachedTrackables[i])->removeTrackedBy(UNIFIED_HANDLE_FIRST_TRACKER + i);
						}
						attachedTrackables[i]->release();
						attachedTrackables[i] = NULL;
					}
					found = true;
					break;
				}
			}

			if (!found)
			{
				LOG_WARNING("ObjectTracker::deleteTracker - Given tracker not found in list (already deleted?).");
			}

			optimizeLastTrackerValue();
		}


		void releaseTracker(ITrackerObject *tracker)
		{
			assert(tracker != NULL);
			assert(!tracker->getType()->doesGiveOwnershipToObjectTracker());

			bool found = false;
			for (int i = 0; i < lastTracker + 1; i++)
			{
				if (trackers[i] == tracker)
				{
					if (trackers[i]->getType()->doesGiveOwnershipToObjectTracker())
						delete tracker;
					trackers[i] = NULL;
					typeNumbersForTrackers[i] = -1;
					if (attachedTrackables[i] != NULL)
					{
						if (attachedTrackables[i]->getTypeId() == TrackableUnifiedHandleObject::typeId)
						{
							// WARNING: unsafe cast! (based on check above)
							((TrackableUnifiedHandleObject *)attachedTrackables[i])->removeTrackedBy(UNIFIED_HANDLE_FIRST_TRACKER + i);
						}
						attachedTrackables[i]->release();
						attachedTrackables[i] = NULL;
					}
					found = true;
					break;
				}
			}

			if (!found)
			{
				LOG_WARNING("ObjectTracker::releaseTracker - Given tracker not found in list (already deleted?).");
			}

			optimizeLastTrackerValue();
		}



		void deleteTracker(UnifiedHandle trackerUnifiedHandle)
		{
			if (!IS_UNIFIED_HANDLE_TRACKER(trackerUnifiedHandle))
			{
				LOG_ERROR("ObjectTracker::deleteTracker - Given unified handle is not of tracker type.");
				assert(!"ObjectTracker::deleteTracker - Given unified handle is not of tracker type.");
				return;
			}

			int i = trackerUnifiedHandle - UNIFIED_HANDLE_FIRST_TRACKER;
			assert(i >= 0);
			if (i < OBJECTTRACKER_MAX_TRACKERS)
			{
				if (trackers[i] != NULL)
				{
					if (trackers[i]->getType()->doesGiveOwnershipToObjectTracker())
						delete trackers[i];
					trackers[i] = NULL;
					typeNumbersForTrackers[i] = -1;
					if (attachedTrackables[i] != NULL)
					{
						attachedTrackables[i]->release();
						attachedTrackables[i] = NULL;
					}
					optimizeLastTrackerValue();
				} else {
					LOG_WARNING("ObjectTracker::deleteTracker - Tracker with given handle not found in list (already deleted?).");
				}
			} else {
				LOG_ERROR("ObjectTracker::deleteTracker - Given handle out of maximum tracker range.");
				assert(!"ObjectTracker::deleteTracker - Given handle out of maximum tracker range.");
			}
		}



		void iterateTrackablesForTracker(ITrackerObject *tracker)
		{
			assert(tracker != NULL);

			// TODO: optimize, some quicker way to solve type bit mask value to index.
			// (a single cache variable would be simple and sufficient?)
			bool foundFactories = false;
			int typeMask = tracker->getType()->getTrackablesTypeOfInterest();

			if (typeMask == 0)
			{
				// silly tracker, not interested in any trackables?
				Logger::getInstance()->warning("ObjectTracker::getTrackablesForTracker - Tracker is not interested in any trackable object types but iterateTrackablesForTracker was called.");
				return;
			}

			for (int i = 0; i < TRACKABLE_TYPEID_MAX_TYPES; i++)
			{
				if ((typeMask & (1<<i)) != 0)
				{
					if (factories[i] != NULL)
					{
						foundFactories = true;

						VC3 pos = tracker->getTrackerPosition();
						float radius = tracker->getType()->getAreaOfInterestRadius();
						ITrackableObjectIterator *iter = factories[i]->getTrackablesFromArea(pos, radius);

						tracker->iterateTrackables(iter);

						// TODO: if iterator has any objects left, release()?
						// or should the iterator destructor always release every one of them...
						// (and add another addReference() method if someone else wants ownership)

						delete iter;
					}
				}
			}
			if (!foundFactories)
			{
				Logger::getInstance()->warning("ObjectTracker::getTrackablesForTracker - No trackable object factory for any of the tracker's requested trackable types.");
				return;
			}

		}

		void signalToTrackerFromTrackable(UnifiedHandle trackable, int trackerSignalNumber)
		{
			ITrackableObject *trackableObject = TrackableUnifiedHandleObject::getInstanceFromPool(trackable);
			if (trackableObject != NULL)
			{
				if (trackableObject->getTypeId() == TrackableUnifiedHandleObject::typeId)
				{
					// WARNING: unsafe cast! (based on check above)
					int index = 0;
					while (true)
					{
						UnifiedHandle trackerUH = ((TrackableUnifiedHandleObject *)trackableObject)->getTrackedByWithIndex(index);
						if (trackerUH == UNIFIED_HANDLE_NONE)
							break;

						signalToTrackerDirectly(trackerUH, trackerSignalNumber);
						index++;
					}
				} else {
					Logger::getInstance()->error("ObjectTracker::signalToTrackerFromTrackable - Only TrackableUnifiedHandleObject type trackable supported.");
					assert(!"ObjectTracker::signalToTrackerFromTrackable - Only TrackableUnifiedHandleObject type trackable supported.");
				}
			} else {
				Logger::getInstance()->error("ObjectTracker::signalToTrackerFromTrackable - Failed to get TrackableUnifiedHandleObject from pool with given unified handle.");
				assert(!"ObjectTracker::signalToTrackerFromTrackable - Failed to get TrackableUnifiedHandleObject from pool with given unified handle.");
			}
		}

		void signalToTrackerDirectly(UnifiedHandle trackerUnifiedHandle, int trackerSignalNumber)
		{
			ITrackerObject *tracker = this->getTrackerByUnifiedHandle(trackerUnifiedHandle);
			assert(tracker != NULL);
			tracker->trackerSignal(trackerSignalNumber);
		}



		ITrackerObject *getTrackerByUnifiedHandle(UnifiedHandle handle)
		{
			assert(IS_UNIFIED_HANDLE_TRACKER(handle));

			int i = handle - UNIFIED_HANDLE_FIRST_TRACKER;

			if (i >= 0 && i < OBJECTTRACKER_MAX_TRACKERS)
			{
				return trackers[i];
			} else {
				Logger::getInstance()->error("ObjectTracker::getTrackerByUnifiedHandle - Given unified handle out of range.");
				assert(!"ObjecTracker::getTrackerByUnifiedHandle - Given unified handle out of range.");
				return NULL;
			}
		}


		TrackerTypeNumber getTrackerTypeNumberForTracker(UnifiedHandle handle)
		{
			assert(IS_UNIFIED_HANDLE_TRACKER(handle));

			int i = handle - UNIFIED_HANDLE_FIRST_TRACKER;

			if (i >= 0 && i < OBJECTTRACKER_MAX_TRACKERS)
			{
				return typeNumbersForTrackers[i];
			} else {
				Logger::getInstance()->error("ObjectTracker::getTrackerTypeNumberByUnifiedHandle - Given unified handle out of range.");
				assert(!"ObjecTracker::getTrackerTypeNumberByUnifiedHandle - Given unified handle out of range.");
				return -1;
			}
		}


		void run(int msec)
		{

#ifdef OBJECTTRACKER_COLLECT_PERF_STATS 
			Timer::update();
			int startTime = Timer::getTime();
#endif

			currentTime += msec;

			int runTrackerTicks = 0;

			for (int i = 0; i < lastTracker + 1; i++)
			{
				if (trackers[i] != NULL)
				{
					assert(trackers[i]->getType() != NULL);
					if (currentTime >= trackerLastCallTimes[i] + trackers[i]->getType()->getTickInterval())
					{
						bool runNow = true;
						if (trackers[i]->getType()->doesAllowTickBalancing())
						{
							if (runTrackerTicks >= OBJECTTRACKER_BALANCING_PREFERRED_SIMULTANEOUS_TRACKERS)
							{
								// delay the running...?
								if (balancingTickDelay > 0)
								{
									runNow = false;
								}
								if (currentTime < trackerLastCallTimes[i] + (trackers[i]->getType()->getTickInterval() * (100 + OBJECTTRACKER_BALANCING_MAX_TICK_INTERVAL_VARIATION_PERCENTAGE)) / 100)
								{
									runNow = false;
								}
							}
						}

						if (runNow)
						{
							trackers[i]->tick();
							// must check that the tracker has not self-deleted...
							if (trackers[i] != NULL)
							{
								if (trackers[i]->getType()->doesAllowTickBalancing())
								{
									// HACK: a bit random distribution...
									// FIXME: this should be GameRandom, not SystemRandom
									if (runTrackerTicks >= OBJECTTRACKER_BALANCING_PREFERRED_SIMULTANEOUS_TRACKERS)
									{
										// TODO: proper real balancing, not just reliance on this kinda random...
										trackerLastCallTimes[i] = currentTime + (SystemRandom::getInstance()->nextInt() & 1) * msec;
									} else {
										trackerLastCallTimes[i] = currentTime;
									}
								} else {
									trackerLastCallTimes[i] += trackers[i]->getType()->getTickInterval();
								}
							}
							runTrackerTicks++;
						}
					}

					if (attachedTrackables[i] != NULL)
					{
						if (!attachedTrackables[i]->doesExist())
						{
							// first call lostTracked...
							trackers[i]->lostTracked();

							// then, if the lostTracked has not changed the trackable to existing one, lose the pointers...
							if (attachedTrackables[i] != NULL
								&& !attachedTrackables[i]->doesExist())
							{
								if (attachedTrackables[i]->getTypeId() == TrackableUnifiedHandleObject::typeId)
								{
									// WARNING: unsafe cast! (based on check above)
									((TrackableUnifiedHandleObject *)attachedTrackables[i])->removeTrackedBy(UNIFIED_HANDLE_FIRST_TRACKER + i);
								}
								attachedTrackables[i]->release();
								attachedTrackables[i] = NULL;
							}
						} else {
							VC3 pos = attachedTrackables[i]->getTrackableObjectPosition();
							trackers[i]->setTrackablePosition(pos);
							QUAT rot = attachedTrackables[i]->getTrackableObjectRotation();
							trackers[i]->setTrackableRotation(rot);
						}
					}

				}
			}

			if (runTrackerTicks == 0)
			{
				nonTrackerRunsSinceTime += msec;
			} else {
				if (runTrackerTicks > OBJECTTRACKER_BALANCING_PREFERRED_SIMULTANEOUS_TRACKERS)
				{
					this->balancingTickDelay = OBJECTTRACKER_BALANCING_INITIAL_WAIT_TIME;
				}
				nonTrackerRunsSinceTime = 0;
			}
			if (this->balancingTickDelay > 0)
			{
				this->balancingTickDelay -= msec;
			}
			trackerTicksAtLastRun = runTrackerTicks;

			if (collectBalancingStats)
			{
				if (runTrackerTicks > peakTrackerTicksPerRun)
				{
					peakTrackerTicksPerRun = runTrackerTicks;
				}
				trackerTicksSinceLastDump += runTrackerTicks;
				runsSinceLastDump++;
			}

#ifdef OBJECTTRACKER_COLLECT_PERF_STATS 
			Timer::update();
			int endTime = Timer::getTime();

			int timeDiff = endTime - startTime;

			if (timeDiff > runTimePeak)
				runTimePeak = timeDiff;

			static int timeAccum = 0;
			static int msecCounter = 0;
			timeAccum += timeDiff;
			msecCounter += msec;
			if (msecCounter >= 5000)  // sample every 5 secs
			{
				runTimeAvg = (int)((float)timeAccum * (1000.0f / msecCounter));
				timeAccum = 0;
				msecCounter = 0;
			}
#endif

		}

	};


	ObjectTracker::ObjectTracker()
	{
		this->impl = new ObjectTrackerImpl();
	}

	ObjectTracker::~ObjectTracker()
	{
		delete impl;
	}

	TrackerTypeNumber ObjectTracker::addTrackerType(ITrackerObjectType *trackerType)
	{
		return impl->addTrackerType(trackerType);
	}


	void ObjectTracker::removeTrackerType(ITrackerObjectType *trackerType)
	{
		impl->removeTrackerType(trackerType);
	}


	void ObjectTracker::removeTrackerType(TrackerTypeNumber trackerTypeNumber)
	{
		impl->removeTrackerType(trackerTypeNumber);
	}


	void ObjectTracker::removeAllTrackerTypes()
	{
		impl->removeAllTrackerTypes();
	}


	TrackerTypeNumber ObjectTracker::getTrackerTypeNumberByName(const std::string &typeName) const
	{
		return impl->getTrackerTypeNumberByName(typeName);
	}


	void ObjectTracker::addTrackableObjectFactory(ITrackableObjectFactory *trackableObjectFactory)
	{
		impl->addTrackableObjectFactory(trackableObjectFactory);
	}


	UnifiedHandle ObjectTracker::createTracker(TrackerTypeNumber trackerTypeNumber)
	{
		return impl->createTracker(trackerTypeNumber);
	}


	void ObjectTracker::deleteAllTrackers()
	{
		impl->deleteAllTrackers();
	}


	void ObjectTracker::deleteAllTrackersOfType(ITrackerObjectType *trackerType)
	{
		impl->deleteAllTrackersOfType(trackerType);
	}


	void ObjectTracker::deleteTracker(ITrackerObject *tracker)
	{
		impl->deleteTracker(tracker);
	}


	void ObjectTracker::deleteTracker(UnifiedHandle trackerUnifiedHandle)
	{
		ITrackerObject *tracker = impl->getTrackerByUnifiedHandle(trackerUnifiedHandle);
		impl->deleteTracker(tracker);
	}


	void ObjectTracker::releaseTracker(ITrackerObject *tracker)
	{
		impl->releaseTracker(tracker);
	}


	void ObjectTracker::iterateTrackablesForTracker(ITrackerObject *tracker)
	{
		impl->iterateTrackablesForTracker(tracker);
	}


	void ObjectTracker::iterateTrackablesForTracker(UnifiedHandle trackerUnifiedHandle)
	{
		ITrackerObject *tracker = impl->getTrackerByUnifiedHandle(trackerUnifiedHandle);
		impl->iterateTrackablesForTracker(tracker);
	}

	void ObjectTracker::signalToTrackerFromTrackable(UnifiedHandle trackable, int trackerSignalNumber)
	{
		impl->signalToTrackerFromTrackable(trackable, trackerSignalNumber);
	}

	void ObjectTracker::signalToTrackerDirectly(UnifiedHandle trackerUnifiedHandle, int trackerSignalNumber)
	{
		impl->signalToTrackerDirectly(trackerUnifiedHandle, trackerSignalNumber);
	}

	ITrackerObject *ObjectTracker::getTrackerByUnifiedHandle(UnifiedHandle handle)
	{
		return impl->getTrackerByUnifiedHandle(handle);
	}

	TrackerTypeNumber ObjectTracker::getTrackerTypeNumberForTracker(UnifiedHandle trackerHandle)
	{
		return impl->getTrackerTypeNumberForTracker(trackerHandle);
	}


	void ObjectTracker::run(int msec)
	{
		impl->run(msec);
	}

	std::vector<ITrackerObject *> ObjectTracker::getAllTrackers()
	{
		std::vector<ITrackerObject *> ret;

		for (int i = 0; i < impl->lastTracker + 1; i++)
		{
			if (impl->trackers[i] != NULL)
				ret.push_back(impl->trackers[i]);
		}

		return ret;
	}

	void ObjectTracker::attachTrackerToTrackable(UnifiedHandle tracker, ITrackableObject *trackableObject)
	{
		int i = tracker - UNIFIED_HANDLE_FIRST_TRACKER;

		assert(i >= 0 && i < OBJECTTRACKER_MAX_TRACKERS);

		ITrackableObject *previousTrackable = impl->attachedTrackables[i];

		assert(impl->trackers[i] != NULL);
		if (impl->trackers[i] != NULL)
		{
			if (impl->attachedTrackables[i] != NULL)
			{
				if (impl->attachedTrackables[i]->getTypeId() == TrackableUnifiedHandleObject::typeId)
				{
					// WARNING: unsafe cast! (based on check above)
					((TrackableUnifiedHandleObject *)impl->attachedTrackables[i])->removeTrackedBy(tracker);
				}
				impl->attachedTrackables[i]->release();
			}
			impl->attachedTrackables[i] = trackableObject;
			if (trackableObject->getTypeId() == TrackableUnifiedHandleObject::typeId)
			{
				// WARNING: unsafe cast! (based on check above)
				((TrackableUnifiedHandleObject *)trackableObject)->addTrackedBy(tracker);
			}

			if (previousTrackable != trackableObject)
			{
				impl->trackers[i]->attachedToTrackable(trackableObject);
			}
		}
	}

	ITrackableObject *ObjectTracker::getTrackerAttachedTrackable(UnifiedHandle tracker)
	{
		int i = tracker - UNIFIED_HANDLE_FIRST_TRACKER;

		assert(i >= 0 && i < OBJECTTRACKER_MAX_TRACKERS);

		assert(impl->trackers[i] != NULL);

		return impl->attachedTrackables[i];
	}

	std::string ObjectTracker::getStatusInfo()
	{
		std::string ret = "ObjectTracker:\r\n";
		ret += std::string("Tracker total amount: ") + int2str(getAllTrackers().size());
		ret += std::string("\r\n");

		int eff = 100;
		if (impl->lastTracker >= 0) eff = 100 * getAllTrackers().size() / (impl->lastTracker+1);
		ret += std::string("Tracker list fill effectiveness: ") + int2str(eff);
		ret += std::string("%\r\n");

		int typeAmount = 0;
		for (int i = 0; i < OBJECTTRACKER_MAX_TRACKERTYPES; i++)
		{
			if (impl->trackerTypes[i] != NULL) typeAmount++;
		}
		ret += std::string("Tracker type amount: ") + int2str(typeAmount);
		ret += std::string("\r\n");

		if (impl->collectBalancingStats
			&& impl->runsSinceLastDump > 0)
		{
			char floatbuf[20];
			sprintf(floatbuf, "%f", (float)impl->trackerTicksSinceLastDump / (float)impl->runsSinceLastDump);
			ret += std::string("Tracker balancing, avg tracker ticks per run: ") + floatbuf;
			ret += std::string("\r\n");
			ret += std::string("Tracker balancing, peak tracker ticks per run: ") + int2str(impl->peakTrackerTicksPerRun);
			ret += std::string("\r\n");
			impl->peakTrackerTicksPerRun = 0;
			impl->trackerTicksSinceLastDump = 0;
			impl->runsSinceLastDump = 0;
		} else {
			impl->collectBalancingStats = true;
			ret += std::string("Tracker balancing, avg tracker ticks per run: ") + "N/A (first tracker dump or no runs)";
			ret += std::string("\r\n");
			ret += std::string("Tracker balancing, peak tracker ticks per run: ") + "N/A (first tracker dump or no runs)";
			ret += std::string("\r\n");
		}
#ifdef OBJECTTRACKER_COLLECT_PERF_STATS 
		ret += std::string("Trackers run peak time usage (msec in one run): ") + int2str(impl->runTimePeak);
		ret += std::string("\r\n");
		ret += std::string("Trackers run avg time usage (msec in one second): ") + int2str(impl->runTimeAvg);
		ret += std::string("\r\n");
#endif

		ret += TrackableUnifiedHandleObject::getStatusInfo();

		return ret;
	}

}
}

