
#ifndef CHAINAREA_H
#define CHAINAREA_H

#include "QuadArea.h"

namespace game
{

	/**
	 * ChainArea is intended to work as an extra area that can be set to "chain" it's own triggering
	 * to trigger another area.
	 * This is done by providing a position for the chain area, than the ChainArea will "forward" the
	 * current position to in order to trigger the actual area.
	 *
	 * In other words, ChainArea can be used to create compound areas.
	 *
	 * Chain area should not have it's own type? rather be under the trigger area type, etc. it wants to
	 * chain to.
	 */
	class ChainArea : public QuadArea
	{
	public:


	};

}

#endif
