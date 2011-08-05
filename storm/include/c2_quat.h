// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once
#include "c2_vectors.h"

//------------------------------------------------------------------
// Prototypes and typedefs
//------------------------------------------------------------------
template <class A> class Quat;
typedef Quat<float> QUAT;


//------------------------------------------------------------------
// Quaternion
//------------------------------------------------------------------
template <class A> class Quat
{

public:

	// Data (public for speed)
	A x,y,z,w;

	// Creation...

	// I-Quaternion (0,0,0,1)
	Quat() : x(0),y(0),z(0),w((A)1) {};

	// Euler angles
	Quat(A xang,A yang,A zang)
	{
		MakeFromAngles(xang,yang,zang);
	}

	// Axis rotation
	Quat(const Vec3<A> &axis,A angle)
	{
		MakeFromAxisRotation(axis,angle);
	}
	
	// Quaternion
	Quat(A _x,A _y,A _z,A _w) : x(_x),y(_y),z(_z),w(_w) {}

	// Quaternion
	Quat(const A f[4]) : x(f[0]),y(f[1]),z(f[2]),w(f[3]) {}

	// Make from (conversion)
	void MakeFromAngles(A xang=0,A yang=0,A zang=0)
	{
		/*
		float xdeg = -xang * .5f;
		float qxx = sinf(xdeg);
		float qxw = cosf(xdeg);

		float ydeg = -yang * .5f;
		float qyy = sinf(ydeg);
		float qyw = cosf(ydeg);

		float zdeg = -zang * .5f;
		float qzz = sinf(zdeg);
		float qzw = cosf(zdeg);

		//result = z * (x * y);
		Quat qx(qxx, 0, 0, qxw);
		Quat qy(0, qyy, 0, qyw);
		Quat qz(0, 0, qzz, qzw);

		//Quat r = qz * (qx * qy);
		qx.Multiply(qy);
		qz.Multiply(qx);

		x = qz.x;
		y = qz.y;
		z = qz.z;
		w = qz.w;
		*/
		
		Quat qx,qy,qz,qf;

		bool xrot,yrot,zrot;
		if (fabs(xang)>EPSILON) xrot=true; else xrot=false;
		if (fabs(yang)>EPSILON) yrot=true; else yrot=false;
		if (fabs(zang)>EPSILON) zrot=true; else zrot=false;

		if (xrot)
		{
			A deg = -xang/(A)2;
			qx.x=(A)sin(deg);
			qx.y=0; 
			qx.z=0; 
			qx.w=(A)cos(deg);
		}

		if (yrot)
		{
			A deg=-yang/(A)2;
			qy.x=0; 
			qy.y=(A)sin(deg);
			qy.z=0;
			qy.w=(A)cos(deg);
		}

		if (zrot)
		{
			A deg=zang/(A)2;
			qz.x=0;
			qz.y=0;
			qz.z=(A)sin(deg);
			qz.w=(A)cos(deg);
		}

		if (xrot)
		{
			if (yrot)
			{
				if (zrot)
				{
					qf=qx*qz;
					(*this)=qf*qy;
				}
				else
				{
					(*this)=qx*qy;
				}
			}
			else
			{
				if (zrot)
				{
					(*this)=qx*qz;
				}
				else
				{
					x=qx.x;
					y=qx.y;
					z=qx.z;
					w=qx.w;
				}
			}
		}
		else
		{
			if (yrot)
			{
				if (zrot)
				{
					(*this)=qz*qy;
				}
				else
				{
					x=qy.x;
					y=qy.y;
					z=qy.z;
					w=qy.w;
				}
			}
			else
			{
				if (zrot)
				{
					x=qz.x;
					y=qz.y;
					z=qz.z;
					w=qz.w;
				}
				else
				{
					x=0;
					y=0;
					z=0;
					w=(A)1;
				}
			}
		}
	}

	void MakeFromAxisRotation(const Vec3<A> &axis=Vec3<A>(0,1,0),A ang=0)
	{
		const A deg=ang/(A)2;
		const A cs=(A)sin(deg);
		w=(A)cos(deg);
		x=cs*axis.x;
		y=cs*axis.y;
		z=cs*axis.z;
	}
	
	void MakeFromQuaternion(A _x=0,A _y=0,A _z=0,A _w=1)
	{
		x=_x;
		y=_y;
		z=_z;
		w=_w;
	}

	// Conversion to
	void ConvertToAxisRotation(Vec3<A> &axis,A &ang)
	{
		const A tw=(A)acos(w);
		const A scale=(A)((A)1/(A)sin(tw));
		ang=tw*(A)2;
		axis.x=x*scale;
		axis.y=y*scale;
		axis.z=z*scale;
	}

	// Functions (these do not modify this quaternion)
	Quat GetNormalized() const
	{
		const A inv_length=(A)1/(A)sqrt(x*x+y*y+z*z+w*w);
		return Quat(x*inv_length,y*inv_length,z*inv_length,w*inv_length);
	}

	Quat GetInverse() const
	{
		return Quat(-x,-y,-z,w);
	}

	bool GetIsIdentity() const
	{
		// Test if (0,0,0,1)
		if ((fabs(x)<0.001)&&
			(fabs(y)<0.001)&&
			(fabs(z)<0.001)&&
			(fabs(w-(A)1)<0.001)) return true;

		return false;
	}

	Quat GetSLInterpolationWith(const Quat &other,A interpolation) const	// interpolation value range: [0,1]
	{
		// Temp variables
		A ox=other.x;
		A oy=other.y;
		A oz=other.z;
		A ow=other.w;

		// Compute dot product (equal to cosine of the angle between quaternions)
		A fCosTheta=x*ox+y*oy+z*oz+w*ow;

		// Check angle to see if quaternions are in opposite hemispheres
		if(fCosTheta<0) 
		{
			// If so, flip one of the quaterions
			fCosTheta=-fCosTheta;
			ox=-ox;
			oy=-oy;
			oz=-oz;
			ow=-ow;
		}

		// Set factors to do linear interpolation, as a special case where the
		// quaternions are close together.
		A fBeta=(A)1-interpolation;
    
		// If the quaternions aren't close, proceed with spherical interpolation
		if((A)1-fCosTheta>(A)0.001) 
		{   
	        A fTheta=(A)acos(fCosTheta);
		    fBeta=(A)sin(fTheta*fBeta)/(A)sin(fTheta);
			interpolation=(A)sin(fTheta*interpolation)/(A)sin(fTheta);
		}

		// Do the interpolation
		return Quat(fBeta*x+interpolation*ox,fBeta*y+interpolation*oy,
			fBeta*z+interpolation*oz,fBeta*w+interpolation*ow);
	}

	// Functions (these modify this quaternion)
	void Normalize()
	{
		const A inv_length=(A)1/(A)sqrt(x*x+y*y+z*z+w*w);
		x*=inv_length;
		y*=inv_length;
		z*=inv_length;
		w*=inv_length;
	}

	void Inverse()
	{
		x=-x;
		y=-y;
		z=-z;
	}

	void Multiply(const Quat &other)
	{
		A Dx =  x*other.w + y*other.z - z*other.y + w*other.x;
		A Dy = -x*other.z + y*other.w + z*other.x + w*other.y;
		A Dz =  x*other.y - y*other.x + z*other.w + w*other.z;
		A Dw = -x*other.x - y*other.y - z*other.z + w*other.w;
		x = Dx; y = Dy; z = Dz; w = Dw;
	}

	// Operators
	Quat operator*(const Quat& other) const
	{
		A Dx =  x*other.w + y*other.z - z*other.y + w*other.x;
		A Dy = -x*other.z + y*other.w + z*other.x + w*other.y;
		A Dz =  x*other.y - y*other.x + z*other.w + w*other.z;
		A Dw = -x*other.x - y*other.y - z*other.z + w*other.w;
		return Quat(Dx,Dy,Dz,Dw);
	}
	
	bool operator==(const Quat& other) const
	{
		if ((fabs(x-other.x)<EPSILON)&&
			(fabs(y-other.y)<EPSILON)&&
			(fabs(z-other.z)<EPSILON)&&
			(fabs(w-other.w)<EPSILON)) return true;
		return false;
	}

	// Vector modify...
	void RotateVector(Vec3<A> &vector) const
	{
		// Convert the quaternion to matrix first
		// Optimized code... Messy but fast... (even a P4 should survive with these 9 mults;)
		A x2=x+x;
		A y2=y+y;
		A z2=z+z;

		A xx2=x*x2; 
		A yy2=y*y2;
		A zz2=z*z2;
		A xy2=x*y2;
		A xz2=x*z2;
		A yz2=y*z2;
		A wx2=w*x2;
		A wy2=w*y2;
		A wz2=w*z2;
    
		A mat[9];
		mat[0]=(A)1-yy2-zz2;
		mat[1]=xy2-wz2;
		mat[2]=xz2+wy2;

		mat[3]=xy2+wz2;
		mat[4]=(A)1-xx2-zz2;
		mat[5]=yz2-wx2;

		mat[6]=xz2-wy2;
		mat[7]=yz2+wx2;
		mat[8]=(A)1-xx2-yy2;

		// Transform as 3x3
		// Much faster than transforming a 4x4 matrix (only 9 mults, compared to 16 of 4x4)
		A tmp_x=vector.x*mat[0]+vector.y*mat[3]+vector.z*mat[6];
		A tmp_y=vector.x*mat[1]+vector.y*mat[4]+vector.z*mat[7];
		A tmp_z=vector.x*mat[2]+vector.y*mat[5]+vector.z*mat[8];
		vector.x=tmp_x;
		vector.y=tmp_y;
		vector.z=tmp_z;
	}

	Vec3<A> GetRotated(const Vec3<A> &vector) const
	{
		Vec3<A> temp(vector);
		RotateVector(temp);
		return temp;
	}

	Vec3<A> getEulerAngles() const;

    static void rotateToward(const VC3 &a, const VC3 &b, QUAT &result)
	{
		VC3 axis = a.GetCrossWith(b);
		float dot = a.GetDotWith(b);

		if(dot < -0.99f)
		{
			result = QUAT();
			return;
		}

		result.x = axis.x;
		result.y = axis.y;
		result.z = axis.z;
		result.w = (dot + 1.0f);
		result.Normalize();
	}
};

