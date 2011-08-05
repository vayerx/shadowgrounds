
#ifndef CLIPPEDCIRCLE_H
#define CLIPPEDCIRCLE_H

#include <DatatypeDef.h>

// clockwise - 1st=nw, 2nd=ne, 3rd=se, 4th=sw (ok?)
#define CLIPPEDCIRCLE_QUARTER_NW (1)
#define CLIPPEDCIRCLE_QUARTER_NE (1<<1)
#define CLIPPEDCIRCLE_QUARTER_SE (1<<2)
#define CLIPPEDCIRCLE_QUARTER_SW (1<<3)

namespace util
{
	typedef struct 
	{
		bool insideArea;
		float distanceFactor;
		float rangeToCenter;
	} ClippedCircleValues;

  class ClippedCircle
	{
		public:
			static ClippedCircleValues getValuesFor(const VC3 &position, 
				const VC3 &circlePosition, float circleRadius, int quarterBitMask, 
				float clipSmoothRange = 0.0f);

			static bool isInsideArea(const VC3 &position, 
				const VC3 &circlePosition, float circleRadius, int quarterBitMask);


			ClippedCircle(const VC3 &circlePosition, float circleRadius, int quarterBitMask);
			ClippedCircle();
			~ClippedCircle();

			void setPosition(const VC3 &position) { circlePosition = position; }
			void setRadius(float radius) { circleRadius = radius; }
			void setQuarterMask(int mask) { quarterBitMask = mask; }

			bool isInsideArea(const VC3 &position) const;
			const VC3 &getPosition() const { return circlePosition; }
			float getRadius() const { return circleRadius; }

		private:
			VC3 circlePosition;
			float circleRadius;
			int quarterBitMask;

	};
}

#endif

