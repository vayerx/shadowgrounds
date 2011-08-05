// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once


//------------------------------------------------------------------
// Class Prototypes
//------------------------------------------------------------------
class Storm3D;
class Storm3D_Adapter;

class Storm3D_Scene;
class Storm3D_Scene_PicList;
class Storm3D_Scene_PicList_Picture;
class Storm3D_Scene_PicList_Picture3D;
class Storm3D_Scene_PicList_Font;
class Storm3D_Model;
class Storm3D_Model_Object;
class Storm3D_Mesh;
class Storm3D_Mesh_CollisionTable;

class Storm3D_Model_Object_Animation;
class Storm3D_Mesh_Animation;
class Storm3D_Light_Animation;
class Storm3D_Helper_Animation;

class Storm3D_Texture;
class Storm3D_Texture_Video;
class Storm3D_Material;
class Storm3D_Material_TextureLayer;

struct Storm3D_KeyFrame_Vector;
struct Storm3D_KeyFrame_Rotation;
struct Storm3D_KeyFrame_Scale;
struct Storm3D_KeyFrame_Mesh;
struct Storm3D_KeyFrame_Luminance;
struct Storm3D_KeyFrame_Cones;

class Storm3D_Helper_AInterface;
class Storm3D_Helper_Point;
class Storm3D_Helper_Vector;
class Storm3D_Helper_Camera;
class Storm3D_Helper_Box;
class Storm3D_Helper_Sphere;

class Storm3D_LensFlare;
class Storm3D_Font;
class Storm3D_Camera;

class Storm3D_ParticleSystem_PMH;
class Storm3D_ParticleSystem;
class Storm3D_RayTraceInfo;
class Storm3D_Terrain;

struct CollisionFace;
class TRBlock;



//------------------------------------------------------------------
// Typedefs
//------------------------------------------------------------------
typedef Storm3D_Model_Object *PStorm3D_Model_Object;
typedef Storm3D_Model *PStorm3D_Model;
typedef Storm3D_Material *PStorm3D_Material;
typedef Storm3D_Texture *PStorm3D_Texture;



//------------------------------------------------------------------
// Includes (etc)
//------------------------------------------------------------------

// DX8 stuff
#define D3D_OVERLOADS
#include <d3d9.h>
#include <d3dx9.h>



//------------------------------------------------------------------
// Defines etc.
//------------------------------------------------------------------
#define STORM3DV2_MAX_ACTIVELIGHTS		8
#define STORM3DV2_SHADOWMAP_SIZE		512

#define SAFE_RELEASE(p)      {if(p) {(p)->Release();(p)=NULL;}}

#define DOT(v1,v2) \
      (((v1.x)*(v2.x))+((v1.y)*(v2.y))+((v1.z)*(v2.z)))

#define CROSS(dest,v1,v2) \
          dest.x=v1.y*v2.z-v1.z*v2.y; \
          dest.y=v1.z*v2.x-v1.x*v2.z; \
          dest.z=v1.x*v2.y-v1.y*v2.x;

// DirectX com-object system needs this
inline DWORD F2DW(FLOAT f) {return *((DWORD*)&f);}

// Sets
#ifdef _MSC_VER
#pragma warning(disable:4786)	// For sets (Microsoft rulez;)
#pragma warning(disable:4103)
#endif
#include <set>
using namespace std;			// For sets

// "this used in constructor" warning disable
#ifdef _MSC_VER
#pragma warning(disable:4355)
#endif

