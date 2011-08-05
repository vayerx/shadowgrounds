
#ifndef ANYBURNABLETRACKABLEOBJECTFACTORY_H
#define ANYBURNABLETRACKABLEOBJECTFACTORY_H

#include "ITrackableObjectFactory.h"
#include "ITrackableUnifiedHandleObjectImplementationManager.h"

namespace game
{
namespace tracking
{
	class AnyBurnableTrackableObjectFactoryImpl;

	class AnyBurnableTrackableObjectFactory : public ITrackableObjectFactory
	{
	public:
		AnyBurnableTrackableObjectFactory(std::vector<ITrackableUnifiedHandleObjectImplementationManager *> implementations);
		~AnyBurnableTrackableObjectFactory();

		void addImplementation(ITrackableUnifiedHandleObjectImplementationManager *implementation);
		void removeImplementation(ITrackableUnifiedHandleObjectImplementationManager *implementation);

		virtual TRACKABLE_TYPEID_DATATYPE getTrackableObjectType();

		virtual ITrackableObjectIterator *getTrackablesFromArea(const VC3 &globalPosition, float radius);

	private:
		AnyBurnableTrackableObjectFactoryImpl *impl;
	};

}
}

#endif

