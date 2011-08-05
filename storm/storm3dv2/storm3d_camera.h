// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d_common_imp.h"
#include "IStorm3D_Scene.h"

template<class T>
struct TFrustum;
typedef TFrustum<float> Frustum;

//------------------------------------------------------------------
// Storm3D_Camera
//------------------------------------------------------------------
class Storm3D_Camera : public IStorm3D_Camera
{
	// Pointer to Storm3D interface
	Storm3D *Storm3D2;

	// View-projection matrices
	D3DXMATRIX mv;	// view
	D3DXMATRIX vp;	// view*projection

	D3DXMATRIX matProj;
	D3DXMATRIX matView;

	D3DXMATRIX matForcedOrtho;

	// View matrix without translation
	mutable MAT viewrot;

	// Basic properties
	VC3 position;
	VC3 target;
	VC3 upvec;
	float fov;
	float fov_factor;
	float vis_range;
	float sq_vis_range;

	// Visibility test stuff
	mutable VC3 pnormals[5];
	mutable bool visplane_update_needed;

	void UpdateVisPlanes() const;
	float aspect_ratio;

	float shearEffectFactor;
	float znear;

	unsigned long timeMsec;
	bool forcedViewProjection;

	bool forcedOrthogonalProjection;

public:

	const D3DXMATRIX& GetViewMatrix();
	const D3DXMATRIX& GetProjectionMatrix();
	
	void Apply();
	void ApplyMirrored();
	void ForceViewProjection(const Storm3D_Camera *other);
	void ForceOrthogonalProjection (bool force, float minX = 0.0f, float maxX = 0.0f, float minY = 0.0f, float maxY = 0.0f); // when force == false, disables forcing the matrix.

	//void ApplyAdjusted(const VC2 &siz);

	void SetPosition(const VC3 &v);
	void SetTarget(const VC3 &v);
	void SetUpVec(const VC3 &v);
	void SetFieldOfView(float fov_ang);
	void SetFieldOfViewFactor(float fov_factor);
	void SetVisibilityRange(float range);
	void SetZNear(float value);
	void SetZNearDefault();

	// sets the weird "shear effect" factor... --jpk
	// set to zero to disable effect
	void SetShearEffectFactor(float shearEffectFactor);

	void SetTime(unsigned long timeMsec);

	const VC3 &GetPosition() const;
	const VC3 &GetTarget() const;
	const VC3 &GetUpVec() const;
	VC3 GetUpVecReal() const;
	VC3 GetDirection() const;
	float GetFieldOfView() const;
	float GetVisibilityRange() const;
	Frustum getFrustum() const;

	// Use camerahelpers values (copies values from camerahelper to camera)
	void UseCameraHelperValues(IStorm3D_Helper_Camera *helper);

	// Get view matrix
	const D3DMATRIX &GetVP() {return vp;}
	const D3DMATRIX &GetV() {return mv;}
	const float *GetViewProjection4x4Matrix();
	const float *GetView4x4Matrix();

	// Visibility testing
	bool TestSphereVisibility(const VC3 &position,float radius);
	bool TestBoxVisibility(const VC3 &min, const VC3 &max);
	bool TestPointVisibility(const VC3 &position);
	bool TestPointIsBehind(const VC3 &pointpos);

	// Transform point to screen space (returns true if not behind)
	bool GetTransformedToScreen(const VC3 &source,VC3 &result,float &rhw,float &real_z);
	void SetAspectRatio(float ratio);

	void getRayVector(int x, int y, VC3 &dir, VC3 &origin, float near_z);

	Storm3D_Camera(Storm3D *Storm3D2, const VC3 &_position=VC3(1,0,0), const VC3 &_target=VC3(0,0,0), float _vis_range=1000, float _fov=PI/3, const VC3 &_upvec=VC3(0,1,0));

	inline bool GetForcedOrthogonalProjectionEnabled() { return forcedOrthogonalProjection; };

	friend class Storm3D_Scene;
	friend class Storm3D_Scene_LightHandler;
	friend class Storm3D_Scene_PicList_Picture3D;
	friend class Storm3D_Mesh;	// shadows
};



