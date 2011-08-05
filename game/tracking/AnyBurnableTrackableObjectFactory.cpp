
#include "precompiled.h"

#include "AnyBurnableTrackableObjectFactory.h"
#include "TrackableUnifiedHandleObject.h"
#include "SimpleTrackableObjectIterator.h"
#include "trackable_types.h"
#include "../Game.h"

namespace game
{
namespace tracking
{
	class AnyBurnableTrackableObjectFactoryImpl
	{
	private:
		std::vector<ITrackableUnifiedHandleObjectImplementationManager *> implementations;

		friend class AnyBurnableTrackableObjectFactory;
	};

	AnyBurnableTrackableObjectFactory::AnyBurnableTrackableObjectFactory(std::vector<ITrackableUnifiedHandleObjectImplementationManager *> implementations)
	{
		impl = new AnyBurnableTrackableObjectFactoryImpl();
		impl->implementations = implementations;
	}

	void AnyBurnableTrackableObjectFactory::addImplementation(ITrackableUnifiedHandleObjectImplementationManager *implementation)
	{
		// TODO: assert that the implementation is not already in the list
		impl->implementations.push_back(implementation);
	}

	void AnyBurnableTrackableObjectFactory::removeImplementation(ITrackableUnifiedHandleObjectImplementationManager *implementation)
	{
		for (int i = 0; i < (int)impl->implementations.size(); i++)
		{
			if (impl->implementations[i] == implementation)
			{
				for (int j = i; j < (int)impl->implementations.size() - 1; j++)
				{
					impl->implementations[j] = impl->implementations[j + 1];
				}
				impl->implementations.pop_back();
				return;
			}
		}
		assert(!"AnyBurnableTrackableObjectFactory::removeImplementation - implementation was not found in the list.");
	}


	AnyBurnableTrackableObjectFactory::~AnyBurnableTrackableObjectFactory()
	{
		delete impl;
	}


	TRACKABLE_TYPEID_DATATYPE AnyBurnableTrackableObjectFactory::getTrackableObjectType()
	{
		return TRACKABLE_TYPE_BURNABLE;
	}


	ITrackableObjectIterator *AnyBurnableTrackableObjectFactory::getTrackablesFromArea(const VC3 &globalPosition, float radius)
	{
		std::vector<ITrackableObject *> ret;

		// TODO: optimize, create an iterator that can hold and iterate multiple ITrackableUnifiedHandleObjectIterators
		// and return that instead of SimpleTrackableObjectIterator... 

		for (int i = 0; i < (int)impl->implementations.size(); i++)
		{
			ITrackableUnifiedHandleObjectIterator *iter = impl->implementations[i]->getTrackableUnifiedHandleObjectsFromArea(globalPosition, radius, TRACKABLE_TYPE_BURNABLE);
			while (iter->iterateAvailable())
			{
				UnifiedHandle handle = iter->iterateNext();
				ret.push_back(TrackableUnifiedHandleObject::getInstanceFromPool(handle));
			}
			delete iter;
		}

		return new SimpleTrackableObjectIterator(ret);
	}

}
}

