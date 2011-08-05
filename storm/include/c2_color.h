// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once


//------------------------------------------------------------------
// Prototypes and typedefs
//------------------------------------------------------------------
template <class A> class TColor;
typedef TColor<float> COL;


//------------------------------------------------------------------
// Color
//------------------------------------------------------------------
template <class A> class TColor
{

public:

	// Data (public for speed)
	A r,g,b;
	
	// Creation
	TColor(A _r=0,A _g=0,A _b=0) : r(_r),g(_g),b(_b) {}
	TColor(const A f[3]) : r(f[0]),g(f[1]),b(f[2]) {}

	// Functions (these do not modify this color)
	DWORD GetAsD3DCompatibleARGB() const
	{
		return ((DWORD)((((255)&0xff)<<24)|((((int)(b*255.0f))&0xff)<<16)|((((int)(g*255.0f))&0xff)<<8)|(((int)(r*255.0f))&0xff)));
	}

	TColor GetClamped() const			// Forces all colors in range [0,1]	
	{
		TColor nc(r,g,b);
		if (nc.r>1.0f) nc.r=1.0f; else if (nc.r<0.0f) nc.r=0.0f;
		if (nc.g>1.0f) nc.g=1.0f; else if (nc.g<0.0f) nc.g=0.0f;
		if (nc.b>1.0f) nc.b=1.0f; else if (nc.b<0.0f) nc.b=0.0f;
		return nc;
	}

	TColor GetClampedNegative() const	// Forces all colors in range [0,n[
	{
		TColor nc(r,g,b);
		if (nc.r<0.0f) nc.r=0.0f;
		if (nc.g<0.0f) nc.g=0.0f;
		if (nc.b<0.0f) nc.b=0.0f;
		return nc;
	}

	TColor GetClampedPositive() const	// Forces all colors in range ]-n,1]
	{
		TColor nc(r,g,b);
		if (nc.r>1.0f) nc.r=1.0f;
		if (nc.g>1.0f) nc.g=1.0f;
		if (nc.b>1.0f) nc.b=1.0f;
		return nc;
	}

	// Functions (these modify this color)
	void Clamp()			// Forces all colors in range [0,1]	
	{
		if (r>1.0f) r=1.0f; else if (r<0.0f) r=0.0f;
		if	(g>1.0f) g=1.0f; else if (g<0.0f) g=0.0f;
		if (b>1.0f) b=1.0f; else if (b<0.0f) b=0.0f;
	}

	void ClampNegative()	// Forces all colors in range [0,n[
	{
		if (r<0.0f) r=0.0f;
		if (g<0.0f) g=0.0f;
		if (b<0.0f) b=0.0f;
	}
	
	void ClampPositive()	// Forces all colors in range ]-n,1]
	{
		if (r>1.0f) r=1.0f;
		if (g>1.0f) g=1.0f;
		if (b>1.0f) b=1.0f;
	}

	// Operators
	TColor operator+(const TColor& other) const
	{
		return TColor(r+other.r,g+other.g,b+other.b);
	}

	TColor operator-(const TColor& other) const
	{
		return TColor(r-other.r,g-other.g,b-other.b);
	}

	TColor operator*(const TColor& other) const
	{
		return TColor(r*other.r,g*other.g,b*other.b);
	}
	
	TColor operator/(const TColor& other) const
	{
		return TColor(r/other.r,g/other.g,b/other.b);
	}

	TColor operator*(const A& num) const
	{
		return TColor(r*num,g*num,b*num);
	}
	
	TColor operator/(const A& num) const
	{
		return TColor(r/num,g/num,b/num);
	}

	void operator+=(const TColor& other)
	{
		r+=other.r;
		g+=other.g;
		b+=other.b;
	}

	void operator-=(const TColor& other)
	{
		r-=other.r;
		g-=other.g;
		b-=other.b;
	}

	void operator*=(const TColor& other)
	{
		r*=other.r;
		g*=other.g;
		b*=other.b;
	}

	void operator*=(const A& num)
	{
		r*=num;
		g*=num;
		b*=num;
	}

	void operator/=(const A& num)
	{
		r/=num;
		g/=num;
		b/=num;
	}

	bool operator==(const TColor& other) const
	{
		if ((fabsf(r-other.r)<EPSILON)&&
			(fabsf(g-other.g)<EPSILON)&&
			(fabsf(b-other.b)<EPSILON)) return true;
		return false;
	}

	bool operator!=(const TColor& other) const
	{
		if ((fabsf(r-other.r)<EPSILON)&&
			(fabsf(g-other.g)<EPSILON)&&
			(fabsf(b-other.b)<EPSILON)) return false;
		return true;
	}
};


