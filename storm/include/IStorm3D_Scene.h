// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------

// Common datatypes
#include "DatatypeDef.h"

// Storm3D includes 
#include "Storm3D_Common.h"
#include "Storm3D_Datatypes.h"
#include "IStorm3D_Scene.h"
#include "IStorm3D_Model.h"
#include "IStorm3D_Material.h"
#include "IStorm3D_Font.h"
#include "IStorm3D_Particle.h"
#include "IStorm3D_Terrain.h"

//------------------------------------------------------------------
// Interface class prototypes
//------------------------------------------------------------------
class IStorm3D_Scene;
class IStorm3D_Camera;
class IStorm3D_Line;
class IStorm3D_Spotlight;
class IStorm3D_StreamBuilder;
class IStorm3D;

template<class T> struct TFrustum;
typedef TFrustum<float> Frustum;

//------------------------------------------------------------------
// IStorm3D_Scene (interface)
//------------------------------------------------------------------
class ST3D_EXP_DLLAPI IStorm3D_Scene
{

public:

	virtual IStorm3D * getStorm()=0;

	// Scene's camera
	virtual IStorm3D_Camera *GetCamera()=0;

	// Scene's particlesystem
	virtual IStorm3D_ParticleSystem *GetParticleSystem()=0;

	// Rendering (returns polygon count)
	virtual int RenderScene(bool present = true) = 0;
	virtual void RenderSceneWithParams(bool flip,bool disable_hsr, bool update_time, bool render_mirrored, IStorm3D_Texture *target) = 0;
	virtual void RenderVideo(const char *fileName, IStorm3D_StreamBuilder *streamBuilder) = 0;

	// Rendering (to dynamic textures)
	// Face parameter is only used if texture is cubemap.
	// Cubefaces: 0=pX, 1=nX, 2=pY, 3=nY, 4=pZ, 5=nZ (p=positive, n=negative)
	virtual int RenderSceneToDynamicTexture(IStorm3D_Texture *target,int face=0)=0;
	virtual void RenderSceneToAllDynamicCubeTexturesInScene()=0;

	// Model add/remove
	virtual void AddModel(IStorm3D_Model *mod)=0;
	virtual void RemoveModel(IStorm3D_Model *mod)=0;
	virtual void EnableCulling(IStorm3D_Model *mod, bool enable) = 0;

	// Stop internal, time based updates (animation halts)
	virtual void SetPauseState(bool scene_paused) = 0;
	virtual void DrawBones(bool draw) = 0;

	// Terrain add/remove
	virtual void AddTerrain(IStorm3D_Terrain *ter)=0;
	virtual void RemoveTerrain(IStorm3D_Terrain *ter)=0;

	// Lines
	virtual void AddLine(IStorm3D_Line *line, bool depth_test) = 0;
	virtual void RemoveLine(IStorm3D_Line *line) = 0;

	// Background Model (v2.3 new)
	virtual void SetBackGround(IStorm3D_Model *mod)=0;
	virtual void RemoveBackGround()=0;

	// 2D-rendering (goes to render list, and it's rendered with RenderScene)
	virtual void Render3D_Picture(IStorm3D_Material *mat,VC3 position,VC2 size)=0;
	virtual void Render2D_Picture(IStorm3D_Material *mat,VC2 position,VC2 size,float alpha=1.f,float rotation=0.f, float x1=0.f, float y1=0.f, float x2=1.f, float y2=1.f, bool wrap=false)=0;
	virtual void Render2D_Picture(IStorm3D_Material *mat, struct VXFORMAT_2D *vertices, int numVertices, float alpha, bool wrap=false)=0;
	virtual void Render2D_Text(IStorm3D_Font *font,VC2 position,VC2 size,const char *text,float alpha=1.f,const COL &colorFactor = COL(1.f, 1.f, 1.f))=0;
	virtual void Render2D_Text(IStorm3D_Font *font,VC2 position,VC2 size,const wchar_t *text,float alpha=1.f,const COL &colorFactor = COL(1.f, 1.f, 1.f))=0;

	// Test collision (to each model in scene)
	virtual void RayTrace(const VC3 &position,const VC3 &direction_normalized,float ray_length,Storm3D_CollisionInfo &cinf, bool accurate = false)=0;
	virtual void SphereCollision(const VC3 &position,float radius,Storm3D_CollisionInfo &cinf, bool accurate = true)=0;
	virtual void GetEyeVectors(const VC2I &screen_position, Vector &position, Vector &direction) = 0;

	// Scene parameter set
	virtual void SetAmbientLight(const COL &color)=0;
	virtual void SetBackgroundColor(const COL &color)=0;
	virtual void SetFogParameters(bool fog_active,const COL &color,float fog_start_range,float fog_end_range)=0;
	virtual void SetAnisotropicFilteringLevel(int level=0)=0;	// 0=off, 1+ = on

	// Debug renderig (cleared after frame)
	virtual void AddTriangle(const VC3 &p1, const VC3 &p2, const VC3 &p3, const COL &color) = 0;
	virtual void AddLine(const VC3 &p1, const VC3 &p2, const COL &color) = 0;
	virtual void AddPoint(const VC3 &p1, const COL &color) = 0;

	virtual void setWorldFoldCenter(const VC3 &position) = 0;
	virtual void addWorldFoldAtPosition(const VC3 &position, const MAT &fold) = 0;
	virtual void changeWorldFoldAtPosition(const VC3 &position, const MAT &fold) = 0;
	virtual void resetWorldFold() = 0;

	// Iterators
	ICreate<IStorm3D_Model*> *ITModel;
	ICreate<IStorm3D_Terrain*> *ITTerrain;

	// Virtual destructor (delete with this in v3)
	virtual ~IStorm3D_Scene() {};
};



//------------------------------------------------------------------
// IStorm3D_Camera (interface)
//------------------------------------------------------------------
class ST3D_EXP_DLLAPI IStorm3D_Camera
{

public:

	// Set parameters
	virtual void SetPosition(const VC3 &v)=0;
	virtual void SetTarget(const VC3 &v)=0;
	virtual void SetUpVec(const VC3 &v)=0;
	virtual void SetFieldOfView(float fov_ang)=0;
	virtual void SetFieldOfViewFactor(float fov_factor)=0;
	virtual void SetVisibilityRange(float range)=0;
	virtual void SetZNear(float value) = 0;
	virtual void SetZNearDefault() = 0;

	// Get parameters
	virtual const VC3 &GetPosition() const=0;
	virtual const VC3 &GetTarget() const=0;
	virtual const VC3 &GetUpVec() const=0;
	virtual VC3 GetUpVecReal() const=0;	// Fixes upvector to 90 deg angle from direction
	virtual VC3 GetDirection() const=0;
	virtual float GetFieldOfView() const=0;
	virtual float GetVisibilityRange() const=0;
	virtual Frustum getFrustum() const = 0;

	// sets the weird "shear effect" factor... --jpk
	// set to zero to disable effect
	virtual void SetShearEffectFactor(float shearEffectFactor)=0;

	// Use camerahelper's values (copies values from camerahelper to camera)
	virtual void UseCameraHelperValues(IStorm3D_Helper_Camera *helper)=0;

	// Visibility testing
	virtual bool TestSphereVisibility(const VC3 &position,float radius)=0;
	virtual bool TestPointVisibility(const VC3 &position)=0;

	virtual void ForceOrthogonalProjection (bool force, float minx = 0.0f, float maxx = 0.0f, float miny = 0.0f, float maxy = 0.0f) = 0; // when force == false, disables forcing the matrix.

	virtual bool GetTransformedToScreen(const VC3 &source,VC3 &result,float &rhw,float &real_z) = 0;
	virtual void SetAspectRatio(float ratio) = 0;

	virtual void getRayVector(int x, int y, VC3 &dir, VC3 &origin, float near_z) = 0;

	virtual bool GetForcedOrthogonalProjectionEnabled() = 0;

	virtual void Apply() = 0;

	// Virtual destructor (do not use, for Storm3D only)
	virtual ~IStorm3D_Camera() {};
};


