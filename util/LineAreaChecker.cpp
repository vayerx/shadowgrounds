
#include "precompiled.h"

#include "LineAreaChecker.h"

namespace util
{
	bool LineAreaChecker::isPointInsideLineArea(const VC3 &point, 
		const VC3 &lineStart, const VC3 &lineEnd, float lineWidth)
	{
		VC3 projPos = lineEnd - lineStart;
		float projPosLenSq = projPos.GetSquareLength();
		VC3 projPosNorm = VC3(0,0,0);
		if (projPosLenSq > 0.00001f)
		{
			projPosNorm = projPos.GetNormalized();
		}

		VC3 chkpos = point - lineStart;
		float chkposLen = chkpos.GetLength();

		VC3 hitAndUnitDiff = chkpos - projPos;
		float hitAndUnitDiffLenSq = hitAndUnitDiff.GetSquareLength();

		float lineRadiusSq = (float)(lineWidth * lineWidth);

		if (hitAndUnitDiffLenSq < lineRadiusSq)
		{
			return true;
		}

		if (chkposLen * chkposLen < projPosLenSq)
		{
			VC3 pipedPos = projPosNorm * chkposLen;
			VC3 posdiff = chkpos - pipedPos;

			if (posdiff.GetSquareLength() < lineRadiusSq)
			{
				return true;
			}
		}

		return false;
	}
}

