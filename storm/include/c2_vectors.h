// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once
#include <math.h>
#include <assert.h>
#include "c2_common.h"

//------------------------------------------------------------------
// Prototypes and typedefs
//------------------------------------------------------------------
template <class A> class Vec4;
template <class A> class Vec3;
template <class A> class Vec2;

template <class A> class Vec4
{
public:

	// Data (public for speed)
	union {
		struct {
			A x, y, z, w;
		};
		A v[4];
	};

	// Constructors
	Vec4() : x(0), y(0), z(0), w(0) {};
	Vec4(A d) : x(d), y(d), z(d), w(d) {};
	Vec4(A _x, A _y, A _z, A _w) : x(_x), y(_y), z(_z), w(_w) {};
	Vec4(A a[4]) : x(a[0]), y(a[1]), z(a[2]), w(a[3]) {};

    // Operators
	Vec4<A> operator-() const
	{
		return Vec4<A>(-x,-y,-z,-w);
	}

	// Get contents as array
	const A *GetAsFloat() const
	{
		return v;
	}
};

//------------------------------------------------------------------
// Vec3
//------------------------------------------------------------------
template <class A> class Vec3
{

public:

	// Data (public for speed)
	A x,y,z;

	// Constructors
	Vec3() : x(0),y(0),z(0) {};
	Vec3(A d) : x(d),y(d),z(d) {};
	Vec3(A _x,A _y,A _z) : x(_x),y(_y),z(_z) {};
	Vec3(A a[3]) : x(a[0]),y(a[1]),z(a[2]) {};

	// Operators
	Vec3<A> operator-() const
	{
		return Vec3<A>(-x,-y,-z);
	}
	
	Vec3<A> operator+(const Vec3<A>& other) const
	{
		return Vec3<A>(x+other.x,y+other.y,z+other.z);
	}

	Vec3<A> operator-(const Vec3<A>& other) const
	{
		return Vec3<A>(x-other.x,y-other.y,z-other.z);
	}
	
	Vec3<A> operator*(const Vec3<A>& other) const
	{
		return Vec3<A>(x*other.x,y*other.y,z*other.z);
	}
	
	Vec3<A> operator/(const Vec3<A>& other) const
	{
		return Vec3<A>(x/other.x,y/other.y,z/other.z);
	}
	
	Vec3<A> operator*(A num) const
	{
		return Vec3<A>(x*num,y*num,z*num);
	}

	Vec3<A> operator/(A num) const
	{
		assert(num != 0);
		A inum=(A)1/num;
		return Vec3<A>(x*inum,y*inum,z*inum);
	}
	
	void operator+=(const Vec3<A>& other)
	{
		x+=other.x;
		y+=other.y;
		z+=other.z;
	}

	void operator-=(const Vec3<A>& other)
	{
		x-=other.x;
		y-=other.y;
		z-=other.z;
	}

	void operator*=(const Vec3<A>& other)
	{
		x*=other.x;
		y*=other.y;
		z*=other.z;
	}

	void operator/=(const Vec3<A>& other)
	{
		x/=other.x;
		y/=other.y;
		z/=other.z;
	}

	void operator*=(A num)
	{
		x*=num;
		y*=num;
		z*=num;
	}

	void operator/=(A num)
	{
		A inum=(A)1/num;
		x*=inum;
		y*=inum;
		z*=inum;
	}

	A GetLength() const
	{
		return A(sqrt(x*x+y*y+z*z));
	}

	A GetSquareLength() const
	{
		return x*x+y*y+z*z;
	}

	A GetDotWith(const Vec3<A>& other) const
	{
		return x*other.x+y*other.y+z*other.z;
	}

	Vec3<A> GetCrossWith(const Vec3<A>& other) const
	{
		return Vec3<A>(y*other.z-z*other.y,z*other.x-x*other.z,x*other.y-y*other.x);
	}

	Vec3<A> GetNormalized() const
	{
		A length = GetLength();
		
		//assert(length != 0);

		// hax.
		if(length == 0)
			return Vec3<A>(0, 0, 0);

		A ilen=(A)1/length;
		return Vec3<A>(x*ilen,y*ilen,z*ilen);
	}

	void Normalize()
	{
		A length = GetLength();
		//assert(length != 0);

		// haxhax.
		if(length == 0)
			return ;

		A ilen=(A)1/length;
		x*=ilen;
		y*=ilen;
		z*=ilen;
	}

	A GetRangeTo(const Vec3<A> &other) const
	{
		return A(sqrt((x-other.x)*(x-other.x)+(y-other.y)*(y-other.y)+(z-other.z)*(z-other.z)));
	}

	A GetSquareRangeTo(const Vec3<A> &other) const
	{
		return (x-other.x)*(x-other.x)+(y-other.y)*(y-other.y)+(z-other.z)*(z-other.z);
	}

	A GetAngleTo(const Vec3<A> &other) const
	{
		A f=GetDotWith(other)/(GetLength()*other.GetLength());
		return (A)acos(f);
	}
};



//------------------------------------------------------------------
// Vec2
//------------------------------------------------------------------
template <class A> class Vec2
{

public:

	// Data (public for speed)
	A x,y;

	// Constructors
	Vec2() : x(0),y(0) {};
	Vec2(A d) : x(d),y(d) {};
	Vec2(A _x,A _y) : x(_x),y(_y) {};
	Vec2(A a[2]) : x(a[0]),y(a[1]) {};

	// Operators
	Vec2<A> operator-() const
	{
		return Vec2<A>(-x,-y);
	}
	
	Vec2<A> operator+(const Vec2<A>& other) const
	{
		return Vec2<A>(x+other.x,y+other.y);
	}

	Vec2<A> operator-(const Vec2<A>& other) const
	{
		return Vec2<A>(x-other.x,y-other.y);
	}
	
	Vec2<A> operator*(const Vec2<A>& other) const
	{
		return Vec2<A>(x*other.x,y*other.y);
	}
	
	Vec2<A> operator/(const Vec2<A>& other) const
	{
		return Vec2<A>(x/other.x,y/other.y);
	}
	
	Vec2<A> operator*(A num) const
	{
		return Vec2<A>(x*num,y*num);
	}

	Vec2<A> operator/(A num) const
	{
		return Vec2<A>(x/num,y/num);
	}
	
	void operator+=(const Vec2<A>& other)
	{
		x+=other.x;
		y+=other.y;
	}

	void operator-=(const Vec2<A>& other)
	{
		x-=other.x;
		y-=other.y;
	}

	void operator*=(const Vec2<A>& other)
	{
		x*=other.x;
		y*=other.y;
	}

	void operator/=(const Vec2<A>& other)
	{
		x/=other.x;
		y/=other.y;
	}

	void operator*=(A num)
	{
		x*=num;
		y*=num;
	}

	void operator/=(A num)
	{
		x/=num;
		y/=num;
	}

	A GetLength() const
	{
		return sqrtf(x*x+y*y);
	}

	A GetSquareLength() const
	{
		return x*x+y*y;
	}

	A GetDotWith(const Vec2<A>& other) const
	{
		return x*other.x+y*other.y;
	}

	Vec2<A> GetNormalized() const
	{
		A ilen=(A)1/GetLength();
		return Vec2<A>(x*ilen,y*ilen);
	}

	void Normalize()
	{
		A ilen=(A)1/GetLength();
		x*=ilen;
		y*=ilen;
	}

	A GetRangeTo(const Vec2<A> &other) const
	{
		return sqrtf((x-other.x)*(x-other.x)+(y-other.y)*(y-other.y));
	}

	A GetSquareRangeTo(const Vec2<A> &other) const
	{
		return (x-other.x)*(x-other.x)+(y-other.y)*(y-other.y);
	}

	A GetAngleTo(const Vec2<A> &other) const
	{
		A f=GetDotWith(other)/(GetLength()*other.GetLength());
		return (A)acos(f);
	}

	// Calculates angle: (1,0)=0, angle increases counterclockwise.
	A CalculateAngle() const
	{
		if (fabs(x)<EPSILON)
		{
			if (y>=0) return (A)(PI/2);
				else return (A)(3*(PI/2));
		}
		else
		{
			if (x>0)
			{
				if (y>=0) return (A)(atan(y/x));
					else return (A)((2*PI)+atan(y/x));
			}
			else
			{
				return (A)(PI+atan(y/x));
			}
		}
	}

};

typedef Vec4<float> VC4;
typedef Vec3<float> VC3;
typedef Vec2<float> VC2;
typedef Vec3<int> VC3I;
typedef Vec2<int> VC2I;
