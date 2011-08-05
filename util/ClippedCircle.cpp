
#include "precompiled.h"

#include "ClippedCircle.h"

namespace util
{
	ClippedCircleValues ClippedCircle::getValuesFor(const VC3 &position, 
		const VC3 &circlePosition, float circleRadius, int quarterBitMask, 
		float clipSmoothRange)
	{
		ClippedCircleValues ret;

		ret.distanceFactor = 0.0f;
		ret.insideArea = false;
		ret.rangeToCenter = 0.0f;

		if (clipSmoothRange != 0.0f)
		{
			assert(!"ClippedCircle::getValuesFor - TODO, Support for clipSmoothRange.");
			return ret;
		}

		VC3 diff = position - circlePosition;
		diff.y = 0.0f; // (use 2d range, not 3d range)

		float distSq = diff.GetSquareLength();
		float dist = sqrtf(distSq);

		ret.rangeToCenter = dist;

		// outside circle radius?
		if(distSq > circleRadius * circleRadius)  
		{
			return ret;
		}

		// inside the radius at least...
		ret.distanceFactor = 1.0f - (dist / circleRadius);
		ret.insideArea = true;
		assert(ret.distanceFactor >= 0.0f && ret.distanceFactor <= 1.0f);

		// but if not inside quarters, then it's gonna be false and zero distance factor.
		// NOTE: positions on axis part of East/South quarters... (but center always true)

		if (diff.y != 0.0f || diff.x != 0.0f)
		{
			if (diff.y < 0)
			{
				if (diff.x < 0)
				{
					if ((quarterBitMask & CLIPPEDCIRCLE_QUARTER_NW) == 0)
					{
						// TODO: clipSmoothRange
						ret.distanceFactor = 0.0f; 
						ret.insideArea = false;
					}
				} else {
					if ((quarterBitMask & CLIPPEDCIRCLE_QUARTER_NE) == 0)
					{
						// TODO: clipSmoothRange
						ret.distanceFactor = 0.0f; 
						ret.insideArea = false;
					}
				}
			} else {
				if (diff.x < 0)
				{
					if ((quarterBitMask & CLIPPEDCIRCLE_QUARTER_SW) == 0)
					{
						// TODO: clipSmoothRange
						ret.distanceFactor = 0.0f; 
						ret.insideArea = false;
					}
				} else {
					if ((quarterBitMask & CLIPPEDCIRCLE_QUARTER_SE) == 0)
					{
						// TODO: clipSmoothRange
						ret.distanceFactor = 0.0f; 
						ret.insideArea = false;
					}
				}
			}
		}

		return ret;
	}



	bool ClippedCircle::isInsideArea(const VC3 &position, 
		const VC3 &circlePosition, float circleRadius, int quarterBitMask)
	{
		VC3 diff = position - circlePosition;
		diff.y = 0.0f; // (use 2d range, not 3d range)

		float distSq = diff.GetSquareLength();

		if(distSq > circleRadius * circleRadius)  
		{
			return false;
		}

		// Check for clipped quarters...
		// NOTE: positions on axis part of East/South quarters... (but center always true)

		if (diff.y != 0.0f || diff.x != 0.0f)
		{
			if (diff.y < 0)
			{
				if (diff.x < 0)
				{
					if ((quarterBitMask & CLIPPEDCIRCLE_QUARTER_NW) == 0)
						return false;
				} else {
					if ((quarterBitMask & CLIPPEDCIRCLE_QUARTER_NE) == 0)
						return false;
				}
			} else {
				if (diff.x < 0)
				{
					if ((quarterBitMask & CLIPPEDCIRCLE_QUARTER_SW) == 0)
						return false;
				} else {
					if ((quarterBitMask & CLIPPEDCIRCLE_QUARTER_SE) == 0)
						return false;
				}
			}
		}

		return true;
	}

	ClippedCircle::ClippedCircle(const VC3 &circlePosition, float circleRadius, int quarterBitMask)
	{
		this->circlePosition = circlePosition;
		this->circleRadius = circleRadius;
		this->quarterBitMask = quarterBitMask;
	}

	ClippedCircle::ClippedCircle()
	:	circleRadius(0),
		quarterBitMask(0)
	{
	}

	ClippedCircle::~ClippedCircle()
	{
		// nop
	}

	bool ClippedCircle::isInsideArea(const VC3 &position) const
	{
		return isInsideArea(position, this->circlePosition, this->circleRadius, this->quarterBitMask);
	}


}

