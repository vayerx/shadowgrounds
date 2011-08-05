
#ifndef BLOCKEDHEIGHTAREAFILLMAPPER_H
#define BLOCKEDHEIGHTAREAFILLMAPPER_H

#include "../util/Floodfill.h"

namespace game
{
	class GameMap;

	// to assist in "BlockedHeightArea" floodfilling, implements the 
	// required mapper interface.
	class BlockedHeightAreaFillMapper : public util::IFloodfillByteMapper
	{
	private:
		GameMap *data;
		int height;

	public:
		// in this case, 2 means "has blocked this height area"
		// 1 means "height are to be blocked"
		// 0 means "some other height (do nothing here)"
		BlockedHeightAreaFillMapper(GameMap *data, int height);

		virtual unsigned char getByte(int x, int y);

		virtual void setByte(int x, int y, unsigned char value);
	};


}

#endif
