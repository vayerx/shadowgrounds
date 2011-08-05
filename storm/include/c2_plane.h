// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_C2_PLANE_H
#define INCLUDED_C2_PLANE_H

//------------------------------------------------------------------
// Prototypes and typedefs
//------------------------------------------------------------------
template <class A> class TPlane;
typedef TPlane<float> PLANE;


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "c2_common.h"
#include "c2_vectors.h"
#include "c2_matrix.h"
#include "c2_quat.h"



//------------------------------------------------------------------
// Plane
//------------------------------------------------------------------
template <class A> class TPlane
{

public:

	// Data (public for speed)
	Vec3<A> planenormal;
	A range_to_origin;

	// Creation
	TPlane() : planenormal(0,1,0),range_to_origin(0) {}

	TPlane(const Vec3<A> &planenormal,A range_to_origin)
	{
		MakeFromNormalAndRange(planenormal,range_to_origin);
	}

	TPlane(const Vec3<A> &planenormal,const Vec3<A> &planepos)
	{
		MakeFromNormalAndPosition(planenormal,planepos);
	}

	TPlane(const Vec3<A> &point1,const Vec3<A> &point2,const Vec3<A> &point3)
	{
		MakeFromPoints(point1,point2,point3);
	}

	// Make from (conversion)
	void MakeFromNormalAndRange(const Vec3<A> &_planenormal,A _range_to_origin)
	{	
		planenormal=_planenormal;
		range_to_origin=_range_to_origin;
	}

	void MakeFromNormalAndPosition(const Vec3<A> &_planenormal,const Vec3<A> &planepos)
	{
		planenormal=_planenormal;
		range_to_origin=planenormal.GetDotWith(planepos);
	}

	void MakeFromPoints(const Vec3<A> &point1,const Vec3<A> &point2,const Vec3<A> &point3)
	{
		Vec3<A> edge1=point2-point1;
		Vec3<A> edge2=point3-point1;
		planenormal=edge1.GetCrossWith(edge2);
		planenormal.Normalize();
		range_to_origin=planenormal.GetDotWith(point1);
	}

	// Functions (these do not modify plane)...
	A GetPointRange(const Vec3<A> &point) const		// Returns negative values if point is behind plane
	{
		return planenormal.GetDotWith(point)-range_to_origin;
	}

	TPlane GetTransformed(const TMatrix<A> &matrix) const
	{
		TPlane temp(*this);
		temp.Transform(matrix);
		return temp;
	}

	TPlane GetRotated(const Quat<A> &rotation) const
	{
		TPlane temp(*this);
		temp.Rotate(rotation);
		return temp;
	}

	// Functions (these modify this plane)
	void Transform(const TMatrix<A> &matrix)
	{
		// Calculate position first
		Vec3<A> pos=planenormal*range_to_origin;
		matrix.TransformVector(pos);
		matrix.GetWithoutTranslation().GetInverse().GetTranspose().TransformVector(planenormal);

		// Calculate new range to origin
		range_to_origin=planenormal.GetDotWith(pos);
	}

	void Rotate(const Quat<A> &rotation)
	{
		// Calculate position first
		Vec3<A> pos=planenormal*range_to_origin;

		// Convert the quaternion to matrix first
		// Optimized code... Messy but fast... (even a P4 should survive with these 9 mults;)
		A x2=rotation.x+rotation.x;
		A y2=rotation.y+rotation.y;
		A z2=rotation.z+rotation.z;

		A xx2=rotation.x*x2; 
		A yy2=rotation.y*y2;
		A zz2=rotation.z*z2;
		A xy2=rotation.x*y2;
		A xz2=rotation.x*z2;
		A yz2=rotation.y*z2;
		A wx2=rotation.w*x2;
		A wy2=rotation.w*y2;
		A wz2=rotation.w*z2;
    
		A mat[9];
		mat[0]=1.0f-yy2+zz2;
		mat[1]=xy2-wz2;
		mat[2]=xz2+wy2;

		mat[3]=xy2+wz2;
		mat[4]=1.0f-xx2+zz2;
		mat[5]=yz2-wx2;

		mat[6]=xz2-wy2;
		mat[7]=yz2+wx2;
		mat[8]=1.0f-xx2+yy2;

		// Transform as 3x3
		// Much faster than transforming a 4x4 matrix (only 9 mults, compared to 16 of 4x4)
		A tmp_x=pos.x*mat[0]+pos.y*mat[3]+pos.z*mat[6];
		A tmp_y=pos.x*mat[1]+pos.y*mat[4]+pos.z*mat[7];
		A tmp_z=pos.x*mat[2]+pos.y*mat[5]+pos.z*mat[8];
		pos.x=tmp_x;
		pos.y=tmp_y;
		pos.z=tmp_z;

		// Calc new normal vector
		planenormal=pos/range_to_origin;
	}

	// Operators
	bool operator==(const TPlane& other) const
	{
		if ((fabs(planenormal.x-other.planenormal.x)<EPSILON)&&
			(fabs(planenormal.y-other.planenormal.y)<EPSILON)&&
			(fabs(planenormal.z-other.planenormal.z)<EPSILON)&&
			(fabs(range_to_origin-other.range_to_origin)<EPSILON)) return true;
		return false;
	}

	// Vector modify...
	void MirrorVector(Vec3<A> &vector) const
	{
		A range=planenormal.GetDotWith(vector)-range_to_origin;
		vector-=planenormal*(2*range);
	}

	void FlattenVector(Vec3<A> &vector,float amount) const
	{
		A range=planenormal.GetDotWith(vector)-range_to_origin;
		vector-=planenormal*(amount*range);
	}

	void ProjectVector(Vec3<A> &vector) const
	{	
		A range=planenormal.GetDotWith(vector)-range_to_origin;
		vector-=planenormal*range;
	}

	const Vec3<A> GetMirroredVector(const Vec3<A> &vector) const
	{
		Vec3<A> temp=vector;
		MirrorVector(temp);
		return temp;
	}

	const Vec3<A> GetFlattenedVector(const Vec3<A> &vector,float amount) const
	{
		Vec3<A> temp=vector;
		FlattenVector(temp);
		return temp;
	}

	const Vec3<A> GetProjectedVector(const Vec3<A> &vector) const
	{	
		Vec3<A> temp=vector;
		ProjectVector(temp);
		return temp;
	}

	bool GetClip ( const Vec3<A> p1, const Vec3<A> p2, Vec3<A> * out ) const
	{
		// Checks if line defined by points p1 and p2 intersects the plane.
		// If it does, and if out != NULL, out will be the intersection point.
		// Otherwise returns false.

		float p1Range = GetPointRange( p1 );
		float p2Range = GetPointRange( p2 );

		if(p1Range == 0)
		{
			if(out)
			{
				*out = p1;
			}
			return true;
		}
		if(p2Range == 0)
		{
			if(out)
			{
				*out = p2;
			}
			return true;
		}


		bool p1Inside = (p1Range < 0);
		bool p2Inside = (p2Range < 0);
		if( p1Inside == p2Inside )
			return false;

		const Vec3<A> q0= planenormal * range_to_origin;
		const Vec3<A> q1= q0 - p1;
		const Vec3<A> q2= p2 - p1;
		const A w1 = q1.GetDotWith(planenormal);
		const A w2 = q2.GetDotWith(planenormal);

		assert( w2 != 0 ); // w2 should be non-zero because we already checked if at least one of the points are on the plane.

		A w3 = w1 / w2;

		if (w3 < 0.0f) w3 = 0.0f;
		if (w3 > 1.0f) w3 = 1.0f;
		//assert( w3 >= 0 && w3 <= 1 ); // w3 should be at range [0,1] since we already checked that intersection occurs for sure.

		if(out)
			*out = p1 + q2 * w3;

		return true;
	}

/*
	void clipLine(const VC3 &p1, const VC3 &p2, VC3 &result)
	{
		float d1 = GetPointRange(p1);
		float d2 = GetPointRange(p2);
		if(d1 < 0 && d2 < 0)
			return;
		if(d1 > 0 && d2 > 0)
			return;

		float s = (d1 / (d1 - d2));
		result = p1 + ((p2 - p1) * s);
	}
*/


};


#endif // INCLUDED_C2_PLANE_H
