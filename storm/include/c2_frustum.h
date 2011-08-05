// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_C2_FRUSTUM_H
#define INCLUDED_C2_FRUSTUM_H

#include "c2_vectors.h"
#include "c2_sphere.h"
#include "c2_aabb.h"
#include "c2_oobb.h"
#include "c2_plane.h"

template<class T>
struct TFrustum
{
	Vec3<T> planeNormals[5];
	Vec3<T> position;
	float range;

	TFrustum()
	:	range(0.f)
	{
	}

	TFrustum(const Vec3<T> *planeNormals_, const Vec3<T> &position_, float range_)
	:	position(position_),
		range(range_)
	{
		for(int i = 0; i < 5; ++i)
			planeNormals[i] = planeNormals_[i];
	}

	bool visibility(const TSphere<T> &sphere, bool checkRange = false) const
	{
		VC3 dif = sphere.position;
		dif -= position;

		if(checkRange)
		{
			float squareDistance = dif.GetSquareLength();
			float maxDistanceSquared = sphere.radius + range;
			maxDistanceSquared *= maxDistanceSquared;

			if(squareDistance > maxDistanceSquared)
				return false;
		}

		// Back/Front planes
		float distance = dif.GetDotWith(planeNormals[0]);
		if(distance < -sphere.radius) 
			return false;
		if(distance > range + sphere.radius) 
			return false;

		// Side Planes
		distance = dif.GetDotWith(planeNormals[1]);
		if(distance < -sphere.radius) 
			return false;
		distance = dif.GetDotWith(planeNormals[2]);
		if(distance < -sphere.radius) 
			return false;

		// Up/Down planes
		distance = dif.GetDotWith(planeNormals[3]);
		if(distance < -sphere.radius) 
			return false;
		distance = dif.GetDotWith(planeNormals[4]);
		if(distance < -sphere.radius) 
			return false;

		return true;
	}

	bool visibility(const TOOBB<T> &box) const
	{
		for(int i = 0; i < 5; ++i)
		{
			float r = box.extents.x * fabsf(planeNormals[i].GetDotWith(box.axes[0])) +
					  box.extents.y * fabsf(planeNormals[i].GetDotWith(box.axes[1])) +
					  box.extents.z * fabsf(planeNormals[i].GetDotWith(box.axes[2]));

			float d = planeNormals[i].GetDotWith(position);
			float s = planeNormals[i].GetDotWith(box.center) - d;

			if(s < -r)
				return false;
			if(i == 0)
			{
				float sq = range + sqrtf(r);
				sq *= sq;
				if(s > sq)
					return false;
			}
		}

		return true;
	}

	bool visibility ( const Vec3<T> point )
	{
		// The back plane.
		PLANE frustumPlane;
		frustumPlane.MakeFromNormalAndPosition(-planeNormals[0], position + planeNormals[0] * range);
		if(frustumPlane.GetPointRange( point ) < 0.0f)
			return false;

		for(int j = 1; j <= 4; j++)
		{
			frustumPlane.MakeFromNormalAndPosition( planeNormals[j], position);
			if(frustumPlane.GetPointRange( point ) < 0.0f)
				return false;
		}

		return true;
	}
  
	enum VisibilityValue
	{
		None,
		Partial,
		Full
	};

	VisibilityValue visibilityValue(const TSphere<T> &sphere) const
	{
		//if(visibility(sphere))
		//	return Full;

		VC3 dif = sphere.position - position;
		VisibilityValue result = Full;

		// Back/Front planes
		float distance = dif.GetDotWith(planeNormals[0]);
		if(distance < -sphere.radius) 
			return None;
		else if(distance < sphere.radius)
			result = Partial;

		if(distance > range + sphere.radius) 
			return None;
		else if(distance > range - sphere.radius)
			result = Partial;

		// Side Planes
		distance = dif.GetDotWith(planeNormals[1]);
		if(distance < -sphere.radius) 
			return None;
		else if(distance < sphere.radius)
			result = Partial;

		distance = dif.GetDotWith(planeNormals[2]);
		if(distance < -sphere.radius) 
			return None;
		else if(distance < sphere.radius)
			result = Partial;

		// Up/Down planes
		distance = dif.GetDotWith(planeNormals[3]);
		if(distance < -sphere.radius) 
			return None;
		else if(distance < sphere.radius)
			return Partial;

		distance = dif.GetDotWith(planeNormals[4]);
		if(distance < -sphere.radius) 
			return None;
		else if(distance < sphere.radius)
			return Partial;

		return result;
	}

	VisibilityValue visibilityValue(const TAABB<T> &aabb) const
	{
		TOOBB<T> box;
		box.center = aabb.mmax;
		box.center += aabb.mmin;
		box.center *= 0.5f;

		box.axes[0].x = 1.f;
		box.axes[0].y = 0.f;
		box.axes[0].z = 0.f;
		box.axes[1].x = 0.f;
		box.axes[1].y = 1.f;
		box.axes[1].z = 0.f;
		box.axes[2].x = 0.f;
		box.axes[2].y = 0.f;
		box.axes[2].z = 1.f;
		box.extents.x = (aabb.mmax.x - aabb.mmin.x) * .5f;
		box.extents.y = (aabb.mmax.y - aabb.mmin.y) * .5f;
		box.extents.z = (aabb.mmax.z - aabb.mmin.z) * .5f;

		/*
		if(visibility(box))
			return Full;
		else
			return None;
		*/

		VisibilityValue result = Full;
		for(int i = 0; i < 5; ++i)
		{
			float r = box.extents.x * fabsf(planeNormals[i].GetDotWith(box.axes[0])) +
					  box.extents.y * fabsf(planeNormals[i].GetDotWith(box.axes[1])) +
					  box.extents.z * fabsf(planeNormals[i].GetDotWith(box.axes[2]));
			float d = planeNormals[i].GetDotWith(position);
			float s = planeNormals[i].GetDotWith(box.center) - d;

			if(s < -r)
				return None;
			if(s < r)
				result = Partial;

			if(i == 0)
			{
				float sq = range + sqrtf(r);
				sq *= sq;
				if(s > sq)
					return None;
				if(s > sq - r)
					result = Partial;
			}
		}

		return result;

		/*
		// ToDo

		Vec3<T> position = aabb.mmax;
		position += aabb.mmin;
		position *= .5f;

		Vec3<T> dif = aabb.mmax;
		dif -= position;
		
		float radius = dif.GetLength();
		return visibilityValue(TSphere<T>(position, radius));
		*/
	}
};

typedef TFrustum<float> Frustum;

#endif
