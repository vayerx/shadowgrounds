
#ifndef IGAMECAMERABOUNDARY_H
#define IGAMECAMERABOUNDARY_H

#include <DatatypeDef.h>

namespace ui
{
	class IGameCameraBoundary
	{
		public:
			virtual ~IGameCameraBoundary() { }

			virtual bool isPositionInsideBoundaries(const VC3 &position) = 0;

			virtual VC3 getVectorToInsideBoundaries(const VC3 &position) = 0;

	};
}

#endif


