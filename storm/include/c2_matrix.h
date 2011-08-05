// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once


//------------------------------------------------------------------
// Prototypes and typedefs
//------------------------------------------------------------------
template <class A> class TMatrix;
typedef TMatrix<float> MAT;


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "c2_common.h"
#include "c2_vectors.h"
#include "c2_quat.h"
#include <cassert>


//------------------------------------------------------------------
// Matrix
//------------------------------------------------------------------
template <class A> class TMatrix
{
	A mat[16];

public:

	// Create...

	// Creates ID-matrix
	TMatrix()
	{ 
		// Leaving data uninitialized and not cheking those on GetTranslation() etc ..
		// That's BAD!
		//	-- psd

		mat[0] = 1;
		mat[1] = 0;
		mat[2] = 0;
		mat[3] = 0;

		mat[4] = 0;
		mat[5] = 1;
		mat[6] = 0;
		mat[7] = 0;
	
		mat[8] = 0;
		mat[9] = 0;
		mat[10] = 1;
		mat[11] = 0;
		
		mat[12] = 0;
		mat[13] = 0;
		mat[14] = 0;
		mat[15] = 1;
	}

	// Creates matrix from array
	TMatrix(const A *f16)
	{
		memcpy(mat,f16,sizeof(A)*16);
	}

	// Creates identity matrix
	void CreateIdentityMatrix()
	{
		mat[0] = 1;
		mat[1] = 0;
		mat[2] = 0;
		mat[3] = 0;

		mat[4] = 0;
		mat[5] = 1;
		mat[6] = 0;
		mat[7] = 0;
	
		mat[8] = 0;
		mat[9] = 0;
		mat[10] = 1;
		mat[11] = 0;
		
		mat[12] = 0;
		mat[13] = 0;
		mat[14] = 0;
		mat[15] = 1;
	}

	// Creates translation(move) matrix
	void CreateTranslationMatrix(const Vec3<A> &translation)
	{
		mat[1]=mat[2]=mat[3]=0;
		mat[4]=mat[6]=mat[7]=0;
		mat[8]=mat[9]=mat[11]=0;
		mat[0]=mat[5]=mat[10]=mat[15]=(A)1;

		mat[12]=translation.x;
		mat[13]=translation.y;
		mat[14]=translation.z;
	}

	// Creates scale matrix
	void CreateScaleMatrix(const Vec3<A> &scale)
	{
		mat[1]=mat[2]=mat[3]=0.0f;
		mat[4]=mat[6]=mat[7]=0.0f;
		mat[8]=mat[9]=mat[11]=0.0f;
		mat[12]=mat[13]=mat[14]=0.0f;
		mat[15]=1.0f;
		
		mat[0]=scale.x;
		mat[5]=scale.y;
		mat[10]=scale.z;
	}

	// Creates rotation matrix
	void CreateRotationMatrix(const Quat<A> &rotation)
	{
		// Convert the quaternion to matrix
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
    
		mat[0]=(A)1-yy2-zz2;
		mat[1]=xy2-wz2;
		mat[2]=xz2+wy2;

		mat[4]=xy2+wz2;
		mat[5]=(A)1-xx2-zz2;
		mat[6]=yz2-wx2;

		mat[8]=xz2-wy2;
		mat[9]=yz2+wx2;
		mat[10]=(A)1-xx2-yy2;

		mat[3]=mat[7]=mat[11]=0;
		mat[12]=mat[13]=mat[14]=0;
		mat[15]=(A)1;
	}

	// Creates camera matrix
	void CreateCameraMatrix(const Vec3<A> &position,const Vec3<A> &target,const Vec3<A> &up)
	{
		// Calculate direction (Z) vector
		Vec3<A> z=target-position;
		
		// Normalize direction vector
		z.Normalize();

		// Calculate real up vector (Y)
		A pdu=up.GetDotWith(z);
		Vec3<A> y=up-z*pdu;

	    // Normalize the y vector
		y.Normalize();

		// Calculate x vector with cross product
		Vec3<A> x=y.GetCrossWith(z);

		// Build matrix (rotation part)
		mat[0]=x.x;	mat[1]=y.x;	mat[2]=z.x;
		mat[4]=x.y;	mat[5]=y.y;	mat[6]=z.y;
		mat[8]=x.z;	mat[9]=y.z;	mat[10]=z.z;

		// Build matrix (translation part)
		mat[12]=-position.GetDotWith(x);
		mat[13]=-position.GetDotWith(y);
		mat[14]=-position.GetDotWith(z);

		// Last
		mat[3]=0;
		mat[7]=0;
		mat[11]=0;
		mat[15]=(A)1;
	}

	// Creates base change matrix
	void CreateBaseChangeMatrix(const Vec3<A> &base_x,const Vec3<A> &base_y,const Vec3<A> &base_z)
	{
		// Create the matrix
		mat[0]=base_x.x;
		mat[1]=base_y.x;
		mat[2]=base_z.x;
	
		mat[4]=base_x.y;
		mat[5]=base_y.y;
		mat[6]=base_z.y;
	
		mat[8]=base_x.z;
		mat[9]=base_y.z;
		mat[10]=base_z.z; 

		mat[3]=mat[7]=mat[11]=0;
		mat[12]=mat[13]=mat[14]=0;
		mat[15]=1;
	}

	// Functions (these do not modify this matrix)...

	TMatrix<A> GetInverse() const
	{
			A mat_inv[16] = { 0 };

			A fDetInv = (A)1 / ( mat[0] * ( mat[5] * mat[10] - mat[6] * mat[9] ) -
				                 mat[1] * ( mat[4] * mat[10] - mat[6] * mat[8] ) +
					             mat[2] * ( mat[4] * mat[9] - mat[5] * mat[8] ) );

			mat_inv[0] =  fDetInv * ( mat[5] * mat[10] - mat[6] * mat[9] );
			mat_inv[1] = -fDetInv * ( mat[1] * mat[10] - mat[2] * mat[9] );
			mat_inv[2] =  fDetInv * ( mat[1] * mat[6] - mat[2] * mat[5] );
			mat_inv[3] = 0;

			mat_inv[4] = -fDetInv * ( mat[4] * mat[10] - mat[6] * mat[8] );
			mat_inv[5] =  fDetInv * ( mat[0] * mat[10] - mat[2] * mat[8] );
			mat_inv[6] = -fDetInv * ( mat[0] * mat[6] - mat[2] * mat[4] );
			mat_inv[7] = 0;

			mat_inv[8] =  fDetInv * ( mat[4] * mat[9] - mat[5] * mat[8] );
			mat_inv[9] = -fDetInv * ( mat[0] * mat[9] - mat[1] * mat[8] );
			mat_inv[10] =  fDetInv * ( mat[0] * mat[5] - mat[1] * mat[4] );
			mat_inv[11] = 0;

		    mat_inv[12] = -( mat[12] * mat_inv[0] + mat[13] * mat_inv[4] + mat[14] * mat_inv[8] );
			mat_inv[13] = -( mat[12] * mat_inv[1] + mat[13] * mat_inv[5] + mat[14] * mat_inv[9] );
			mat_inv[14] = -( mat[12] * mat_inv[2] + mat[13] * mat_inv[6] + mat[14] * mat_inv[10] );
			mat_inv[15] = (A)1;

		return TMatrix<A>(mat_inv);
	}

	TMatrix<A> GetTranspose() const
	{
			A mat_t[16];

			// Transpose it
			mat_t[1]=mat[4];
			mat_t[2]=mat[8];
			mat_t[3]=mat[12];
			mat_t[6]=mat[9];
			mat_t[7]=mat[13];
			mat_t[11]=mat[14];

			mat_t[4]=mat[1];
			mat_t[8]=mat[2];
			mat_t[12]=mat[3];
			mat_t[9]=mat[6];
			mat_t[13]=mat[7];
			mat_t[14]=mat[11];

		return TMatrix<A>(mat_t);
	}

	TMatrix<A> GetWithoutTranslation() const
	{
			TMatrix m(mat);

			// Remove translation
			m.mat[12]=0;
			m.mat[13]=0;
			m.mat[14]=0;

			return m;
		}

	// Functions (these modify this matrix)...
	inline void Inverse()
	{
			A mat_inv[16] = { 0 };

			// Copy matrix to inverse
			memcpy(mat_inv,mat,sizeof(A)*16);

			// Make new matrix (inversed)
			A fDetInv = (A)1 / ( mat_inv[0] * ( mat_inv[5] * mat_inv[10] - mat_inv[6] * mat_inv[9] ) -
				                     mat_inv[1] * ( mat_inv[4] * mat_inv[10] - mat_inv[6] * mat_inv[8] ) +
					                 mat_inv[2] * ( mat_inv[4] * mat_inv[9] - mat_inv[5] * mat_inv[8] ) );

			mat[0] =  fDetInv * ( mat_inv[5] * mat_inv[10] - mat_inv[6] * mat_inv[9] );
			mat[1] = -fDetInv * ( mat_inv[1] * mat_inv[10] - mat_inv[2] * mat_inv[9] );
			mat[2] =  fDetInv * ( mat_inv[1] * mat_inv[6] - mat_inv[2] * mat_inv[5] );
			mat[3] = 0;

			mat[4] = -fDetInv * ( mat_inv[4] * mat_inv[10] - mat_inv[6] * mat_inv[8] );
			mat[5] =  fDetInv * ( mat_inv[0] * mat_inv[10] - mat_inv[2] * mat_inv[8] );
			mat[6] = -fDetInv * ( mat_inv[0] * mat_inv[6] - mat_inv[2] * mat_inv[4] );
			mat[7] = 0;

			mat[8] =  fDetInv * ( mat_inv[4] * mat_inv[9] - mat_inv[5] * mat_inv[8] );
			mat[9] = -fDetInv * ( mat_inv[0] * mat_inv[9] - mat_inv[1] * mat_inv[8] );
			mat[10] =  fDetInv * ( mat_inv[0] * mat_inv[5] - mat_inv[1] * mat_inv[4] );
			mat[11] = 0;

			mat[12] = -( mat_inv[12] * mat[0] + mat_inv[13] * mat[4] + mat_inv[14] * mat[8] );
			mat[13] = -( mat_inv[12] * mat[1] + mat_inv[13] * mat[5] + mat_inv[14] * mat[9] );
			mat[14] = -( mat_inv[12] * mat[2] + mat_inv[13] * mat[6] + mat_inv[14] * mat[10] );
			mat[15] = (A)1;
		}

	void Multiply(const TMatrix<A>& other)
	{
			A tmat[16];

			/*
			In a 4x4 matrix
			3  = 0
			7  = 0
			11 = 0
			15 = 1

			So let's optimize 28 multiplys off... 64 -> 36 multiplys. (78% more speed!)
			*/

			tmat[0] = mat[0] * other.mat[0] + mat[1] * other.mat[4] + mat[2] * other.mat[8];
			tmat[1] = mat[0] * other.mat[1] + mat[1] * other.mat[5] + mat[2] * other.mat[9];
			tmat[2] = mat[0] * other.mat[2] + mat[1] * other.mat[6] + mat[2] * other.mat[10];
			tmat[3] = 0;
                                                                          
			tmat[4] = mat[4] * other.mat[0] + mat[5] * other.mat[4] + mat[6] * other.mat[8];
			tmat[5] = mat[4] * other.mat[1] + mat[5] * other.mat[5] + mat[6] * other.mat[9];
			tmat[6] = mat[4] * other.mat[2] + mat[5] * other.mat[6] + mat[6] * other.mat[10];
			tmat[7] = 0;
	                                                     
			tmat[8] = mat[8] * other.mat[0] + mat[9] * other.mat[4] + mat[10] * other.mat[8];
			tmat[9] = mat[8] * other.mat[1] + mat[9] * other.mat[5] + mat[10] * other.mat[9];
			tmat[10] = mat[8] * other.mat[2] + mat[9] * other.mat[6] + mat[10] * other.mat[10];
			tmat[11] = 0;
                                                         
			tmat[12] = mat[12] * other.mat[0] + mat[13] * other.mat[4] + mat[14] * other.mat[8] + other.mat[12];
			tmat[13] = mat[12] * other.mat[1] + mat[13] * other.mat[5] + mat[14] * other.mat[9] + other.mat[13];
			tmat[14] = mat[12] * other.mat[2] + mat[13] * other.mat[6] + mat[14] * other.mat[10] + other.mat[14];
			tmat[15] = 1;

			// Set this matrix
			memcpy(mat,tmat,sizeof(A)*16);
	}


	// Operators...

	TMatrix<A> operator*(const TMatrix<A>& other) const
	{
			A tmat[16];

		    tmat[0] = mat[0] * other.mat[0] + mat[1] * other.mat[4] + mat[2] * other.mat[8];
			tmat[1] = mat[0] * other.mat[1] + mat[1] * other.mat[5] + mat[2] * other.mat[9];
			tmat[2] = mat[0] * other.mat[2] + mat[1] * other.mat[6] + mat[2] * other.mat[10];
			tmat[3] = 0;
		                                                                      
			tmat[4] = mat[4] * other.mat[0] + mat[5] * other.mat[4] + mat[6] * other.mat[8];
			tmat[5] = mat[4] * other.mat[1] + mat[5] * other.mat[5] + mat[6] * other.mat[9];
			tmat[6] = mat[4] * other.mat[2] + mat[5] * other.mat[6] + mat[6] * other.mat[10];
			tmat[7] = 0;
	                                                     
			tmat[8] = mat[8] * other.mat[0] + mat[9] * other.mat[4] + mat[10] * other.mat[8];
			tmat[9] = mat[8] * other.mat[1] + mat[9] * other.mat[5] + mat[10] * other.mat[9];
			tmat[10] = mat[8] * other.mat[2] + mat[9] * other.mat[6] + mat[10] * other.mat[10];
			tmat[11] = 0;
                                                         
			tmat[12] = mat[12] * other.mat[0] + mat[13] * other.mat[4] + mat[14] * other.mat[8] + other.mat[12];
			tmat[13] = mat[12] * other.mat[1] + mat[13] * other.mat[5] + mat[14] * other.mat[9] + other.mat[13];
			tmat[14] = mat[12] * other.mat[2] + mat[13] * other.mat[6] + mat[14] * other.mat[10] + other.mat[14];
			tmat[15] = (A)1;

			return TMatrix<A>(tmat);
		}

	// Vector modify...
	void TransformVector(Vec3<A> &vec) const
	{
			// Transform as 4x3
			A tmp_x=vec.x*mat[0]+vec.y*mat[4]+vec.z*mat[8]+mat[12];
			A tmp_y=vec.x*mat[1]+vec.y*mat[5]+vec.z*mat[9]+mat[13];
			A tmp_z=vec.x*mat[2]+vec.y*mat[6]+vec.z*mat[10]+mat[14];

			vec.x=tmp_x;
			vec.y=tmp_y;
			vec.z=tmp_z;
		}

	// psd
	void RotateVector(Vec3<A> &vec) const
	{
			// Transform as 4x3
			A tmp_x=vec.x*mat[0]+vec.y*mat[4]+vec.z*mat[8];
			A tmp_y=vec.x*mat[1]+vec.y*mat[5]+vec.z*mat[9];
			A tmp_z=vec.x*mat[2]+vec.y*mat[6]+vec.z*mat[10];

			vec.x=tmp_x;
			vec.y=tmp_y;
			vec.z=tmp_z;
		}

	Vec3<A> GetTransformedVector(const Vec3<A> &vec) const
	{
		Vec3<A> temp=vec;
		TransformVector(temp);
		return temp;
	}

	void GetAsD3DCompatible4x4(A *dest) const
	{
			for(int i = 0; i < 16; ++i)
				dest[i] = mat[i];
		}

	// Needed for exporters
	// -- psd
	A Get(int index) const
	{
		assert((index >= 0) && (index < 16));
		return mat[index];
	}
	
	void Set(int index, A value)
	{
		assert((index >= 0) && (index < 16));
		mat[index] = value;
	}

	Quat<A> GetRotation() const
	{
		Quat<A> result;

		// check the diagonal
		A tr = mat[0] + mat[5] + mat[10];
		if (tr > 0.0) 
		{
			A s = A(sqrt(tr + 1.0f));
			result.w = s / 2.0f;
			s = 0.5f / s;
			
			result.x = (mat[9] - mat[6]) * s;
			result.y = (mat[2] - mat[8]) * s;
			result.z = (mat[4] - mat[1]) * s;
		} 
		else 
		{
			// diagonal is negative
			A q[4] = { 0 };
			int nxt[3] = { 1, 2, 0 };
			int i = 0;
			
			if(mat[5] > mat[0])
				i = 1;
			if(mat[10] > mat[i*4 + i])
				i = 2;
			
			int j = nxt[i];
			int k = nxt[j];

			A s = A(sqrt((mat[i*4+i] - (mat[j*4+j] + mat[k*4+k])) + A(1.0)));
			q[i] = s * 0.5f;
            
			if (s != 0) 
				s = 0.5f / s;

			q[3] = (mat[k*4+j] - mat[j*4+k]) * s;
			q[j] = (mat[j*4+i] + mat[i*4+j]) * s;
			q[k] = (mat[k*4+i] + mat[i*4+k]) * s;

			result.x = q[0];
			result.y = q[1];
			result.z = q[2];
			result.w = q[3];
		}

		result.Normalize();
		return result;
	}

	Vec3<A> GetTranslation() const
	{
		return Vec3<A> (mat[12], mat[13], mat[14]);
	}

	Vec3<A> GetScale() const
	{
		A x = A(sqrt(mat[0]*mat[0] + mat[1]*mat[1] + mat[2]*mat[2]));
		A y = A(sqrt(mat[4]*mat[4] + mat[5]*mat[5] + mat[6]*mat[6]));
		A z = A(sqrt(mat[8]*mat[8] + mat[9]*mat[9] + mat[10]*mat[10]));

		return Vec3<A>(x,y,z);
	}

	const A *GetAsFloat() const
	{
		return mat;
	}

};
