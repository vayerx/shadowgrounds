
#ifndef COVERFINDER_H
#define COVERFINDER_H

namespace game
{
	class CoverMap;

	class CoverFinder
	{
		public:
			static bool findCover(CoverMap *covermap, int x, int y,
				int *destx, int *desty, int fromx, int fromy);

			static bool isCoveredFrom(CoverMap *covermap, int x, int y,
				int fromx, int fromy);
	};
}

#endif



