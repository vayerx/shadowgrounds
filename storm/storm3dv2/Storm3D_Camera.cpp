// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d_camera.h"
#include "storm3d.h"
#include <d3dx9math.h>
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



//------------------------------------------------------------------
// Get view matrix
//------------------------------------------------------------------
const float *Storm3D_Camera::GetViewProjection4x4Matrix()
{
	Apply();
	return (const float*)vp;
}

const float *Storm3D_Camera::GetView4x4Matrix()
{
	Apply();
	return (const float *)mv;
}


//------------------------------------------------------------------
// Storm3D_Camera::SetPosition
//------------------------------------------------------------------
void Storm3D_Camera::SetPosition(const VC3 &v)
{
	position=v;
	visplane_update_needed=true;
}



//------------------------------------------------------------------
// Storm3D_Camera::SetTarget
//------------------------------------------------------------------
void Storm3D_Camera::SetTarget(const VC3 &v)
{
	target=v;
	visplane_update_needed=true;
}



//------------------------------------------------------------------
// Storm3D_Camera::SetUpVec
//------------------------------------------------------------------
void Storm3D_Camera::SetUpVec(const VC3 &v)
{
	upvec=v;
	visplane_update_needed=true;
}



//------------------------------------------------------------------
// Storm3D_Camera::SetFieldOfView
//------------------------------------------------------------------
void Storm3D_Camera::SetFieldOfView(float fov_ang)
{
	fov=fov_ang;
	visplane_update_needed=true;
}

void Storm3D_Camera::SetFieldOfViewFactor(float fov_factor_)
{
	fov_factor=fov_factor_;
	visplane_update_needed=true;
}


//------------------------------------------------------------------
// Storm3D_Camera::SetVisibilityRange
//------------------------------------------------------------------
void Storm3D_Camera::SetVisibilityRange(float range)
{
	if (range>10000000) range=10000000;
	vis_range=range;
	sq_vis_range=range*range;
	visplane_update_needed=true;
}

void Storm3D_Camera::SetZNear(float value)
{
	znear = value;	
}

void Storm3D_Camera::SetZNearDefault()
{
	znear = DEFAULT_ZNEAR;
}


//------------------------------------------------------------------
// Storm3D_Camera::GetFieldOfView
//------------------------------------------------------------------
float Storm3D_Camera::GetFieldOfView() const
{
	return fov;
}



//------------------------------------------------------------------
// Storm3D_Camera::GetVisibilityRange
//------------------------------------------------------------------
float Storm3D_Camera::GetVisibilityRange() const
{
	return vis_range;
}

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


//------------------------------------------------------------------
// Storm3D_Camera::Storm3D_Camera
//------------------------------------------------------------------
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

//------------------------------------------------------------------
// Storm3D_Camera::Apply
//------------------------------------------------------------------
void Storm3D_Camera::Apply()
{
	if(!forcedViewProjection)
	{
		// Calc direction
		VC3 dir=target-position;

		// Create View matrix
		D3DXMatrixLookAtLH(&mv,(D3DXVECTOR3*)&position,
			(D3DXVECTOR3*)&target,(D3DXVECTOR3*)&upvec);
		Storm3D2->GetD3DDevice()->SetTransform(D3DTS_VIEW,&mv);

		matView = mv;

		// Calc aspect!
		//Storm3D_SurfaceInfo ss=Storm3D2->GetScreenSize();
		//float aspect=(float)ss.width/(float)ss.height;
		//aspect=1;	//BETA!

		float ff = this->timeMsec / 500.f;
	  //static DWORD time = timeGetTime();
		//int newTime = timeGetTime();
		//int delta = time - newTime();
		//time = newTime;

		// Create Projection matrix
		//D3DXMATRIX matProj;

		if(!forcedOrthogonalProjection)
		{
			if (shearEffectFactor > 0.001f)
			{
				D3DXMatrixPerspectiveFovLH(&matProj,(fov * fov_factor) + sinf(ff * .93f) * .1f * shearEffectFactor,aspect_ratio,znear,vis_range);
			} else {
				D3DXMatrixPerspectiveFovLH(&matProj,fov * fov_factor,aspect_ratio,znear,vis_range);
			}
 		}


		Storm3D2->GetD3DDevice()->SetTransform(D3DTS_PROJECTION,&matProj);
		// Multiply matrices to get VP (view-projection) matrix
		if (shearEffectFactor > 0.001f)
		{
			D3DXMATRIX shearMat;
			D3DXMatrixIdentity(&shearMat);
			shearMat._12 = sinf(ff) * .1f * shearEffectFactor;
			shearMat._21 = cosf(ff * 1.3f) * .1f * shearEffectFactor;
			vp=mv * shearMat * matProj;
			//vp= shearMat * mv * matProj;
		} else {
			vp=mv*matProj;
		}
	}

	if(forcedOrthogonalProjection)
	{
		matProj = matForcedOrtho;
	}

	Storm3D2->GetD3DDevice()->SetTransform(D3DTS_PROJECTION,&matProj);

//	Storm3D_ShaderManager::GetSingleton()->SetViewMatrix(mv);
//	Storm3D_ShaderManager::GetSingleton()->SetProjectionMatrix(matProj);
	Storm3D_ShaderManager::GetSingleton()->SetViewProjectionMatrix(vp, mv);

	D3DXVECTOR4 p;
	p.x = position.x;
	p.y = position.y;
	p.z = position.z;
	Storm3D_ShaderManager::GetSingleton()->SetViewPosition(p);

	// HAXHAX
	if(forcedViewProjection && !forcedOrthogonalProjection)
	{
		D3DXMATRIX view;
		D3DXMatrixMultiply(&view, &reflection_matrix, &matView);
		D3DXMATRIX proj;
		proj = matProj * clip_matrix;

		Storm3D2->GetD3DDevice()->SetTransform(D3DTS_PROJECTION,&proj);
		Storm3D2->GetD3DDevice()->SetTransform(D3DTS_VIEW,&view);
	}
}


const D3DXMATRIX& Storm3D_Camera::GetViewMatrix() 
{
	//return matView;

	/**/
	static D3DXMATRIX view;
	D3DXMatrixMultiply(&view, &reflection_matrix, &matView);
	return view;
	/**/
}
	

const D3DXMATRIX& Storm3D_Camera::GetProjectionMatrix() 
{
	//return matProj;

	/**/
	static D3DXMATRIX proj;
	proj = matProj * clip_matrix;
	return proj;
	/**/
}



//------------------------------------------------------------------
// Storm3D_Camera::ApplyAdjusted
//------------------------------------------------------------------
/*void Storm3D_Camera::ApplyAdjusted(const VC2 &siz)
{
	// Create View matrix
    D3DXMatrixLookAtLH(&mv,(D3DXVECTOR3*)&position,
		(D3DXVECTOR3*)&target,(D3DXVECTOR3*)&upvec);    
	Storm3D2->D3DDevice->SetTransform(D3DTS_VIEW,&mv);

	// Calc aspect!
	Storm3D_SurfaceInfo ss=Storm3D2->GetScreenSize();
	float aspect=(float)ss.width/(float)ss.height;
	//aspect=1;	//BETA!

	// Create Projection matrix
    D3DXMATRIX matProj;
    //D3DXMatrixPerspectiveFovLH(&matProj,fov,aspect,DEFAULT_ZNEAR,vis_range);
	SetAdjustedProjectionMatrix(matProj,fov,aspect,1,vis_range,0,0,siz.x,siz.y);
	Storm3D2->D3DDevice->SetTransform(D3DTS_PROJECTION,&matProj);

	// Multiply matrices to get VP (view-projection) matrix
	vp=mv*matProj;
}*/



//------------------------------------------------------------------
// Storm3D_Camera::ApplyMirrored
//------------------------------------------------------------------
void Storm3D_Camera::ApplyMirrored()
{
	// Create View matrix
    D3DXMatrixLookAtLH(&mv,(D3DXVECTOR3*)&position,
		(D3DXVECTOR3*)&target,(D3DXVECTOR3*)&upvec);    
	Storm3D2->GetD3DDevice()->SetTransform(D3DTS_VIEW,&mv);

	// Calc aspect!
	Storm3D_SurfaceInfo ss=Storm3D2->GetScreenSize();
	float aspect=(float)ss.width/(float)ss.height;

	// Create Projection matrix
    D3DXMATRIX matProj;
    //D3DXMatrixPerspectiveFovLH(&matProj,fov,-aspect,1.0f,vis_range);
    D3DXMatrixPerspectiveFovLH(&matProj,fov,-aspect,znear,vis_range);
	Storm3D2->GetD3DDevice()->SetTransform(D3DTS_PROJECTION,&matProj);

	// Multiply matrices to get VP (view-projection) matrix
	vp=mv*matProj;
}

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


//------------------------------------------------------------------
// Storm3D_Camera::UpdateVisPlanes
//------------------------------------------------------------------
void Storm3D_Camera::UpdateVisPlanes() const
{
	// Create view matrix (without translation)
	D3DXMATRIX matView;
	D3DXVECTOR3 zp=D3DXVECTOR3(0,0,0);
	VC3 temp = target - position;
	D3DXMatrixLookAtLH(&matView,&zp,
		(D3DXVECTOR3*)&temp,(D3DXVECTOR3*)&upvec);
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



//------------------------------------------------------------------
// Storm3D_Camera::TestSphereVisibility
//------------------------------------------------------------------
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


//------------------------------------------------------------------
// Storm3D_Camera::TestPointVisibility
//------------------------------------------------------------------
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



//------------------------------------------------------------------
// Storm3D_Camera::TestPointIsBehind
//------------------------------------------------------------------
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



//------------------------------------------------------------------
// Storm3D_Camera::GetPosition
//------------------------------------------------------------------
const VC3 &Storm3D_Camera::GetPosition() const
{
	return position;
}



//------------------------------------------------------------------
// Storm3D_Camera::GetTarget
//------------------------------------------------------------------
const VC3 &Storm3D_Camera::GetTarget() const
{
	return target;
}



//------------------------------------------------------------------
// Storm3D_Camera::GetUpVec
//------------------------------------------------------------------
const VC3 &Storm3D_Camera::GetUpVec() const
{
	return upvec;
}



//------------------------------------------------------------------
// Storm3D_Camera::GetUpVecReal
//------------------------------------------------------------------
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



//------------------------------------------------------------------
// Storm3D_Camera::GetDirection
//------------------------------------------------------------------
VC3 Storm3D_Camera::GetDirection() const
{
	return target-position;
}



//------------------------------------------------------------------
// Storm3D_Camera::GetDirection
//------------------------------------------------------------------
void Storm3D_Camera::UseCameraHelperValues(IStorm3D_Helper_Camera *helper)
{
	position=helper->GetGlobalPosition();
	target=position+helper->GetGlobalDirection().GetNormalized();
	upvec=helper->GetGlobalUpVector();
	upvec.Normalize();
	visplane_update_needed=true;
}



//-----------------------------------------------------------------------------
// Adjusted matrix stuff
//-----------------------------------------------------------------------------
/*void SetFrustumMatrix(D3DMATRIX& mat,FLOAT fLeft,FLOAT fRight,FLOAT fTop,
	FLOAT fBottom,FLOAT fNearPlane,FLOAT fFarPlane)
{
    float Q = fFarPlane / ( fFarPlane - fNearPlane );

    ZeroMemory( &mat, sizeof(D3DMATRIX) );
    mat._11 = ( 2.0f*fNearPlane )/( fRight - fLeft );
    mat._22 = ( 2.0f*fNearPlane )/( fTop - fBottom );
    mat._31 = ( fRight + fLeft )/ (fRight - fLeft );
    mat._32 = ( fTop + fBottom )/ (fTop - fBottom );
    mat._33 = Q;
    mat._34 = 1.0f;
    mat._43 = -Q*fNearPlane;
}


void SetAdjustedProjectionMatrix(D3DMATRIX& mat,FLOAT fFOV,FLOAT fAspect,
	FLOAT fNearPlane,FLOAT fFarPlane,FLOAT fPixDx,FLOAT fPixDy,FLOAT fVPWidth, 
	FLOAT fVPHeight)
{
    if(fabs(fFarPlane-fNearPlane)<0.01f) return;
    if(fabs(sin(fFOV/2))<0.01f) return;

    float h =   1.0f  * ( cosf(fFOV/2)/sinf(fFOV/2) );

    float fTop = fNearPlane/h;
    float fBottom = -fTop;
    float fRight = fTop * fAspect;
    float fLeft = -fRight;

    float fXWSize = fRight - fLeft;
    float fYWSize = fTop - fBottom;

    float fDx = -( fPixDx*fXWSize/fVPWidth );
    float fDy = -( fPixDy*fYWSize/fVPHeight );
    
    SetFrustumMatrix( mat, fLeft+fDx, fRight+fDx, fTop+fDy, fBottom+fDy, fNearPlane,
        fFarPlane );
}*/



//------------------------------------------------------------------
// Transform point to screen space (returns true if not behind)
//------------------------------------------------------------------
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
	//if ((real_z<1)||(fabsf(rhw)<EPSILON)) return false;
	if(fabsf(rhw) < EPSILON)
		return false;

	// Calculate the point to perspective
	float irhw=1.0f/rhw;
	result.x=0.5f+(rx*irhw*0.5f);
	result.y=0.5f-(ry*irhw*0.5f);
	result.z=real_z*irhw;

	if(real_z<1)
		return false;

	// Visible
	return true;
}

void Storm3D_Camera::SetAspectRatio(float ratio)
{
	aspect_ratio = ratio;
}


void Storm3D_Camera::SetShearEffectFactor(float shearEffectFactor)
{
	this->shearEffectFactor = shearEffectFactor;
}


void Storm3D_Camera::SetTime(unsigned long timeMsec)
{
	this->timeMsec = timeMsec;
}

void Storm3D_Camera::ForceOrthogonalProjection (bool force, float minX, float maxX, float minY, float maxY)
{
	if(force)
	{
		D3DXMatrixIdentity ( &matProj );
		//D3DXMatrixOrthoLH ( &matProj , width, height, 0.1f, 1000.0f );
		D3DXMatrixOrthoOffCenterLH ( &matProj, minX, maxX, minY, maxY, 0.1f, 1000.0f );
		matForcedOrtho = matProj;
		forcedOrthogonalProjection = true;
	}
	else
		forcedOrthogonalProjection = false;
	
}

void Storm3D_Camera::getRayVector(int x, int y, VC3 &dir, VC3 &origin, float near_z) {
	D3DXMATRIX pProjection;
	D3DXMATRIX pView;

	VC3 upvec = this->GetUpVec();

	VC3 position = this->GetPosition();
	VC3 target = this->GetTarget();
	D3DXMatrixLookAtLH(&pView, (D3DXVECTOR3 *) &position,
					   (D3DXVECTOR3 *) &target, (D3DXVECTOR3 *) &upvec);

	float fov = this->GetFieldOfView();
	Storm3D_SurfaceInfo ss = Storm3D2->GetScreenSize();
	float aspect=(float) ss.width / (float) ss.height;
	float vis_range = this->GetVisibilityRange();

	D3DXVECTOR3 pV;

	D3DXMatrixPerspectiveFovLH(&pProjection, fov, aspect, 1.0f, vis_range);

	pV.x =  ( ( ( 2.0f * (float)x * ss.width / 1024 ) / ss.width  ) - 1 ) / pProjection._11;
	pV.y = -( ( ( 2.0f * (float)y * ss.height / 768 ) / ss.height ) - 1 ) / pProjection._22;
	pV.z =  1.0f;

	D3DXMATRIX m;
	D3DXMatrixInverse(&m, NULL, &pView);

	D3DXVECTOR3 vPickRayDir;
	D3DXVECTOR3 vPickRayOrig;

	vPickRayDir.x  = pV.x * m._11 + pV.y * m._21 + pV.z * m._31;
	vPickRayDir.y  = pV.x * m._12 + pV.y * m._22 + pV.z * m._32;
	vPickRayDir.z  = pV.x * m._13 + pV.y * m._23 + pV.z * m._33;
	D3DXVec3Normalize(&vPickRayDir, &vPickRayDir);
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
