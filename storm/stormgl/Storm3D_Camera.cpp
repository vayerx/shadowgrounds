// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#ifdef _MSC_VER
#include <windows.h>
#endif

#include <GL/glew.h>
#include "storm3d_camera.h"
#include "storm3d.h"
#include <c2_frustum.h>

#include "Storm3D_ShaderManager.h"
#include "../../util/Debug_MemoryManager.h"

// NOTE: apparently, this was previously 1.0f
// now that it is 0.3f, has shadowmap quality possibly decreased? (assuming that shadows use
// the default clip planes - has anyone checked this)? 

const float DEFAULT_ZNEAR = 0.3f;


//-----------------------------------------------------------------------------
// Protos
//-----------------------------------------------------------------------------
/*void SetFrustumMatrix(D3DMATRIX& mat,FLOAT fLeft,FLOAT fRight,FLOAT fTop,
	FLOAT fBottom,FLOAT fNearPlane,FLOAT fFarPlane);
void SetAdjustedProjectionMatrix(D3DMATRIX& mat,FLOAT fFOV,FLOAT fAspect,
	FLOAT fNearPlane,FLOAT fFarPlane,FLOAT fPixDx,FLOAT fPixDy,FLOAT fVPWidth, 
	FLOAT fVPHeight);*/


//! Set the camera position
/*!
	\param v camera position
*/
void Storm3D_Camera::SetPosition(const VC3 &v)
{
	position=v;
	visplane_update_needed=true;
}

//! Set the camera target
/*!
	\param v target location
*/
void Storm3D_Camera::SetTarget(const VC3 &v)
{
	target=v;
	visplane_update_needed=true;
}

//! Set the up vector
/*!
	\param v up vector
*/
void Storm3D_Camera::SetUpVec(const VC3 &v)
{
	upvec=v;
	visplane_update_needed=true;
}

//! Set the Field of View
/*!
	\param fov_ang FOV angle, in ?
*/
void Storm3D_Camera::SetFieldOfView(float fov_ang)
{
	fov=fov_ang;
	visplane_update_needed=true;
}

//! Set the Field of View factor
/*!
	\param fov_factor_ FOV factor
*/
void Storm3D_Camera::SetFieldOfViewFactor(float fov_factor_)
{
	fov_factor=fov_factor_;
	visplane_update_needed=true;
}

//! Set visibility range
/*!
	\param range visibility range
*/
void Storm3D_Camera::SetVisibilityRange(float range)
{
	if (range>10000000) range=10000000;
	vis_range=range;
	sq_vis_range=range*range;
	visplane_update_needed=true;
}

//! Set the Z near
/*!
	\param value
*/
void Storm3D_Camera::SetZNear(float value)
{
	znear = value;	
}

//! Set Z near to default
void Storm3D_Camera::SetZNearDefault()
{
	znear = DEFAULT_ZNEAR;
}

//! Get the field of view
/*!
	\return FOV
*/
float Storm3D_Camera::GetFieldOfView() const
{
	return fov;
}

//! Get visibility range
/*!
	\return visibility range
*/
float Storm3D_Camera::GetVisibilityRange() const
{
	return vis_range;
}

//! Get the view frustum
/*!
	\return frustum
*/
Frustum Storm3D_Camera::getFrustum() const
{
	if(!forcedOrthogonalProjection)
	{
		if(visplane_update_needed) 
			UpdateVisPlanes();

		Frustum result(pnormals, position, vis_range);
		return result;
	}
	else
	{
		Frustum ret;
		VC3 nor = target - position;
		nor.Normalize ();
		VC3 pn[5] = { nor, nor, nor, nor, nor };
		return Frustum(pn, position, 10000.0f);
	}
}

//! Constructor
Storm3D_Camera::Storm3D_Camera(Storm3D *s2, const VC3 &_position,
		const VC3 &_target, float _vis_range, float _fov, const VC3 &_upvec) :
	Storm3D2(s2),
	position(_position),
	target(_target),
	upvec(_upvec),
	fov(_fov),
	fov_factor(1.f),
	vis_range(_vis_range),
	sq_vis_range(vis_range*vis_range),
	visplane_update_needed(true),
	shearEffectFactor(0.0f),
	znear(DEFAULT_ZNEAR),
	timeMsec(0),
	forcedViewProjection(false),
	forcedOrthogonalProjection(false)
{
	Storm3D_SurfaceInfo ss=Storm3D2->GetScreenSize();
	aspect_ratio = (float)ss.width/(float)ss.height;
}

extern D3DXMATRIX clip_matrix;
extern D3DXMATRIX reflection_matrix;

//! Applies camera settings
void Storm3D_Camera::Apply()
{
	if(!forcedViewProjection)
	{
		// Calc direction
		VC3 dir=target-position;

		// Create View matrix
		D3DXMatrixLookAtLH(mv, position, target, upvec);

		matView = mv;

		float ff = this->timeMsec / 500.f;

		if(!forcedOrthogonalProjection)
		{
			if (shearEffectFactor > 0.001f)
			{
				D3DXMatrixPerspectiveFovLH(matProj,(fov * fov_factor) + sinf(ff * .93f) * .1f * shearEffectFactor,aspect_ratio,znear,vis_range);
			} else {
				D3DXMatrixPerspectiveFovLH(matProj,fov * fov_factor,aspect_ratio,znear,vis_range);
			}
 		}

		Storm3D_ShaderManager::GetSingleton()->SetProjectionMatrix(matProj);
		// Multiply matrices to get VP (view-projection) matrix
		if (shearEffectFactor > 0.001f)
		{
			D3DXMATRIX shearMat;
			shearMat._12 = sinf(ff) * .1f * shearEffectFactor;
			shearMat._21 = cosf(ff * 1.3f) * .1f * shearEffectFactor;
			vp = mv * shearMat * matProj;
		} else {
			vp = mv * matProj;
		}
	}

	if(forcedOrthogonalProjection)
	{
		matProj = matForcedOrtho;
	}


	Storm3D_ShaderManager::GetSingleton()->SetViewMatrix(mv);
	Storm3D_ShaderManager::GetSingleton()->SetProjectionMatrix(matProj);
	//Storm3D_ShaderManager::GetSingleton()->SetViewProjectionMatrix(vp, mv);

	VC4 p;
	p.x = position.x;
	p.y = position.y;
	p.z = position.z;
	Storm3D_ShaderManager::GetSingleton()->SetViewPosition(p);

	// HAXHAX
	if(forcedViewProjection && !forcedOrthogonalProjection)
	{
		D3DXMATRIX view;
		D3DXMatrixMultiply(view, reflection_matrix, matView);
		D3DXMATRIX proj;
		proj = matProj * clip_matrix;

		Storm3D_ShaderManager::GetSingleton()->SetProjectionMatrix(proj);
		Storm3D_ShaderManager::GetSingleton()->SetViewMatrix(view);
	}
}

//! Get the view matrix
/*!
	\return view matrix
*/
const D3DXMATRIX& Storm3D_Camera::GetViewMatrix()
{
	static D3DXMATRIX view;
	/* FIXME: reflection_matrix[2][2] is -1 when it should be 1 ?
	static D3DXMATRIX tempref = reflection_matrix;
	tempref._22 = 1.0f;
	D3DXMatrixMultiply(view, tempref, matView);
	*/
	D3DXMatrixMultiply(view, reflection_matrix, matView);
	return view;
}

//! Get the projection matrix
/*!
	\return view matrix
*/
const D3DXMATRIX& Storm3D_Camera::GetProjectionMatrix()
{
	static D3DXMATRIX proj;
	proj = matProj * clip_matrix;
	return proj;
}

//! Apply mirrored camera settings
void Storm3D_Camera::ApplyMirrored()
{
	// Create View matrix
    D3DXMatrixLookAtLH(mv, position, target, upvec);
	Storm3D_ShaderManager::GetSingleton()->SetViewMatrix(mv);

	// Calc aspect!
	Storm3D_SurfaceInfo ss=Storm3D2->GetScreenSize();
	float aspect=(float)ss.width/(float)ss.height;

	// Create Projection matrix
    D3DXMATRIX matProj;
	D3DXMatrixPerspectiveFovLH(matProj, fov, -aspect, znear, vis_range);
	Storm3D_ShaderManager::GetSingleton()->SetProjectionMatrix(matProj);

	// Multiply matrices to get VP (view-projection) matrix
	vp = mv * matProj;
}

//! Force view projection
/*!
	\param other
*/
void Storm3D_Camera::ForceViewProjection(const Storm3D_Camera *other)
{
	if(other)
	{
		vp = other->vp;
		mv = other->mv;
		matView = other->matView;
		matProj = other->matProj;

		forcedViewProjection = true;
	}
	else
		forcedViewProjection = false;
}

//! Update the visibility planes
void Storm3D_Camera::UpdateVisPlanes() const
{
	// Create view matrix (without translation)
	D3DXMATRIX matView;
	VC3 zp(0,0,0);
	VC3 temp = target - position;
	D3DXMatrixLookAtLH(matView, zp, temp, upvec);
	viewrot=MAT((float*)matView.m);
	viewrot=viewrot.GetWithoutTranslation();
	viewrot.Inverse();

	// Precalc stuff
	float hfov=fov*0.5f*fov_factor;
	float spy=vis_range*tanf(hfov);
	Storm3D_SurfaceInfo ss=Storm3D2->GetScreenSize();
	float spx=spy*((float)ss.width/(float)ss.height);

	// Calculate 4 points (to create faces)
	VC3 points[4];	
	points[0]=VC3(spx,spy,-vis_range);
	points[1]=VC3(spx,-spy,-vis_range);
	points[2]=VC3(-spx,spy,-vis_range);
	points[3]=VC3(-spx,-spy,-vis_range);

	// Transform points
	viewrot.TransformVector(points[0]);
	viewrot.TransformVector(points[1]);
	viewrot.TransformVector(points[2]);
	viewrot.TransformVector(points[3]);

	// Calculate normals from faces

	// 0: Camera vector (normalized)
	pnormals[0]=target-position;
	pnormals[0].Normalize();
	
	// 1: Side face +
	VC3 e1=points[0];
	VC3 e2=-points[1];
	pnormals[1].x=e1.y*e2.z-e1.z*e2.y;
	pnormals[1].y=e1.z*e2.x-e1.x*e2.z;
	pnormals[1].z=e1.x*e2.y-e1.y*e2.x;
	pnormals[1].Normalize();
	
	// 2: Side face -
	e1=points[2];
	e2=points[3];
	pnormals[2].x=e1.y*e2.z-e1.z*e2.y;
	pnormals[2].y=e1.z*e2.x-e1.x*e2.z;
	pnormals[2].z=e1.x*e2.y-e1.y*e2.x;
	pnormals[2].Normalize();
	
	// 3: Down face (+)
	e1=points[2];
	e2=-points[0];
	pnormals[3].x=e1.y*e2.z-e1.z*e2.y;
	pnormals[3].y=e1.z*e2.x-e1.x*e2.z;
	pnormals[3].z=e1.x*e2.y-e1.y*e2.x;
	pnormals[3].Normalize();
	
	// 4: Up face (-)
	e1=points[1];
	e2=-points[3];
	pnormals[4].x=e1.y*e2.z-e1.z*e2.y;
	pnormals[4].y=e1.z*e2.x-e1.x*e2.z;
	pnormals[4].z=e1.x*e2.y-e1.y*e2.x;
	pnormals[4].Normalize();
	
	// Clear update flag
	visplane_update_needed=false;
}

//! Test if given sphere is visible
/*!
	\param pointpos center of sphere
	\param radius radius of sphere
	\return true if visible
*/
bool Storm3D_Camera::TestSphereVisibility(const VC3 &pointpos,float radius)
{

	if(forcedOrthogonalProjection) // Todo: real check
		return true;

	// Update vis planes, if needed
	if (visplane_update_needed) UpdateVisPlanes();

	// Back/Front planes
	float er=VC3(pointpos.x-position.x,
		pointpos.y-position.y,pointpos.z-position.z).GetDotWith(pnormals[0]);
	if (er<-radius) return false;
	if (er>(vis_range+radius)) return false;
	
	// Side Planes
	if (VC3(pointpos.x-position.x,pointpos.y-position.y,
		pointpos.z-position.z).GetDotWith(pnormals[1])
		<-radius) return false;

	if (VC3(pointpos.x-position.x,pointpos.y-position.y,
		pointpos.z-position.z).GetDotWith(pnormals[2])
		<-radius) return false;

	// Up/Down planes
	if (VC3(pointpos.x-position.x,pointpos.y-position.y,
		pointpos.z-position.z).GetDotWith(pnormals[3])
		<-radius) return false;

	if (VC3(pointpos.x-position.x,pointpos.y-position.y,
		pointpos.z-position.z).GetDotWith(pnormals[4])
		<-radius) return false;


	// It's visible (at last)
	return true;
}

//! Test if given box is visible
/*!
	\param min
	\param max
	\return true if visible
*/
bool Storm3D_Camera::TestBoxVisibility(const VC3 &min, const VC3 &max)
{

	if(forcedOrthogonalProjection) // Todo: real check
		return true;

	VC3 size = max - min;
	VC3 vertices[8] = 
	{
		VC3(min.x, min.y, min.z),
		VC3(min.x + size.x, min.y, min.z),
		VC3(min.x + size.x, min.y + size.y, min.z),
		VC3(min.x + size.x, min.y + size.y, min.z + size.z),
		VC3(min.x + size.x, min.y, min.z + size.z),
		VC3(min.x, min.y + min.y, min.z),
		VC3(min.x, min.y + min.y, min.z + min.z),
		VC3(min.x, min.y, min.z + min.z),
	};

	float origoDistance = position.GetRangeTo(VC3());

	bool allInside = true;
	for(int i = 0; i < 5; ++i)
	{
        bool allOutside = true;

		if(i == 0)
		{
			for(int j = 0; j < 8; ++j)
			{
				float distance = pnormals[i].GetDotWith(vertices[j]) + origoDistance + vis_range;

				allOutside = allOutside && (distance >= 0);
				allInside = allInside && (distance < 0);

				if(!allOutside && !allInside)
					break;
			}
		}

		for(int j = 0; j < 8; ++j)
		{
			float distance = pnormals[i].GetDotWith(vertices[j]) + origoDistance;

			allOutside = allOutside && (distance < 0);
			allInside = allInside && (distance >= 0);

			if(!allOutside && !allInside)
				break;
		}

        if(allOutside)
            return false;
	}

	return true;
}

//! Test if point is visible
/*!
	\param pointpos point position
	\return true if visible
*/
bool Storm3D_Camera::TestPointVisibility(const VC3 &pointpos)
{
	if(forcedOrthogonalProjection) // Todo: real check
		return true;

	// Update vis planes, if needed
	if (visplane_update_needed) UpdateVisPlanes();

	// Back/Front planes
	float er=VC3(pointpos.x-position.x,
		pointpos.y-position.y,pointpos.z-position.z).GetDotWith(pnormals[0]);
	if (er<0) return false;
	if (er>vis_range) return false;
	
	// Side Planes
	if (VC3(pointpos.x-position.x,pointpos.y-position.y,
		pointpos.z-position.z).GetDotWith(pnormals[1])
		<0) return false;

	if (VC3(pointpos.x-position.x,pointpos.y-position.y,
		pointpos.z-position.z).GetDotWith(pnormals[2])
		<0) return false;

	// Up/Down planes
	if (VC3(pointpos.x-position.x,pointpos.y-position.y,
		pointpos.z-position.z).GetDotWith(pnormals[3])
		<0) return false;

	if (VC3(pointpos.x-position.x,pointpos.y-position.y,
		pointpos.z-position.z).GetDotWith(pnormals[4])
		<0) return false;

	// It's visible (at last)
	return true;
}

//! Test if a point is behind camera
/*!
	\param pointpos point position
	\return true if behind
*/
bool Storm3D_Camera::TestPointIsBehind(const VC3 &pointpos)
{
	// Update vis planes, if needed
	if (visplane_update_needed) UpdateVisPlanes();

	// Front plane
	float er=VC3(pointpos.x-position.x,
		pointpos.y-position.y,pointpos.z-position.z).GetDotWith(pnormals[0]);
	if (er<0) return true;
	
	// It's on front (of camera)
	return false;
}

//! Get camera position vector
/*!
	\return position vector
*/
const VC3 &Storm3D_Camera::GetPosition() const
{
	return position;
}

//! Get camera target vector
/*!
	\return target vector
*/
const VC3 &Storm3D_Camera::GetTarget() const
{
	return target;
}

//! Get camera up vector
/*!
	\return up vector
*/
const VC3 &Storm3D_Camera::GetUpVec() const
{
	return upvec;
}

//! Get camera real up vector
/*!
	\return real up vector
*/
VC3 Storm3D_Camera::GetUpVecReal() const
{
	// Fix up vector
	VC3 dir=target-position;
	VC3 upr=upvec.GetCrossWith(-dir);
	upr=upr.GetCrossWith(dir);
	if (fabs(upr.GetSquareLength()) < 0.001)
	{
		upr = VC3(0,1,0);
	} else {
		upr.Normalize();
	}
	return upr;
}

//! Get camera direction vector
/*!
	\return direction vector
*/
VC3 Storm3D_Camera::GetDirection() const
{
	return target-position;
}

//! Set camera values to those provided by helper
/*!
	\param helper helper
*/
void Storm3D_Camera::UseCameraHelperValues(IStorm3D_Helper_Camera *helper)
{
	position=helper->GetGlobalPosition();
	target=position+helper->GetGlobalDirection().GetNormalized();
	upvec=helper->GetGlobalUpVector();
	upvec.Normalize();
	visplane_update_needed=true;
}

//! Transform point to screen space (returns true if not behind)
/*!
	\param source
	\param result
	\param rhw
	\param real_z
	\return true if point is not behind
*/
bool Storm3D_Camera::GetTransformedToScreen(const VC3 &source,VC3 &result,float &rhw,float &real_z)
{
	if(visplane_update_needed) 
	{
		UpdateVisPlanes();
		Apply();
	}

    const float rx=source.x*vp[0]+source.y*vp[4]+source.z*vp[8]+vp[12];
    const float ry=source.x*vp[1]+source.y*vp[5]+source.z*vp[9]+vp[13];
    real_z=source.x*vp[2]+source.y*vp[6]+source.z*vp[10]+vp[14];
    rhw=source.x*vp[3]+source.y*vp[7]+source.z*vp[11]+vp[15];

	// Is the point behind the screen
	if(fabsf(rhw) < EPSILON)
		return false;

	// Calculate the point to perspective
	float irhw=1.0f/rhw;
	result.x=0.5f+(rx*irhw*0.5f);
	result.y=0.5f+(ry*irhw*0.5f); // FIXME: not quite right?
	result.z=real_z*irhw;

	if(real_z<1)
		return false;

	// Visible
	return true;
}

//! Set aspect ratio
/*!
	\param ratio aspect ratio
*/
void Storm3D_Camera::SetAspectRatio(float ratio)
{
	aspect_ratio = ratio;
}

//! Set shear effect factor
/*!
	\param shearEffectFactor shear effect factor
*/
void Storm3D_Camera::SetShearEffectFactor(float shearEffectFactor)
{
	this->shearEffectFactor = shearEffectFactor;
}

//! Set time
/*!
	\param timeMsec time in milliseconds
*/
void Storm3D_Camera::SetTime(unsigned long timeMsec)
{
	this->timeMsec = timeMsec;
}

//! Force orthogonal projection
/*!
	\param force true to force projection
	\param minX X-axis minimum
	\param maxX X-axis maximum
	\param minY X-axis minimum
	\param maxY Y-axis maximum
*/
void Storm3D_Camera::ForceOrthogonalProjection(bool force, float minX, float maxX, float minY, float maxY)
{
	if(force)
	{
		D3DXMatrixIdentity (matProj);
		//D3DXMatrixOrthoLH ( &matProj , width, height, 0.1f, 1000.0f );
		D3DXMatrixOrthoOffCenterLH(matProj, minX, maxX, minY, maxY, 0.1f, 1000.0f);
		matForcedOrtho = matProj;
		forcedOrthogonalProjection = true;
	}
	else
		forcedOrthogonalProjection = false;
	
}

//! Get ray vector
/*!
	\param x
	\param y
	\param dir
	\param origin
	\param near_z
*/
void Storm3D_Camera::getRayVector(int x, int y, VC3 &dir, VC3 &origin, float near_z)
{
	D3DXMATRIX pProjection;
	D3DXMATRIX pView;

	VC3 upvec = this->GetUpVec();

	VC3 position = this->GetPosition();
	VC3 target = this->GetTarget();
	D3DXMatrixLookAtLH(pView, position, target, upvec);

	float fov = this->GetFieldOfView();
	Storm3D_SurfaceInfo ss = Storm3D2->GetScreenSize();
	float aspect=(float) ss.width / (float) ss.height;
	float vis_range = this->GetVisibilityRange();

	VC3 pV;

	D3DXMatrixPerspectiveFovLH(pProjection, fov, aspect, 1.0f, vis_range);

	pV.x = ( ( ( 2.0f * (float)x * ss.width / 1024 ) / ss.width  ) - 1 ) / pProjection._11;
	pV.y = ( ( ( 2.0f * (float)y * ss.height / 768 ) / ss.height ) - 1 ) / pProjection._22;
	pV.z = 1.0f;

	D3DXMATRIX m;
	D3DXMatrixInverse(m, NULL, pView);

	VC3 vPickRayDir;
	VC3 vPickRayOrig;

	vPickRayDir.x  = pV.x * m._11 + pV.y * m._21 + pV.z * m._31;
	vPickRayDir.y  = pV.x * m._12 + pV.y * m._22 + pV.z * m._32;
	vPickRayDir.z  = pV.x * m._13 + pV.y * m._23 + pV.z * m._33;
	vPickRayDir.Normalize();
	vPickRayOrig.x = m._41;
	vPickRayOrig.y = m._42;
	vPickRayOrig.z = m._43;

	vPickRayOrig += vPickRayDir * near_z;

	dir.x = vPickRayDir.x;
	dir.y = vPickRayDir.y;
	dir.z = vPickRayDir.z;
	origin.x = vPickRayOrig.x;
	origin.y = vPickRayOrig.y;
	origin.z = vPickRayOrig.z;
}
