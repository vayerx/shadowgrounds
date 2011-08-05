#include "Export_Types.h"

namespace frozenbyte {
namespace exporter {

MAT convert(const FBMatrix &m)
{
	MAT result;
	for(int i = 0; i < 16; ++i)
		result.Set(i, float(m.Get(i)));

	return result;
}

QUAT convert(const FBQuaternion &q)
{
	QUAT result;
	result.x = float(q.x);
	result.y = float(q.y);
	result.z = float(q.z);
	result.w = float(q.w);

	return result;
}

VC3 convert(const FBVector &v)
{
	VC3 result;
	result.x = float(v.x);
	result.y = float(v.y);
	result.z = float(v.z);

	return result;
}

VC2 convert(const FBVector2 &v)
{
	VC2 result;
	result.x = float(v.x);
	result.y = float(v.y);

	return result;
}

} // exporter
} // frozenbyte
