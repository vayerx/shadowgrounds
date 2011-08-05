
#ifndef IUNITLISTITERATOR_H
#define IUNITLISTITERATOR_H

namespace game
{
	class Unit;

	class IUnitListIterator
	{
		public:
			virtual ~IUnitListIterator() { };

			virtual Unit *iterateNext() = 0;

			virtual bool iterateAvailable() = 0;
	};
}

#endif



