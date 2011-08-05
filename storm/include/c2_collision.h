// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_C2_COLLISION_H
#define INCLUDED_C2_COLLISION_H

#include <stdlib.h>
#include "c2_vectors.h"
#include "c2_sphere.h"
#include "c2_aabb.h"
#include "c2_ray.h"
#include "c2_oobb.h"

	enum Outcode
	{
		RightClip = 1 << 0,
		LeftClip = 1 << 1,
		TopClip = 1 << 2,
		BottomClip = 1 << 3,
		FrontClip = 1 << 4,
		BackClip = 1 << 5
	};

static inline int getCohenSutherlandOutcode(const AABB &box, const VC3 &point)
	{
		int code = 0;

		const VC3 &max = box.mmax;
		const VC3 &min = box.mmin;

		float x = point.x;
		float y = point.y;
		float z = point.z;

		if(x > max.x)
			code |= RightClip;
		else if(x < min.x)
			code |= LeftClip;

		if(y > max.y)
			code |= TopClip;
		else if(y < min.y)
			code |= BottomClip;

		if(z > max.z)
			code |= BackClip;
		else if(z < min.z)
			code |= FrontClip;

		return code;
	}

inline bool collision(const Ray &ray, const Sphere &sphere)
{
	if(&ray == 0)
		abort();
	if(&sphere == 0)
		abort();

	VC3 distanceVector = sphere.position;
	distanceVector -= ray.origin;

	float rayRange = ray.range;
	float sphereRadius = sphere.radius;
	float distanceSquared = distanceVector.GetDotWith(distanceVector);

	// Out of range?
	float maxRange = rayRange + sphereRadius;
	if(maxRange * maxRange < distanceSquared)
		return false;

	float projectedDistance = distanceVector.GetDotWith(ray.direction);
	//float projectionRadius = distanceSquared - (projectedDistance * projectedDistance);
	//float projectionRadius = sqrtf(distanceSquared) - projectedDistance;
	float squaredRadius = sphereRadius * sphereRadius;

	// If projection to radius is bigger than actual radius, there can be no collision
	//if(projectionRadius > sphereRadius)
	//	return false;

	{
		VC3 pMinusC = ray.origin - sphere.position;
		float a = ray.direction.GetDotWith(ray.direction);
		float b = 2.f * ray.direction.GetDotWith(pMinusC);
		float c = pMinusC.GetDotWith(pMinusC) - (sphere.radius * sphere.radius);
		float discriminant = (b * b) - (4 * a * c);

		if(discriminant < 0)
			return false;
	}

	// Sphere behind ray?
	if((projectedDistance < 0) && (distanceSquared > squaredRadius))
		return false;

	return true;
}

inline bool collision(const Sphere &sphere, const Ray &ray)
{
	return collision(ray, sphere);
}

inline bool collision(const Ray &ray, const AABB &box)
{
	// Based on standard Cohen-Sutherland line clipping (Schroeder's implementation)
	// Clean and has nice early outs but propably not the most efficient solution

	const VC3 &start = ray.origin;
	VC3 directionVector = ray.direction * ray.range;
	VC3 end = start + directionVector;

	int outcode1 = getCohenSutherlandOutcode(box, start);
	if(!outcode1)
		return true; // inside?

	int outcode2 = getCohenSutherlandOutcode(box, end);
	if(!outcode2)
		return true; // inside?

	// Both points on same side?
	if((outcode1 & outcode2) > 0)
		return false;

	const VC3 &max = box.mmax;
	const VC3 &min = box.mmin;

	// Find actual intersection
	// Div by zero if ray is degenerate

	if(outcode1 & (RightClip | LeftClip)) 
	{
		float x = 0;
		if(outcode1 & RightClip) 
			x = max.x;
		else
			x = min.x;

		float x1 = end.x - start.x;
		float x2 = x - start.x;
		float y = start.y + x2 * (end.y - start.y) / x1;
		float z = start.z + x2 * (end.z - start.z) / x1;

		if(y <= max.y && y >= min.y && z <= max.z && z >= min.z)
			return true;
	}

	if(outcode1 & (TopClip | BottomClip)) 
	{
		float y = 0;
		if(outcode1 & TopClip)
			y = max.y;
		else
			y = min.y;

		float y1 = end.y - start.y;
		float y2 = y - start.y;
		float x = start.x + y2 * (end.x - start.x) / y1;
		float z = start.z + y2 * (end.z - start.z) / y1;

		if(x <= max.x && x >= min.x && z <= max.z && z >= min.z)
			return true;
	}

	if(outcode1 & (FrontClip | BackClip)) 
	{
		float z = 0;
		if(outcode1 & BackClip)
			z = max.z;
		else
			z = min.z;

		float z1 = end.z - start.z;
		float z2 = z - start.z;
		float x = start.x + z2 * (end.x - start.x) / z1;
		float y = start.y + z2 * (end.y - start.y) / z1;

		if(x <= max.x && x >= min.x && y <= max.y && y >= min.y)
			return true;
	}

	return false;
}

inline bool collision(const AABB &box, const Ray &ray)
{
	return collision(ray, box);
}

inline bool collision(const Sphere &sphereA, const Sphere &sphereB)
{
	VC3 direction = sphereA.position - sphereB.position;

	float distanceSquared = direction.GetDotWith(direction);
	float radiusSumSquared = sphereA.radius + sphereB.radius;
	radiusSumSquared *= radiusSumSquared;

	if(distanceSquared > radiusSumSquared)
		return false;

	return true;
}

inline bool collision(const Sphere &sphere, const AABB &box)
{
	const VC3 &spherePosition = sphere.position;
	float sphereRadius = sphere.radius;

	const VC3 &min = box.mmin;
	const VC3 &max = box.mmax;

	if(spherePosition.x > max.x + sphereRadius)
		return false;
	if(spherePosition.x < min.x - sphereRadius)
		return false;
	if(spherePosition.y > max.y + sphereRadius)
		return false;
	if(spherePosition.y < min.y - sphereRadius)
		return false;
	if(spherePosition.z > max.z + sphereRadius)
		return false;
	if(spherePosition.z < min.z - sphereRadius)
		return false;

	return true;
}

inline bool collision(const AABB &box, const Sphere &sphere)
{
	return collision(sphere, box);
}

inline bool contains(const AABB &box, const Sphere &sphere)
{
	const VC3 &spherePosition = sphere.position;
	float sphereRadius = sphere.radius;

	const VC3 &min = box.mmin;
	const VC3 &max = box.mmax;

	if(spherePosition.x + sphereRadius <= max.x)
	if(spherePosition.x - sphereRadius >= min.x)
	if(spherePosition.y + sphereRadius <= max.y)
	if(spherePosition.y - sphereRadius >= min.y)
	if(spherePosition.z + sphereRadius <= max.z)
	if(spherePosition.z - sphereRadius >= min.z)
		return true;
		
	return false;
}

#define C_EPSILON 0.0001f
#define OOBB_MARGIN 0.005f
#define MR(r, c) (((r) << 2) + (c))
inline bool collision(const AABB &aabbBox, const OOBB &obbBox)
{

	VC3 aabbCenter = (aabbBox.mmax + aabbBox.mmin) / 2.0f;
	VC3 aabbE = (aabbBox.mmax - aabbBox.mmin) / 2.0f;

	float ra, rb;
	MAT R, AbsR;
	R.Set( 0, obbBox.axes[0].x );
	R.Set( 1, obbBox.axes[1].x );
	R.Set( 2, obbBox.axes[2].x );
	R.Set( 4, obbBox.axes[0].y );
	R.Set( 5, obbBox.axes[1].y );
	R.Set( 6, obbBox.axes[2].y );
	R.Set( 8, obbBox.axes[0].z );
	R.Set( 9, obbBox.axes[1].z );
	R.Set(10, obbBox.axes[2].z );

	VC3 t = obbBox.center - aabbCenter;

	for(int l = 0; l < 16; l++)
		AbsR.Set (l, fabsf( R.Get(l) ) + C_EPSILON );

	ra = aabbE.x;
	rb = obbBox.extents.x * AbsR.Get(0) + obbBox.extents.y * AbsR.Get(1)
		+ obbBox.extents.z * AbsR.Get(2);
	if ( fabsf( t.x ) > ra + rb )
		return false;

	ra = aabbE.y;
	rb = obbBox.extents.x * AbsR.Get(4) + obbBox.extents.y * AbsR.Get(5)
		+ obbBox.extents.z * AbsR.Get(6);
	if ( fabsf( t.y ) > ra + rb )
		return false;

	ra = aabbE.z;
	rb = obbBox.extents.x * AbsR.Get(8) + obbBox.extents.y * AbsR.Get(9)
		+ obbBox.extents.z * AbsR.Get(10);
	if ( fabsf( t.z ) > ra + rb )
		return false;

	ra = aabbE.x * AbsR.Get(0) + aabbE.y * AbsR.Get(4)
		+ aabbE.z * AbsR.Get(8);
	rb = obbBox.extents.x;
	if ( fabsf( t.x * R.Get(0) + t.y * R.Get(4) + t.z * R.Get(8) ) > ra + rb )
		return false;
	ra = aabbE.x * AbsR.Get(1) + aabbE.y * AbsR.Get(5)
		+ aabbE.z * AbsR.Get(9);
	rb = obbBox.extents.y;
	if ( fabsf( t.x * R.Get(1) + t.y * R.Get(5) + t.z * R.Get(9) ) > ra + rb )
		return false;
	ra = aabbE.x * AbsR.Get(2) + aabbE.y * AbsR.Get(6)
		+ aabbE.z * AbsR.Get(10);
	rb = obbBox.extents.z;
	if ( fabsf( t.x * R.Get(2) + t.y * R.Get(6) + t.z * R.Get(10) ) > ra + rb )
		return false;

	ra = aabbE.y			 * AbsR.Get(MR(2,0)) + aabbE.z				* AbsR.Get(MR(1,0)) ;
	rb = obbBox.extents.y * AbsR.Get(MR(0,2)) + obbBox.extents.z	* AbsR.Get(MR(0,1)) ;
	if( fabsf( t.z			 * R.Get   (MR(1,0)) - t.y							* R.Get(MR(2,0)) ) > ra + rb )
		return false;

	ra = aabbE.y			 * AbsR.Get(MR(2,1)) + aabbE.z				* AbsR.Get(MR(1,1)) ;
	rb = obbBox.extents.x * AbsR.Get(MR(0,2)) + obbBox.extents.z	* AbsR.Get(MR(0,0)) ;
	if( fabsf( t.z			 * R.Get   (MR(1,1)) - t.y							* R.Get(MR(2,1)) ) > ra + rb )
		return false;

	ra = aabbE.y			 * AbsR.Get(MR(2,2)) + aabbE.z				* AbsR.Get(MR(1,2)) ;
	rb = obbBox.extents.x * AbsR.Get(MR(0,2)) + obbBox.extents.y	* AbsR.Get(MR(0,0)) ;
	if( fabsf( t.z			 * R.Get   (MR(1,2)) - t.y							* R.Get(MR(2,2)) ) > ra + rb )
		return false;

	ra = aabbE.x			 * AbsR.Get(MR(2,0)) + aabbE.z				* AbsR.Get(MR(0,0)) ;
	rb = obbBox.extents.y * AbsR.Get(MR(1,2)) + obbBox.extents.z	* AbsR.Get(MR(1,1)) ;
	if( fabsf( t.x			 * R.Get   (MR(2,0)) - t.z							* R.Get(MR(0,0)) ) > ra + rb )
		return false;

	ra = aabbE.x			 * AbsR.Get(MR(2,1)) + aabbE.z				* AbsR.Get(MR(0,1)) ;
	rb = obbBox.extents.x * AbsR.Get(MR(1,2)) + obbBox.extents.z	* AbsR.Get(MR(1,0)) ;
	if( fabsf( t.x			 * R.Get   (MR(2,1)) - t.z							* R.Get(MR(0,1)) ) > ra + rb )
		return false;

	ra = aabbE.x			 * AbsR.Get(MR(2,2)) + aabbE.z				* AbsR.Get(MR(0,2)) ;
	rb = obbBox.extents.x * AbsR.Get(MR(1,1)) + obbBox.extents.y	* AbsR.Get(MR(1,0)) ;
	if( fabsf( t.x			 * R.Get   (MR(2,2)) - t.z							* R.Get(MR(0,2)) ) > ra + rb )
		return false;

	ra = aabbE.x			 * AbsR.Get(MR(1,0)) + aabbE.y				* AbsR.Get(MR(0,0)) ;
	rb = obbBox.extents.y * AbsR.Get(MR(2,2)) + obbBox.extents.z	* AbsR.Get(MR(2,1)) ;
	if( fabsf( t.y			 * R.Get   (MR(0,0)) - t.x							* R.Get(MR(1,0)) ) > ra + rb )
		return false;

	ra = aabbE.x			 * AbsR.Get(MR(1,1)) + aabbE.y				* AbsR.Get(MR(0,1)) ;
	rb = obbBox.extents.x * AbsR.Get(MR(2,2)) + obbBox.extents.z	* AbsR.Get(MR(2,0)) ;
	if( fabsf( t.y			 * R.Get   (MR(0,1)) - t.x							* R.Get(MR(1,1)) ) > ra + rb )
		return false;

	ra = aabbE.x			 * AbsR.Get(MR(1,2)) + aabbE.y				* AbsR.Get(MR(0,2)) ;
	rb = obbBox.extents.x * AbsR.Get(MR(2,1)) + obbBox.extents.y	* AbsR.Get(MR(2,0)) ;
	if( fabsf( t.y			 * R.Get   (MR(0,2)) - t.x							* R.Get(MR(1,2)) ) > ra + rb )
		return false;

	return true;
}


#endif
