// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once

#include <list>
#include <vector>
#include "storm3d_common_imp.h"
#include "IStorm3D_Scene.h"
#include "storm3d.h"
#include "storm3d_camera.h"
#include "storm3d_terrain_utils.h"

class Storm3D_Line;

//------------------------------------------------------------------
// Storm3D_Scene
//------------------------------------------------------------------
class Storm3D_Scene : public IStorm3D_Scene
{
	// Pointer to Storm3D interface
	Storm3D *Storm3D2;

	// "Models in scene" - set
	set<IStorm3D_Model*> models;

	// "Terrains in scene" - set
	set<IStorm3D_Terrain*> terrains;

	// "Pictures to render" - set
	// CHANGED: was set
	list<Storm3D_Scene_PicList*> piclist;

	// Background model
	Storm3D_Model *bg_model;

	// Scene's parameters
	COL ambient;
	COL bgcolor;
	COL fog_color;
	float fog_start;
	float fog_end;
	bool fog_active;
	int anisotropic_level;

	// Renderarray stuff (v3)
	int renderlist_size;
	int renderlistmir_size;
	int shadlist_size;
	PStorm3D_Model_Object *renderlist_obj;
	PStorm3D_Model_Object *renderlistmir_obj;
	float *renderlist_points;
	float *renderlistmir_points;

	// Time in seconds (0.0 at start)
	float time;	

	// Polygon counter (v3)
	int poly_counter;

	// State
	bool scene_paused;
	bool draw_bones;

	// Lines
	std::vector<Storm3D_Line *> depth_lines;
	std::vector<Storm3D_Line *> no_depth_lines;
	frozenbyte::storm::VertexShader basic_shader;

	struct Debug3
	{
		COL color;
		VC3 p1;
		VC3 p2;
		VC3 p3;
	};

	struct Debug2
	{
		COL color;
		VC3 p1;
		VC3 p2;
	};

	struct Debug1
	{
		COL color;
		VC3 p1;
	};

	std::vector<Debug3> debugTriangles;
	std::vector<Debug2> debugLines;
	std::vector<Debug1> debugPoints;

	// Scene's particlesystem
	Storm3D_ParticleSystem *particlesystem;

	void renderRealScene(bool flip, bool render_mirrored, Storm3D_Texture *target = NULL);
#ifdef NVPERFSDK
	int bottlenecks[9];
#endif
public:

	inline IStorm3D * getStorm() { return (IStorm3D*) Storm3D2; };

	// Polygon counter add (v3)
	void AddPolyCounter(int amount) {poly_counter+=amount;}

	// Get stuff (v3)
	float GetTime() {return time;}
	bool IsFogEnabled() {return fog_active;}

	// Used on rendering (particles/animation/videos)
	int time_dif;

	// Camera
	Storm3D_Camera camera;

	// Get camera/particlesystem
	IStorm3D_Camera *GetCamera();
	IStorm3D_ParticleSystem *GetParticleSystem();

	// Rendering (returns polygon count)
	int RenderScene(bool present);
	void RenderSceneWithParams(bool flip=true,bool disable_hsr=false, bool update_time=true, bool render_mirrored=false, IStorm3D_Texture *target = NULL);
	void RenderVideo(const char *fileName, IStorm3D_StreamBuilder *streamBuilder);

	// Rendering (to dynamic textures)
	// Face parameter is only used if texture is cubemap.
	// Cubefaces: 0=pX, 1=nX, 2=pY, 3=nY, 4=pZ, 5=nZ (p=positive, n=negative)
	int RenderSceneToDynamicTexture(IStorm3D_Texture *target,int face=0);
	void RenderSceneToAllDynamicCubeTexturesInScene();

	// Model add/remove
	void AddModel(IStorm3D_Model *mod);
	void RemoveModel(IStorm3D_Model *mod);
	void EnableCulling(IStorm3D_Model *mod, bool enable);

	// Stop internal updates (time based)
	void SetPauseState(bool scene_paused);
	void DrawBones(bool draw);

	// Terrain add/remove
	void AddTerrain(IStorm3D_Terrain *ter);
	void RemoveTerrain(IStorm3D_Terrain *ter);

	// Lines
	void AddLine(IStorm3D_Line *line, bool depth_test);
	void RemoveLine(IStorm3D_Line *line);

	// Background Model (v2.3 new)
	void SetBackGround(IStorm3D_Model *mod);
	void RemoveBackGround();

	// 2D-rendering (goes to render list, and it's rendered with RenderScene)
	void Render3D_Picture(IStorm3D_Material *mat,VC3 position,VC2 size);
	void Render2D_Picture(IStorm3D_Material *mat,VC2 position,VC2 size,float alpha,float rotation,float x1,float y1,float x2,float y2,bool wrap);
	void Render2D_Picture(IStorm3D_Material *mat, struct VXFORMAT_2D *vertices, int numVertices, float alpha, bool wrap);
	void Render2D_Text(IStorm3D_Font *font,VC2 position,VC2 size,const char *text,float alpha,const COL &colorFactor);
	void Render2D_Text(IStorm3D_Font *font,VC2 position,VC2 size,const wchar_t *text,float alpha,const COL &colorFactor);

	// Test collision (to each model in scene)
	void RayTrace(const VC3 &position,const VC3 &direction_normalized,float ray_length,Storm3D_CollisionInfo &rti, bool accurate);
	void SphereCollision(const VC3 &position,float radius,Storm3D_CollisionInfo &cinf, bool accurate);
	void GetEyeVectors(const VC2I &screen_position, Vector &position, Vector &direction);

	// Scene parameter set
	void SetAmbientLight(const COL &color);
	void SetBackgroundColor(const COL &color);
	void SetFogParameters(bool fog_active,const COL &color,float fog_start_range,float fog_end_range);
	void SetAnisotropicFilteringLevel(int level=0);	// 0=off, 1+ = on

	void AddTriangle(const VC3 &p1, const VC3 &p2, const VC3 &p3, const COL &color);
	void AddLine(const VC3 &p1, const VC3 &p2, const COL &color);
	void AddPoint(const VC3 &p1, const COL &color);

	// World folding --jpk
	// (does not do anything if no WORLD_FOLDING_ENABLED defined)	
	void setWorldFoldCenter(const VC3 &position);
	void addWorldFoldAtPosition(const VC3 &position, const MAT &fold);
	void changeWorldFoldAtPosition(const VC3 &position, const MAT &fold);
	void resetWorldFold();


	// Creation/delete
	Storm3D_Scene(Storm3D *Storm3D2);
	~Storm3D_Scene();

	friend class Storm3D_Mesh;
	friend class Storm3D_Material;		// fogging for shaders
	friend class Storm3D_Model_Object;
};
