
#ifndef NOCAMERABOUNDARY_H
#define NOCAMERABOUNDARY_H

#include "IGameCameraBoundary.h"

namespace ui
{
	/**
	 * A dummy boundary for camera (treats all positions to be inside
	 * boundaries). Probably want to use this boundary for 
	 * cinematic cameras that should not be limited by player's
	 * units' positions.
	 *
   * @version 1.0, 28.3.2003
   * @author Jukka Kokkonen <jukka@frozenbyte.com>
   * @see PlayerUnitCameraBoundary
   * @see IGameCameraBoundary
	 */
	class NoCameraBoundary : public IGameCameraBoundary
	{
		public:
			NoCameraBoundary();

			~NoCameraBoundary();

			virtual bool isPositionInsideBoundaries(const VC3 &position);

			virtual VC3 getVectorToInsideBoundaries(const VC3 &position);
	};
}

#endif

