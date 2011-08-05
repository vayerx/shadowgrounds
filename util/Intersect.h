#ifndef INTERSECT_H
#define INTERSECT_H

/* Could I forward declare this? */
#include <Storm3D_Datatypes.h>


namespace util
{



/** Function tests if a given point is inside or outside a given point
  @param point The point to test
  @param a First point of the triangle
  @param b Second point of the triangle
  @param c Third point of the triangle
  @return true if inside the triangle, false otherwise.
*/
bool isPointInsideTriangle(const VC2& point, const VC2& a, const VC2& b, const VC2& c);



} /* namespace util */


#endif /* INTERSECT_H */

/* EOF */
