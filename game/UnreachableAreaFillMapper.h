
#ifndef UNREACHABLEAREAFILLMAPPER_H
#define UNREACHABLEAREAFILLMAPPER_H

#include "../util/Floodfill.h"


// in this case:
// 2 means "don't block this area as it was reachable" (result of floodfill)
// 1 means "this is an obstacle" (to be set before fill)
// 0 means "this area is not obstacle" (...but is unreachable if left in result of floodfill)
#define UNR_VALUE_REACHABLE 2
#define UNR_VALUE_OBSTACLE 1
#define UNR_VALUE_EMPTY 0


namespace game
{
	class GameMap;

	class UnreachableAreaFillMapper : public util::IFloodfillByteMapper
	{
	private:
		GameMap *data;
		unsigned char *fillmap;

	public:
		UnreachableAreaFillMapper(GameMap *data);
		~UnreachableAreaFillMapper();

		virtual unsigned char getByte(int x, int y);

		virtual void setByte(int x, int y, unsigned char value);

		void addReachablePoint(int x, int y);

		// (applies the result to gamemap.)
		void applyResult();

	private:
		void init();

	};


}

#endif
