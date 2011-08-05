#ifndef INCLUDED_EXPORT_TYPES_H
#define INCLUDED_EXPORT_TYPES_H

#include <datatypedef.h>

namespace frozenbyte {
namespace exporter {

typedef TMatrix<double> FBMatrix;
typedef Quat<double> FBQuaternion;
typedef Vec3<double> FBVector;
typedef Vec2<double> FBVector2;

MAT convert(const FBMatrix &m);
QUAT convert(const FBQuaternion &q);
VC3 convert(const FBVector &v);
VC2 convert(const FBVector2 &v);

} // export
} // frozenbyte

#endif
