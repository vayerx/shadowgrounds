
#ifndef COVERMAP_H
#define COVERMAP_H

#include <assert.h>

namespace game
{
	class CoverMap
	{
		public:
			static const int COVER_FROM_WEST_MASK;
			static const int COVER_FROM_EAST_MASK;
			static const int COVER_FROM_NORTH_MASK;
			static const int COVER_FROM_SOUTH_MASK;
			static const int COVER_FROM_ALL_MASK;

			enum COVER_DIRECTION
			{
				COVER_DIRECTION_N = 1,
				COVER_DIRECTION_NE = 2,
				COVER_DIRECTION_E = 3,
				COVER_DIRECTION_SE = 4,
				COVER_DIRECTION_S = 5,
				COVER_DIRECTION_SW = 6,
				COVER_DIRECTION_W = 7,
				COVER_DIRECTION_NW = 8
			};

			CoverMap(int sizeX, int sizeY);

			~CoverMap();

			bool load(char *filename);

			bool save(char *filename);

			void create(unsigned short *obstacleMap, unsigned short *heightMap);

			inline int getDistanceToNearestCover(int x, int y)
			{
				assert(x >= 0 && x < sizeX);
				assert(y >= 0 && y < sizeY);
				return (int)(covermap[x + y * sizeX] & 255);
			}

			COVER_DIRECTION getNearestCoverDirection(int x, int y);

			bool isCoveredFromAll(int fromMask, int x, int y);

			bool isCoveredFromAny(int fromMask, int x, int y);

			inline int getSizeX() { return sizeX; }

			inline int getSizeY() { return sizeY; }

			void removeCover(int x, int y);

		private:
			unsigned short *covermap;
			int sizeX;
			int sizeY;

	};
}

#endif

