
#ifndef EJECTER_H
#define EJECTER_H

// HACK: crude copy from muzzleflasher

namespace ui
{
	class IPointableObject;
	class VisualObject;

	class Ejecter
	{
		public:

			// NOTICE: expects a _unit_, not just any IPointableObject
			// (will do a nasty cast to that)
			// should use some "ejectable" interface instead. :P

			static void createEject(IPointableObject *unit);

			static void deleteEject(IPointableObject *unit);

	};

}

#endif

