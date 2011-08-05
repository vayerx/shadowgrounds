
#ifndef LINEAREACHECKER_H
#define LINEAREACHECKER_H

#include <DatatypeDef.h>

namespace util
{
	class LineAreaChecker
	{
		public:
			/**
			 * Check if given point is inside "pipe" with given width,
			 * defined by line start and end position.
			 */
			static bool isPointInsideLineArea(const VC3 &point, 
				const VC3 &lineStart, const VC3 &lineEnd, float lineWidth);
	};
}

#endif


