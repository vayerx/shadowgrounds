
#ifndef ITRACKABLEOBJECTITERATOR_H
#define ITRACKABLEOBJECTITERATOR_H

namespace game
{
namespace tracking
{
	class ITrackableObject;

	class ITrackableObjectIterator
	{
	public:
		virtual ~ITrackableObjectIterator() { };

		virtual bool iterateAvailable() = 0;
		virtual ITrackableObject *iterateNext() = 0;
	};

}
}

#endif
