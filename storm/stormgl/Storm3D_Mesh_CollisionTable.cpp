// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d_mesh_collisiontable.h"
#include "storm3d_mesh.h"
#include "igios3D.h"
#include "../../util/Debug_MemoryManager.h"

#include <algorithm>
#include <string>
#include <boost/lexical_cast.hpp>

namespace {

	void getClosestPoint(const VC3 &p, const VC3 &a, const VC3 &b, const VC3 &c, VC3 &result)
	{
		VC3 ab = b - a;
		VC3 ac = c - a;
		VC3 ap = p - a;

		float d1 = ab.GetDotWith(ap);
		float d2 = ac.GetDotWith(ap);
		if(d1 <= 0.f && d2 <= 0.f)
		{
			// Barycentric (1,0,0)
			result = a;
			return;
		}

		VC3 bp = p - b;
		float d3 = ab.GetDotWith(bp);
		float d4 = ac.GetDotWith(bp);
		if(d3 >= 0.f && d4 <= d3)
		{
			// Barycentric (0,1,0)
			result = b;
			return;
		}

		float vc = d1*d4 - d3*d2;
		if(vc <= 0.f && d1 >= 0.f && d3 <= 0.f)
		{
			// Barycentric (1-v,v,0)
			float v = d1 / (d1 - d3);
			result = a + (ab * v);
			return;
		}

		VC3 cp = p - c;
		float d5 = ab.GetDotWith(cp);
		float d6 = ac.GetDotWith(cp);
		if(d6 >= 0.f && d5 <= d6)
		{
			// Barycentric (0,0,1)
			result = c;
			return;
		}

		float vb = d5*d2 - d1*d6;
		if(vb <= 0.f && d2 >= 0.f && d6 <= 0.f)
		{
			// Barycentric (1-w,0,w)
			float w = d2 / (d2 - d6);
			result = a + (ac * w);
			return;
		}

		float va = d3*d6 - d5*d4;
		if(va <= 0.f && (d4 - d3) >= 0.f && (d5 - d6) >= 0.f)
		{
			// Barycentric (0,1-w,w)
			float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
			result = b + ((c - b) * w);
			return;
		}

		// Inside face now
		float denom = 1.f / (va + vb + vc);
		float v = vb * denom;
		float w = vc * denom;
		result = a + (ab * v) + (ac * w);
	}

} // unnamed

struct CollisionFace
{
	Sphere sphere;

	// Ray stuff
	VC3 vertex0;
	VC3 vertex1;
	VC3 vertex2;

	VC3 e01;
	VC3 e02;
	PLANE plane;

	//VC3 center_position;
	//float radius;

	void RayTrace(const VC3 &position,const VC3 &direction_normalized,float ray_length,Storm3D_CollisionInfo &rti, bool accurate = true);
	void SphereCollision(const VC3 &position,float radius,Storm3D_CollisionInfo &rti, bool accurate = true);

	bool fits(const AABB &area) const
	{
		if(!contains2D(area, vertex0))
			return false;
		if(!contains2D(area, vertex1))
			return false;
		if(!contains2D(area, vertex2))
			return false;

		return true;
	}
};

void CollisionFace::RayTrace(const VC3 &position, const VC3 &direction, float ray_length, Storm3D_CollisionInfo &rti, bool accurate)
{
	// Begin calculating determinant - also used to calculate U parameter
	VC3 pvec(direction.y * e02.z - direction.z * e02.y, direction.z * e02.x - direction.x * e02.z, direction.x * e02.y - direction.y * e02.x);

	// If determinant is near zero, ray lies in plane of triangle
	float det = e01.x * pvec.x + e01.y*pvec.y + e01.z * pvec.z;
	if(det < 0.0001f) 
		return;

	// Calculate distance from vert0 to ray origin
	VC3 tvec(position.x - vertex0.x, position.y - vertex0.y, position.z - vertex0.z);

	// Calculate U parameter and test bounds
	float u = tvec.x * pvec.x + tvec.y * pvec.y + tvec.z * pvec.z;
	if((u < 0) || (u > det)) 
		return;

	// Prepare to test V parameter
	VC3 qvec(tvec.y * e01.z - tvec.z * e01.y, tvec.z * e01.x - tvec.x * e01.z, tvec.x * e01.y - tvec.y * e01.x);
	
	// Calculate V parameter and test bounds
	float v = direction.x * qvec.x + direction.y * qvec.y + direction.z * qvec.z;
	if((v < 0) || ((u + v) > det))
		return;

	// Calculate t, scale parameters, ray intersects triangle
	float t = e02.x * qvec.x + e02.y * qvec.y + e02.z * qvec.z;
	t /= det;

	// Set collision info
	if((t < rti.range) && (t < ray_length) && (t > 0))
	{
		rti.hit = true;
		rti.range = t;
		rti.position = position+direction*t;
		rti.plane_normal = plane.planenormal;
	}
}

void CollisionFace::SphereCollision(const VC3 &position, float radius, Storm3D_CollisionInfo &colinfo, bool accurate)
{
	VC3 closest;
	getClosestPoint(position, vertex0, vertex1, vertex2, closest);

	float sqRange = closest.GetSquareRangeTo(position);
	if(sqRange < radius * radius && sqRange < colinfo.range * colinfo.range)
	{
		colinfo.hit = true;
		colinfo.range = sqrtf(sqRange);
		colinfo.position = closest;
		colinfo.plane_normal = plane.planenormal;
		colinfo.inside_amount = radius - colinfo.range;

		float planeRange = plane.GetPointRange(position);
		if(planeRange < 0)
			colinfo.plane_normal = -colinfo.plane_normal;
	}
}

//------------------------------------------------------------------
// Storm3D_Mesh_CollisionTable::Storm3D_Mesh_CollisionTable
//------------------------------------------------------------------
Storm3D_Mesh_CollisionTable::Storm3D_Mesh_CollisionTable() :
	faces(0),
	face_amount(0),
	tree(0)
{
}



//------------------------------------------------------------------
// Storm3D_Mesh_CollisionTable::~Storm3D_Mesh_CollisionTable
//------------------------------------------------------------------
Storm3D_Mesh_CollisionTable::~Storm3D_Mesh_CollisionTable()
{
	delete tree;
	SAFE_DELETE_ARRAY(faces);
}

//------------------------------------------------------------------
// Storm3D_Mesh_CollisionTable::ReBuild
//------------------------------------------------------------------
void Storm3D_Mesh_CollisionTable::ReBuild(Storm3D_Mesh *mesh)
{
	// TEMP!!!
	//return;

	// Set owner
	owner=mesh;

	// Allocate memory for collision table
	delete tree;
	tree = 0;

	if(!owner->vertexes || !owner->faces[0])
		return;

	face_amount=owner->face_amount[0];
	SAFE_DELETE_ARRAY(faces);
	faces = new CollisionFace[face_amount];

	VC2 mmin(10000000.f, 10000000.f);
	VC2 mmax(-10000000.f, -10000000.f);

	// Create collision table
	for(int fi = 0; fi < face_amount; ++fi)
	{
		// Set vertexes
		faces[fi].vertex0=owner->vertexes[owner->faces[0][fi].vertex_index[0]].position;
		faces[fi].vertex1=owner->vertexes[owner->faces[0][fi].vertex_index[1]].position;
		faces[fi].vertex2=owner->vertexes[owner->faces[0][fi].vertex_index[2]].position;

		// Create edges
		faces[fi].e01=owner->vertexes[owner->faces[0][fi].vertex_index[1]].position-owner->vertexes[owner->faces[0][fi].vertex_index[0]].position;
		faces[fi].e02=owner->vertexes[owner->faces[0][fi].vertex_index[2]].position-owner->vertexes[owner->faces[0][fi].vertex_index[0]].position;
		VC3 e12=owner->vertexes[owner->faces[0][fi].vertex_index[2]].position-owner->vertexes[owner->faces[0][fi].vertex_index[1]].position;

		VC3 cross_e01_e02 = faces[fi].e01.GetCrossWith(faces[fi].e02);
		if(cross_e01_e02.GetSquareLength() < 0.0001f)
		{
			CollisionFace &face = faces[fi];
			face.sphere.radius = 0;

			continue;
		}

		// Create plane
		faces[fi].plane=PLANE(cross_e01_e02.GetNormalized(),faces[fi].vertex0);

		// psd
		CollisionFace *face = &faces[fi];
		face->sphere.position = (face->vertex0 + face->vertex1 + face->vertex2) / 3;
		
		face->sphere.radius = face->sphere.position.GetSquareRangeTo(face->vertex0);
		float tmp = face->sphere.position.GetSquareRangeTo(face->vertex1);
		if(tmp > face->sphere.radius)
			face->sphere.radius = tmp;
		tmp = face->sphere.position.GetSquareRangeTo(face->vertex2);
		if(tmp > face->sphere.radius)
			face->sphere.radius = tmp;

		face->sphere.radius = sqrtf(face->sphere.radius);

		if(face->sphere.position.x - face->sphere.radius < mmin.x)
			mmin.x = face->sphere.position.x - face->sphere.radius;
		if(face->sphere.position.z - face->sphere.radius < mmin.y)
			mmin.y = face->sphere.position.z - face->sphere.radius;

		if(face->sphere.position.x + face->sphere.radius > mmax.x)
			mmax.x = face->sphere.position.x + face->sphere.radius;
		if(face->sphere.position.z + face->sphere.radius > mmax.y)
			mmax.y = face->sphere.position.z + face->sphere.radius;
	}

	tree = new StaticQuadtree<CollisionFace>(mmin, mmax);
	//tree = new Quadtree<CollisionFace>(mmin, mmax);
	for(int i = 0; i < face_amount; ++i)
	{
		CollisionFace &face = faces[i];
		if(face.sphere.radius < 0.001f)
			continue;

		//tree->insert(&face, face.sphere.position, face.sphere.radius);

		float mmin = min(face.vertex0.y, face.vertex1.y);
		mmin = min(mmin, face.vertex2.y);
		float mmax = max(face.vertex0.y, face.vertex1.y);
		mmax = max(mmax, face.vertex2.y);

		tree->insert(face, face.sphere.position, face.sphere.radius, mmin, mmax);
	}
}



//------------------------------------------------------------------
// Storm3D_Mesh_CollisionTable::RayTrace
//------------------------------------------------------------------
bool Storm3D_Mesh_CollisionTable::RayTrace(const VC3 &position,const VC3 &direction,float ray_length,Storm3D_CollisionInfo &rti, bool accurate)
{
	if(!tree)
		return false;

	bool oldHit = rti.hit;
	float oldRange = rti.range;

	Ray ray(position, direction, ray_length);
	tree->RayTrace(ray, rti, accurate);

	if(oldHit && rti.range < oldRange)
		return true;
	if(!oldHit && rti.hit)
		return true;

	return false;
}

//------------------------------------------------------------------
// Storm3D_Mesh_CollisionTable::SphereCollision
//------------------------------------------------------------------
bool Storm3D_Mesh_CollisionTable::SphereCollision(const VC3 &position,float radius,Storm3D_CollisionInfo &colinfo, bool accurate)
{
	if(!tree)
		return false;

	bool hit = colinfo.hit;
	float range = colinfo.range;

	Sphere sphere(position, radius);
	tree->SphereCollision(sphere, colinfo, accurate);

	if(hit && colinfo.range < range)
		return true;
	if(!hit && colinfo.hit)
		return true;

	return false;
}

void Storm3D_Mesh_CollisionTable::reset()
{
	delete tree;
	tree = 0;

	delete[] faces;
	faces = NULL;
	owner = 0;
}

