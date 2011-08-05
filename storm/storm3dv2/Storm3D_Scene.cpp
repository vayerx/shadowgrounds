// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

#ifdef NVPERFSDK
#include "NVPerfSDK.h"
#endif

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d.h"
#include "storm3d_scene_piclist.h"
#include "storm3d_adapter.h"
#include "storm3d_material.h"
#include "storm3d_mesh.h"
#include "storm3d_light.h"
#include "storm3d_helper.h"
#include "storm3d_model_object.h"
#include "storm3d_model.h"
#include "storm3d_texture.h"
#include "storm3d_particle.h"
#include "Storm3D_structs.h"
#include "storm3d_scene.h"
#include "storm3d_terrain.h"
#include "storm3d_terrain_renderer.h"
#include "storm3d_terrain_models.h"
#include "storm3d_video_player.h"
#include <IStorm3D_Logger.h>
#include "Iterator.h"
#include "VertexFormats.h"
#include <boost/lexical_cast.hpp>
#include <d3dx9shape.h>
#include <d3dx9mesh.h>

#include "Storm3D_Bone.h"
#include "Storm3D_ShaderManager.h"
#include "Storm3D_Line.h"
#include "../../util/Debug_MemoryManager.h"

#ifdef WORLD_FOLDING_ENABLED
#include "WorldFold.h"
#define STORM_WORLD_FOLD_STEP 2.5f
#endif

extern int storm3d_dip_calls;

namespace {
	int pick_min(int a, int b)
	{
		return (a < b) ? a : b;
	}
}

//------------------------------------------------------------------
// Structs & defines etc.
//------------------------------------------------------------------
#define MAX_MIRRORS 500
Mirror mirrors[MAX_MIRRORS];



//------------------------------------------------------------------
// Storm3D_Scene::Storm3D_Scene
//------------------------------------------------------------------
Storm3D_Scene::Storm3D_Scene(Storm3D *s2) :
	Storm3D2(s2),
	bg_model(NULL),
	ambient(0,0,0),
	bgcolor(0,0,0),
	fog_active(false),
	anisotropic_level(0),
	renderlist_size(10),
	renderlistmir_size(10),
	time(0),
	scene_paused(false),
	draw_bones(false),
	basic_shader(*s2->GetD3DDevice()),
	camera(Storm3D2)
{
	basic_shader.createBasicBoneLightingShader();
	// Create iterators
	ITModel=new ICreateIM_Set<IStorm3D_Model*>(&(models));
	ITTerrain=new ICreateIM_Set<IStorm3D_Terrain*>(&(terrains));

	// Create particlesystem
	particlesystem=new Storm3D_ParticleSystem(Storm3D2);

	// Allocate renderarrays
	renderlist_obj=new PStorm3D_Model_Object[renderlist_size];
	renderlist_points=new float[renderlist_size];
	renderlistmir_obj=new PStorm3D_Model_Object[renderlistmir_size];
	renderlistmir_points=new float[renderlistmir_size];

#ifdef NVPERFSDK
	for( int i = 0; i < 9; i++ )
		bottlenecks[i] = 0;
#endif

#ifdef WORLD_FOLDING_ENABLED
	static bool storm_scene_inited_world_fold = false;
	if (!storm_scene_inited_world_fold)
	{
		storm_scene_inited_world_fold = true;
		WorldFold::initWorldFold(STORM_WORLD_FOLD_STEP);
	}
#endif
}



//------------------------------------------------------------------
// Storm3D_Scene::~Storm3D_Scene
//------------------------------------------------------------------
Storm3D_Scene::~Storm3D_Scene()
{
// can't do this - as there are multiple scenes.
//#ifdef WORLD_FOLDING_ENABLED
//	WorldFold::uninitWorldFold();
//#endif

	for(std::set<IStorm3D_Terrain *>::iterator it = terrains.begin(); it != terrains.end(); ++it)
	{
		Storm3D_Terrain *terrain = static_cast<Storm3D_Terrain *> (*it);
		
		for(std::set<IStorm3D_Model *>::iterator it = models.begin(); it != models.end(); ++it)
			terrain->getModels().removeModel(**it);
	}

	// Remove from Storm3D's list
	Storm3D2->Remove(this);

	// Delete renderarrays
	delete[] renderlist_obj;
	delete[] renderlist_points;
	//delete[] shadowrenderlist;

	// psd: these were leaks
	delete[] renderlistmir_obj;
	delete[] renderlistmir_points;

	// Delete iterators
	delete ITModel;
	delete ITTerrain;

	// Delete stuff
	delete particlesystem;
#ifdef NVPERFSDK
	char *bname = new char[50];
	for( int i = 1; i < 9; i++ )
	{
		NVPMGetGPUBottleneckName(i, bname);
		fprintf(stderr, "%i: %d \n", i, bottlenecks[i]);
	}
#endif
}



//------------------------------------------------------------------
// Storm3D_Scene::RenderSceneWithParams
//------------------------------------------------------------------
void Storm3D_Scene::RenderSceneWithParams(bool flip,bool disable_hsr, bool update_time, bool render_mirrored, IStorm3D_Texture *target)
{
	storm3d_dip_calls = 0;

	// Restore D3D-device if lost
	if (!Storm3D2->RestoreDeviceIfLost()) 
		return;

	// Calculate time difference
	static DWORD last_time=timeGetTime();
	DWORD time_now=timeGetTime();
	if (flip)
	{
		time_dif=time_now-last_time;
		// added use of timing factor...
		// -jpk
 		//last_time=time_now;
		if (this->Storm3D2->timeFactor != 1.0f)
		{
			// FIXME: may have a small error on some values
			// should work just fine for factor values like 0.5 though.
			time_dif = (int)(float(time_dif) * this->Storm3D2->timeFactor);
			last_time+=(int)(float(time_dif) / this->Storm3D2->timeFactor);
		} 
		else 
		{
			last_time+=time_dif;
		}
	}
	else
	{
		time_dif=time_now-last_time;
		//time_now=last_time;
		if (this->Storm3D2->timeFactor != 1.0f)
		{
			// FIXME: may have a small error on some values
			// should work just fine for factor values like 0.5 though.
			time_dif = (int)(float(time_dif) * this->Storm3D2->timeFactor);
			//last_time+=(int)(float(time_dif) / this->Storm3D2->timeFactor);
		} 
		else 
		{
			//last_time+=time_dif;
		}

		if(!update_time)
			time_dif = 0;
	}

	// Add time
	float ftime_dif=((float)time_dif)/1000.0f;
	time+=ftime_dif;

	// If paused
	if(scene_paused == true)
	{
		ftime_dif = 0.f;
		time_dif = 0;
		time_now = last_time;
	}

	this->camera.SetTime(time_now);

	// Reset active material and mesh
	Storm3D2->active_material=(Storm3D_Material*)1;	// NULL is not right!
	Storm3D2->active_mesh=NULL;

	// Basic renderstates
	Storm3D2->D3DDevice->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL);
//    Storm3D2->D3DDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);
    Storm3D2->D3DDevice->SetRenderState(D3DRS_ZENABLE,D3DZB_TRUE);
    Storm3D2->D3DDevice->SetRenderState(D3DRS_DITHERENABLE,TRUE);
	Storm3D2->D3DDevice->SetRenderState(D3DRS_NORMALIZENORMALS,FALSE);
	Storm3D2->D3DDevice->SetRenderState(D3DRS_LOCALVIEWER,TRUE);


	frozenbyte::storm::setCurrentAnisotrophy(anisotropic_level);
	frozenbyte::storm::applyMaxAnisotrophy(*Storm3D2->D3DDevice, Storm3D2->adapters[Storm3D2->active_adapter].multitex_layers);
	frozenbyte::storm::enableMinMagFiltering(*Storm3D2->D3DDevice, 0, Storm3D2->adapters[Storm3D2->active_adapter].multitex_layers, true);
	frozenbyte::storm::enableMipFiltering(*Storm3D2->D3DDevice, 0, Storm3D2->adapters[Storm3D2->active_adapter].multitex_layers, true);

	// Clear renderlists
	for (int i=0;i<renderlist_size;i++)
	{
		renderlist_obj[i]=NULL;
		renderlist_points[i]=-99999999.0f;
	}

	// Put each model in set into the list
	if(terrains.empty())
	for (set<IStorm3D_Model*>::iterator mit=models.begin();mit!=models.end();++mit)
	{
		// Typecast (to simplify code)
		Storm3D_Model *mod=(Storm3D_Model*)*mit;

		// new, external occlusion culling --jpk
		// umm.. this should be elsewhere?
		/*
		if (mod->GetOccluded())
		{
			continue;
		}
		*/

		// psd: Is the whole model visible (no animation/object stuff)
		// FIXME: only to boned objects
		if(!mod->bones.empty())
		{
			Vector &model_position = mod->position;

			// Calculate range (outside check part1... fastest)
			float radius = mod->bounding_radius;
			float nr = camera.vis_range + radius;
			
			if (fabsf(camera.position.x-model_position.x)>nr) continue;
			if (fabsf(camera.position.y-model_position.y)>nr) continue;
			if (fabsf(camera.position.z-model_position.z)>nr) continue;

			// Test sphere visibility
			if (!camera.TestSphereVisibility(model_position, radius)) 
				continue;
		}

		float range = camera.GetPosition().GetRangeTo(mod->GetPosition());
		mod->lodLevel = int(range / 20.f);
		if(mod->lodLevel >= IStorm3D_Mesh::LOD_AMOUNT)
			mod->lodLevel = IStorm3D_Mesh::LOD_AMOUNT - 1;

		//(!terrains.empty())
		//	continue;

		// Animate bone structure
		mod->AdvanceAnimation(time_dif);

		if (flip)
		{
			// Apply animation to helpers
			for(set<IStorm3D_Helper*>::iterator ih=mod->helpers.begin();ih!=mod->helpers.end();++ih)
			{
				// Typecast (to simplify code)
				IStorm3D_Helper *hlp=(IStorm3D_Helper*)*ih;
				switch(hlp->GetHelperType())
				{
					case IStorm3D_Helper::HTYPE_POINT:
						((Storm3D_Helper_Point*)hlp)->animation.Apply(this);
						break;

					case IStorm3D_Helper::HTYPE_VECTOR:
						((Storm3D_Helper_Vector*)hlp)->animation.Apply(this);
						break;

					case IStorm3D_Helper::HTYPE_BOX:
						((Storm3D_Helper_Box*)hlp)->animation.Apply(this);
						break;

					case IStorm3D_Helper::HTYPE_CAMERA:
						((Storm3D_Helper_Camera*)hlp)->animation.Apply(this);
						break;

					case IStorm3D_Helper::HTYPE_SPHERE:
						((Storm3D_Helper_Sphere*)hlp)->animation.Apply(this);
						break;
				}
			}
		}

		// Put each object in set into the list
		for(set<IStorm3D_Model_Object*>::iterator io=mod->objects.begin();io!=mod->objects.end();++io)
		{	
			// Typecast (to simplify code)
			Storm3D_Model_Object *obj=(Storm3D_Model_Object*)*io;
			//if(obj->light_object)
			//	continue;

			// Skip if object does not have a mesh
			if (obj->mesh==NULL) continue;

			// If object has no_render skip it
			if (obj->no_render) continue;
			
					// Calculate object world position
					VC3 owp=obj->GetGlobalPosition();

					// Calculate range (outside check part1... fastest)
					float mrad=obj->mesh->GetRadius();
					float nr=camera.vis_range+mrad;
					if (fabsf(camera.position.x-owp.x)>nr) continue;
					if (fabsf(camera.position.y-owp.y)>nr) continue;
					if (fabsf(camera.position.z-owp.z)>nr) continue;

					// Test sphere visibility
					if (!camera.TestSphereVisibility(owp,mrad)) continue;

					// Calculate range to camera (LOD needs)
					// Check if it's outside camera's range
					//float range=sqrtf(sqrange);
					float range=camera.position.GetRangeTo(owp);
					if ((range-mrad)>camera.vis_range) continue;

					// Calculate object points
					float points=range;

					// Add points if alphablending is used:
					// alpha-object's are always rendered last
					bool alpha_on=false;
					if (obj->mesh->GetMaterial())
					{
						if (obj->mesh->GetMaterial()->GetAlphaType()!=Storm3D_Material::ATYPE_NONE) alpha_on=true;
					}
					if (alpha_on)
					{
						// Draw alpha's always last
						points-=999999;
					}
					else
					{
						// Inverse points if opaque (drawn from front to back)
						// Speeds up raster performance
						points=-points;
					}

					// Calculate list position (optimizE!)
					int lp = 0;
					for (;renderlist_points[lp]>points;lp++);	// OK!
					
					// Move end of list 1 position backwards
					for (int i=renderlist_size-1;i>lp;i--)
					{
						renderlist_points[i]=renderlist_points[i-1];
						renderlist_obj[i]=renderlist_obj[i-1];
					}
	
					// Put object into the list
					renderlist_points[lp]=points;
					renderlist_obj[lp]=obj;

					// Test if there is enough room in list (v3)
					if (renderlist_obj[renderlist_size-1])
					{
						// Allocate double size (v3)
						int new_renderlist_size=renderlist_size*2;
						PStorm3D_Model_Object *new_renderlist_obj=new PStorm3D_Model_Object[new_renderlist_size];
						float *new_renderlist_points=new float[new_renderlist_size];
						
						// Clear new renderlists
						for (int i=0;i<new_renderlist_size;i++)
						{
							new_renderlist_obj[i]=NULL;
							new_renderlist_points[i]=-99999999.0f;
						}

						// Copy data
						memcpy(new_renderlist_obj,renderlist_obj,sizeof(PStorm3D_Model_Object)*renderlist_size);
						memcpy(new_renderlist_points,renderlist_points,sizeof(float)*renderlist_size);

						// Delete old data
						delete[] renderlist_obj;
						delete[] renderlist_points;

						// Set values
						renderlist_size=new_renderlist_size;
						renderlist_obj=new_renderlist_obj;
						renderlist_points=new_renderlist_points;
					}
		}
	}

	CComPtr<IDirect3DSurface9> originalDepthBuffer;
	Storm3D2->D3DDevice->GetDepthStencilSurface(&originalDepthBuffer);


	// Start REAL scene rendering
#ifdef NVPERFSDK
	if (flip) {
		int nCount = 0;
		NVPMBeginExperiment(&nCount);
		if (nCount > 0) {
			fprintf(stderr, "begin experiment, %d cycles\n", nCount);
			for (int i = 0; i < nCount; i++ ) {
				NVPMBeginPass(i);
				renderRealScene(flip, render_mirrored);
				NVPMEndPass(i);
			}
			NVPMEndExperiment();

			UINT64 value = 0, cycles = 0;
			char *bname = new char[50];
			NVPMGetCounterValueByName("GPU Bottleneck", 0, &value, &cycles);
			NVPMGetGPUBottleneckName(value, bname);
			bottlenecks[value]++;
			fprintf(stderr, "GPU Bottleneck value: %lu cycles: %lu\n", value, cycles);
			fprintf(stderr, "Bottleneck : %s\n", bname);
			delete[] bname;
		} else {
			renderRealScene(flip, render_mirrored);
		}
	} else {
		renderRealScene(flip, render_mirrored);
	}
#else
	renderRealScene(flip, render_mirrored);
#endif


	// Present the scene (flip)
	if (flip)
		Storm3D2->D3DDevice->Present(NULL,NULL,NULL,NULL);
	else if(!render_mirrored)
		Storm3D2->D3DDevice->SetDepthStencilSurface(originalDepthBuffer);
}

void Storm3D_Scene::renderRealScene(bool flip, bool render_mirrored) {
	Storm3D2->D3DDevice->BeginScene();
	Storm3D2->D3DDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	if(!flip && !render_mirrored)
	{
		int clearFlag = D3DCLEAR_ZBUFFER;
		if(Storm3D2->support_stencil)
			clearFlag |= D3DCLEAR_STENCIL;

		Storm3D2->D3DDevice->SetDepthStencilSurface(Storm3D2->getDepthTarget());

		DWORD color = bgcolor.GetAsD3DCompatibleARGB() & 0x00FFFFFF;
		Storm3D2->D3DDevice->Clear(0,0,D3DCLEAR_TARGET | clearFlag, color, 1, 0);
	}
	else
	{
		Storm3D2->getProceduralManagerImp().update(time_dif);
		Storm3D2->D3DDevice->Clear(0,0,D3DCLEAR_TARGET,bgcolor.GetAsD3DCompatibleARGB(),1,0);
	}

	// Update terrain render targets
	{
		for(set<IStorm3D_Terrain*>::iterator itr=terrains.begin();itr!=terrains.end();++itr)
		{
			// Typecast (to simplify code)
			Storm3D_Terrain *terra=(Storm3D_Terrain*)*itr;
			Storm3D_TerrainRenderer &renderer = static_cast<Storm3D_TerrainRenderer &> (terra->getRenderer());

			// Render it!
			renderer.updateVisibility(*this, time_dif);
			renderer.renderTargets(*this);
		}
	}

	// Apply the camera
	camera.Apply();

	// Set ambient
    Storm3D2->D3DDevice->SetRenderState(D3DRS_AMBIENT,ambient.GetAsD3DCompatibleARGB());

	// psd: fog was here

	/*
	// Render background model!
	Storm3D_ShaderManager::GetSingleton()->BackgroundShader(Storm3D2->D3DDevice);
	if (bg_model)
	{
		// Set model to camera position
		bg_model->SetPosition(camera.position);
		bg_model->lodLevel = 0;

		// Render each object in background model
		for(set<IStorm3D_Model_Object*>::iterator io=bg_model->objects.begin();io!=bg_model->objects.end();io++)
		{	
			// Typecast (to simplify code)
			Storm3D_Model_Object *obj=(Storm3D_Model_Object*)*io;

			// If object has no_render skip it
			if (obj->no_render) continue;
			
			// If object has no mesh skip it
			if (obj->mesh==NULL) continue;

			// Calculate object world position
			VC3 owp=obj->GetGlobalPosition();

			// Calculate range
			float range=camera.position.GetRangeTo(owp);

			// Set correct shader
			Storm3D_ShaderManager::GetSingleton()->SetShader(Storm3D2->D3DDevice, obj);

			// Do not apply lights!
			// Render it
			obj->mesh->RenderToBackground(this,obj);
		}
	}
	*/

	// psd: switch to hw t&l if needed
	if(Storm3D_ShaderManager::GetSingleton()->SoftwareShaders())
		Storm3D2->D3DDevice->SetSoftwareVertexProcessing(FALSE);
/*
	// Set fog
	if (fog_active)
	{
		// Set values
		Storm3D2->D3DDevice->SetRenderState(D3DRS_FOGENABLE, TRUE);
		Storm3D2->D3DDevice->SetRenderState(D3DRS_FOGCOLOR,fog_color.GetAsD3DCompatibleARGB());
		Storm3D2->D3DDevice->SetRenderState(D3DRS_FOGSTART,*(DWORD*)(&fog_start));
		Storm3D2->D3DDevice->SetRenderState(D3DRS_FOGEND,*(DWORD*)(&fog_end));

		//if(Storm3D_ShaderManager::GetSingleton()->SoftwareShaders() == true)
		{
			// Set pixel or vertex fog (depending on hardware support)
			if (Storm3D2->adapters[Storm3D2->active_adapter].caps&Storm3D_Adapter::CAPS_PIXELFOG)
				Storm3D2->D3DDevice->SetRenderState(D3DRS_FOGTABLEMODE,D3DFOG_LINEAR);
			else 
				Storm3D2->D3DDevice->SetRenderState(D3DRS_FOGVERTEXMODE,D3DFOG_LINEAR);
		}
	}
	else Storm3D2->D3DDevice->SetRenderState(D3DRS_FOGENABLE,FALSE);
*/
	// Render terrains
	//if (flip) // Does not render to mirrors
	{
		for(set<IStorm3D_Terrain*>::iterator itr=terrains.begin();itr!=terrains.end();++itr)
		{
			// Typecast (to simplify code)
			Storm3D_Terrain *terra=(Storm3D_Terrain*)*itr;
			Storm3D_TerrainRenderer &renderer = static_cast<Storm3D_TerrainRenderer &> (terra->getRenderer());

			// Render it!
			renderer.renderBase(*this);
		}
	}
/*
	// Render background model!
	Storm3D_ShaderManager::GetSingleton()->BackgroundShader(Storm3D2->D3DDevice);
	Storm3D_ShaderManager::GetSingleton()->setNormalShaders();
	if (bg_model)
	{
		// Set model to camera position
		bg_model->SetPosition(camera.position);
		bg_model->lodLevel = 0;

		// Render each object in background model
		for(set<IStorm3D_Model_Object*>::iterator io=bg_model->objects.begin();io!=bg_model->objects.end();io++)
		{	
			// Typecast (to simplify code)
			Storm3D_Model_Object *obj=(Storm3D_Model_Object*)*io;

			// If object has no_render skip it
			if (obj->no_render) continue;
			
			// If object has no mesh skip it
			if (obj->mesh==NULL) continue;

			// Set correct shader
			Storm3D_ShaderManager::GetSingleton()->SetShader(Storm3D2->D3DDevice, obj);
			Storm3D_ShaderManager::GetSingleton()->BackgroundShader(Storm3D2->D3DDevice);
			Storm3D_Material *mat = (Storm3D_Material *) (obj->mesh->GetMaterial());

			mat->ApplyBaseTextureOnly();
			obj->mesh->ReBuild();
			obj->mesh->RenderBuffers(obj);
			AddPolyCounter(obj->mesh->GetFaceCount());

			// Do not apply lights!
			// Render it
			//obj->mesh->RenderToBackground(this,obj);
		}
	}
*/

	// psd: fix flickering (random stuff as textures)
	Storm3D2->active_material=0;

	// Set fog
	if (fog_active)
	{
		// psd: no pixel fog for shaders
		// psd: disable fog table
		Storm3D2->D3DDevice->SetRenderState(D3DRS_FOGCOLOR,fog_color.GetAsD3DCompatibleARGB());

		if(Storm3D_ShaderManager::GetSingleton()->SoftwareShaders() == false)
		{
			Storm3D2->D3DDevice->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
			Storm3D2->D3DDevice->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_NONE );  
		}
	}

	// psd: switch back to sw t&l if needed
	if(Storm3D_ShaderManager::GetSingleton()->SoftwareShaders())
		Storm3D2->D3DDevice->SetSoftwareVertexProcessing(TRUE);

	Storm3D_ShaderManager::GetSingleton()->ResetShader();
	Storm3D_ShaderManager::GetSingleton()->ClearCache();
	//Storm3D_ShaderManager::GetSingleton()->SetModelAmbient(ambient);
	Storm3D_ShaderManager::GetSingleton()->setLightingShaders();

	//Storm3D2->D3DDevice->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);

	// Render objects in list (to screen)
	for (int i=0;renderlist_obj[i];i++)
	{
		if(i == 0)
		{
			Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_TEXTURE);
			Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_COLORARG2,D3DTA_DIFFUSE);
			Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);
			Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
			Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_ALPHAARG2,D3DTA_DIFFUSE);
			Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_MODULATE);
			Storm3D2->D3DDevice->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_DISABLE);
			Storm3D2->D3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE,FALSE);
			Storm3D2->D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);

			//Storm3D_ShaderManager::GetSingleton()->SetForceAmbient(COL());
			this->camera.Apply();
		}

		// Set shader constants
		Storm3D_Material *m = static_cast<Storm3D_Material *> (renderlist_obj[i]->mesh->GetMaterial());
		Storm3D_Model *mod = renderlist_obj[i]->parent_model;

		//Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG2);
		//Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_SELECTARG2);

		if(m)
		{
			Storm3D_ShaderManager::GetSingleton()->SetObjectAmbient(m->GetSelfIllumination());
			Storm3D_ShaderManager::GetSingleton()->SetObjectDiffuse(m->GetColor());
			Storm3D_Texture *t = (Storm3D_Texture *) m->GetBaseTexture();
		
			if(t)
			{
				t->Apply(0);
				Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);
				Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_MODULATE);
			}
			else
			{
				Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_SELECTARG2);
				Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG2);
			}

			IDirect3DDevice9 &device = *Storm3D2->D3DDevice;
			int alphaType = m->GetAlphaType();
			if(alphaType == IStorm3D_Material::ATYPE_NONE)
			{
				device.SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
			}
			else
			{
				device.SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);

				if(alphaType == IStorm3D_Material::ATYPE_ADD)
				{
					device.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);				
					device.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
				}
				else if(alphaType == IStorm3D_Material::ATYPE_MUL)
				{
					device.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
					device.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
				}
				else if(alphaType == IStorm3D_Material::ATYPE_USE_TRANSPARENCY)
				{
					device.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
					device.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
				}
				else if(alphaType == IStorm3D_Material::ATYPE_USE_TEXTRANSPARENCY || renderlist_obj[i]->force_alpha > 0.0001f)
				{
					device.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
					device.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
				}
			}

			Storm3D_ShaderManager::GetSingleton()->SetTextureOffset(m->getScrollOffset1());

		}
		else
		{
			Storm3D_ShaderManager::GetSingleton()->SetObjectAmbient(Color(1.f,1.f,1.f));
			Storm3D_ShaderManager::GetSingleton()->SetObjectDiffuse(Color(1.f,1.f,1.f));
		}

		Storm3D_ShaderManager::GetSingleton()->SetModelAmbient(mod->self_illumination + ambient);
		//Storm3D_ShaderManager::GetSingleton()->SetLight(0, mod->light_position1, mod->light_color1, mod->light_range1);
		//Storm3D_ShaderManager::GetSingleton()->SetLight(1, mod->light_position2, mod->light_color2, mod->light_range2);
		
		// Horrible ...
		{
			/*
			for(set<IStorm3D_Terrain*>::iterator itr=terrains.begin();itr!=terrains.end();++itr)
			{
				Storm3D_Terrain *terrain = (Storm3D_Terrain *) *itr;

				for(int i = 0; i < LIGHT_MAX_AMOUNT; ++i)
				{
					int index = mod->light_index[i];
					if(index != -1)
					{
						Light l = terrain->getModels().getLight(index);
						Storm3D_ShaderManager::GetSingleton()->SetLight(i, l.position, l.color, l.radius);
					}
					else
					{
						Storm3D_ShaderManager::GetSingleton()->SetLight(i, VC3(), COL(), 1.f);
					}
				}
				//Storm3D_TerrainRenderer &renderer = static_cast<Storm3D_TerrainRenderer &> (terra->getRenderer());
			}
			*/

			Storm3D_ShaderManager::GetSingleton()->setLightingParameters(false, false, 1);

			if(mod->type_flag == 0)
				Storm3D_ShaderManager::GetSingleton()->SetLight(0, VC3(-2.5f, 5.f, -10.f), COL(0.03f, 0.03f, 0.03f), 20.f);
			else
				Storm3D_ShaderManager::GetSingleton()->SetLight(0, VC3( 2.5f, 5.f, -10.f), COL(0.03f, 0.03f, 0.03f), 20.f);

			for(int i = 1; i < LIGHT_MAX_AMOUNT; ++i)
				Storm3D_ShaderManager::GetSingleton()->SetLight(i, VC3(), COL(), 1.f);
		}

		Storm3D_ShaderManager::GetSingleton()->SetSun(VC3(), 0.f);

		// Set correct shader
		Storm3D_ShaderManager::GetSingleton()->SetShader(Storm3D2->D3DDevice, renderlist_obj[i]);
		basic_shader.apply();

		if(!renderlist_obj[i]->parent_model->bones.empty())
		{
			renderlist_obj[i]->mesh->ReBuild();
			renderlist_obj[i]->mesh->RenderBuffers(renderlist_obj[i]);
		}

		Storm3D2->D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
		// Render it
		//renderlist_obj[i]->mesh->Render(this,false,renderlist_obj[i]); // always has a mesh!
	}

	//Storm3D2->D3DDevice->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);

	Storm3D2->D3DDevice->SetTexture(0,0);
	Storm3D2->D3DDevice->SetVertexShader(0);

	// Set renderstates (for shadows/particles/sprites)
	Storm3D2->D3DDevice->SetRenderState(D3DRS_LIGHTING,FALSE);
	Storm3D2->D3DDevice->SetRenderState(D3DRS_SPECULARENABLE,FALSE);
	Storm3D2->D3DDevice->SetRenderState(D3DRS_FOGENABLE,FALSE);
	Storm3D2->D3DDevice->SetRenderState(D3DRS_NORMALIZENORMALS,FALSE);
	Storm3D2->D3DDevice->SetRenderState(D3DRS_FILLMODE,D3DFILL_SOLID);

	// Apply the original camera back!
	camera.Apply();


	// Render particles
	Storm3D2->D3DDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
	if(terrains.empty())
		particlesystem->Render(this);
	Storm3D2->D3DDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);


	// Set renderstates for sprite rendering
//	Storm3D2->D3DDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
	Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_TEXTURE);
	Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_COLORARG2,D3DTA_DIFFUSE);
	Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);
	Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
	Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_ALPHAARG2,D3DTA_DIFFUSE);
	Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_MODULATE);
	Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,0);
	Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_DISABLE);
	Storm3D2->D3DDevice->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_DISABLE);

	Storm3D2->D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
	Storm3D2->D3DDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
	Storm3D2->D3DDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);

	// Moved 2D rendering after line rendering...
	// -- jpk

	// Renderstate for lines
//	Storm3D2->D3DDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);
	Storm3D2->D3DDevice->SetRenderState(D3DRS_LIGHTING,TRUE);
	Storm3D2->D3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE,FALSE);
	Storm3D2->D3DDevice->SetRenderState(D3DRS_ZENABLE,TRUE);

	// don't like the z-buffer update with semi-transparent lines
	// nasty flickering when the lines are on a same plane
	// thus, no z-buffer update 
	// --jpk
	//Storm3D2->D3DDevice->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);
	Storm3D2->D3DDevice->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);

	// Textures off
	for (int i=0;i<3;i++)
	{
		Storm3D2->D3DDevice->SetTexture(i,NULL);
	}

	/* Render lines */
	{	
		Storm3D2->D3DDevice->SetRenderState(D3DRS_LIGHTING,FALSE);
		//Storm3D2->D3DDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
		frozenbyte::storm::setCulling(*Storm3D2->D3DDevice, D3DCULL_NONE);

		D3DXMATRIX dm;
		D3DXMatrixIdentity(&dm);
		Storm3D2->D3DDevice->SetTransform(D3DTS_WORLD,&dm);

		if(!depth_lines.empty())
		{
			for(unsigned int i = 0; i < depth_lines.size(); ++i)
				depth_lines[i]->Render(Storm3D2->D3DDevice);
		}

		if(!no_depth_lines.empty())
		{
			Storm3D2->D3DDevice->SetRenderState(D3DRS_ZENABLE,FALSE);

			for(unsigned int i = 0; i < no_depth_lines.size(); ++i)
				no_depth_lines[i]->Render(Storm3D2->D3DDevice);
			
			Storm3D2->D3DDevice->SetRenderState(D3DRS_ZENABLE,TRUE);
		}
		
		Storm3D2->D3DDevice->SetRenderState(D3DRS_LIGHTING,TRUE);
		//Storm3D2->D3DDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);
		frozenbyte::storm::setCulling(*Storm3D2->D3DDevice, D3DCULL_CCW);
	}

	// Debug rendering
	if(!debugTriangles.empty() || !debugLines.empty() || !debugPoints.empty())
	{
		D3DXMATRIX dm;
		D3DXMatrixIdentity(&dm);
		Storm3D2->D3DDevice->SetTransform(D3DTS_WORLD,&dm);

		int vertexAmount = (debugTriangles.size() * 3) + (debugLines.size() * 2) + (debugPoints.size());
		
		frozenbyte::storm::VertexBuffer vertexBuffer;
		vertexBuffer.create(*Storm3D2->D3DDevice, vertexAmount, sizeof(VXFORMAT_PSD), true);
		VXFORMAT_PSD *buffer = static_cast<VXFORMAT_PSD *> (vertexBuffer.lock());

		for(unsigned int i = 0; i < debugTriangles.size(); ++i)
		{
			const Debug3 &d = debugTriangles[i];
			DWORD color = d.color.GetAsD3DCompatibleARGB();
			
			buffer->color = color;
			buffer->position = d.p1;
			++buffer;
			buffer->color = color;
			buffer->position = d.p2;
			++buffer;
			buffer->color = color;
			buffer->position = d.p3;
			++buffer;
		}

		int lineOffset = debugTriangles.size() * 3;
		for(unsigned int i = 0; i < debugLines.size(); ++i)
		{
			const Debug2 &d = debugLines[i];
			DWORD color = d.color.GetAsD3DCompatibleARGB();
			
			buffer->color = color;
			buffer->position = d.p1;
			++buffer;
			buffer->color = color;
			buffer->position = d.p2;
			++buffer;
		}

		int pointOffset = lineOffset + (debugLines.size() * 2);
		for(unsigned int i = 0; i < debugPoints.size(); ++i)
		{
			const Debug1 &d = debugPoints[i];
			DWORD color = d.color.GetAsD3DCompatibleARGB();
			
			buffer->color = color;
			buffer->position = d.p1;
			++buffer;
		}

		vertexBuffer.unlock();
		vertexBuffer.apply(*Storm3D2->D3DDevice, 0);

		Storm3D2->D3DDevice->SetVertexShader(0);
		Storm3D2->D3DDevice->SetFVF(FVF_VXFORMAT_PSD);

		Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_TEXTURE);
		Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_COLORARG2,D3DTA_DIFFUSE);
		Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_SELECTARG2);
		Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
		Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_ALPHAARG2,D3DTA_DIFFUSE);
		Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG2);
		Storm3D2->D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
		Storm3D2->D3DDevice->SetRenderState(D3DRS_LIGHTING,FALSE);

		if(!debugTriangles.empty())
			Storm3D2->D3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, debugTriangles.size());
		if(!debugLines.empty())
			Storm3D2->D3DDevice->DrawPrimitive(D3DPT_LINELIST, lineOffset, debugLines.size());
		if(!debugPoints.empty())
			Storm3D2->D3DDevice->DrawPrimitive(D3DPT_POINTLIST, pointOffset, debugPoints.size());

		Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_TEXTURE);
		Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_COLORARG2,D3DTA_DIFFUSE);
		Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);
		Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
		Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_ALPHAARG2,D3DTA_DIFFUSE);
		Storm3D2->D3DDevice->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_MODULATE);
		Storm3D2->D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
		Storm3D2->D3DDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	}

	// Render bones. SLOOOOOOOOW!
	
	// Ugly hack anyway
	//		-- psd
	if(draw_bones)
	{
		D3DXMATRIX dm;
		D3DXMatrixIdentity(&dm);
		Storm3D2->D3DDevice->SetTransform(D3DTS_WORLD,&dm);

		for(set<IStorm3D_Model *>::iterator mit=models.begin();mit!=models.end();++mit)
		{
			// Typecast (to simplify code)
			Storm3D_Model *mod=(Storm3D_Model*)*mit;

			if(mod->bones.size() > 0)
			{
				Storm3D2->D3DDevice->SetRenderState(D3DRS_LIGHTING,FALSE);
				Storm3D2->D3DDevice->SetRenderState(D3DRS_ZENABLE,FALSE);

				if(true)
				{
					VXFORMAT_PSD line[2];
					line[0].color = D3DCOLOR_RGBA(255,255,128,255);
					line[1].color = D3DCOLOR_RGBA(255,255,128,255);

					IDirect3DVertexBuffer9 *vbuffer = 0;
					if(Storm3D_ShaderManager::GetSingleton()->SoftwareShaders() == true)
						Storm3D2->D3DDevice->CreateVertexBuffer( 3*sizeof(VXFORMAT_PSD), D3DUSAGE_SOFTWAREPROCESSING| D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC, FVF_VXFORMAT_PSD, D3DPOOL_DEFAULT, &vbuffer, 0);
					else
						Storm3D2->D3DDevice->CreateVertexBuffer( 3*sizeof(VXFORMAT_PSD), D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC, FVF_VXFORMAT_PSD, D3DPOOL_DEFAULT, &vbuffer, 0);

					Vector bone_position;
					Vector bone_offset = Vector(0,0,1.f);

					for(unsigned int i = 0; i < mod->bones.size(); ++i)
					{
						Storm3D_Bone *b = mod->bones[i];
						
						// Get D3D matrix and grab position from there
						Matrix global_tm = b->GetTM();
						float tm[16] = { 0 };
						global_tm.GetAsD3DCompatible4x4(tm);
						
						float thickness = b->GetThickness();
						thickness = 0.01f;
						bone_position.x = tm[12] - (thickness * tm[8]);
						bone_position.y = tm[13] - (thickness * tm[9]);
						bone_position.z = tm[14] - (thickness * tm[10]);

						bone_offset.z = b->GetLenght() + (2.f * thickness);

						line[0].position = bone_position;
						line[1].position = global_tm.GetTransformedVector(bone_offset);

						{
							// Fill buffer
							BYTE *vp;
							vbuffer->Lock(0,0,(void**)&vp,D3DLOCK_DISCARD);

							VXFORMAT_PSD *p=(VXFORMAT_PSD*)vp;
							for(int i=0;i<2;i++)
								p[i]=line[i];
							
							vbuffer->Unlock();

							// Render
							Storm3D2->D3DDevice->SetStreamSource(0, vbuffer, 0, sizeof(VXFORMAT_PSD));
							Storm3D2->D3DDevice->SetVertexShader(0);
							Storm3D2->D3DDevice->SetFVF( FVF_VXFORMAT_PSD );
							Storm3D2->D3DDevice->DrawPrimitive( D3DPT_LINELIST, 0, 1 );
						}
					}

					vbuffer->Release();
					Storm3D2->D3DDevice->SetRenderState(D3DRS_LIGHTING,TRUE);
					Storm3D2->D3DDevice->SetRenderState(D3DRS_ZENABLE,TRUE);
				}
				else
				{	
					Storm3D2->D3DDevice->SetFVF(D3DFVF_XYZ|D3DFVF_NORMAL);
					Storm3D2->D3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

					for(unsigned int i = 0; i < mod->bones.size(); ++i)
					{
						Storm3D_Bone *b = mod->bones[i];
						float thickness = b->GetThickness();

						CComPtr<ID3DXMesh> mesh;
						D3DXCreateCylinder(Storm3D2->D3DDevice, thickness, thickness, b->GetLenght() + 2*thickness, 4, 4, &mesh, 0);
						//D3DXCreateCylinder(Storm3D2->D3DDevice, thickness, thickness, b->GetLenght(), 6, 6, &mesh, 0);
						//thickness = 0.f;

						Matrix global_tm = b->GetTM();
						D3DMATRIX tm;
						global_tm.GetAsD3DCompatible4x4(&tm.m[0][0]);
						tm._41 += (-thickness + (0.5f * b->GetLenght())) * tm._31;
						tm._42 += (-thickness + (0.5f * b->GetLenght())) * tm._32;
						tm._43 += (-thickness + (0.5f * b->GetLenght())) * tm._33;

						Storm3D2->D3DDevice->SetTransform(D3DTS_WORLD, &tm);
						mesh->DrawSubset(0);
					}

					Storm3D2->D3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
				}
			}
		}

		Storm3D2->D3DDevice->SetTransform(D3DTS_WORLD,&dm);
	}

	Storm3D2->D3DDevice->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
	Storm3D2->D3DDevice->SetRenderState(D3DRS_ZENABLE,FALSE);
	Storm3D2->D3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE,FALSE);

	Storm3D2->D3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	Storm3D2->D3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	frozenbyte::storm::enableMipFiltering(*Storm3D2->D3DDevice, 0, 0, false);

	// Render picturelist
	// CHANGED: was set
	if(!render_mirrored)
	{
		for (list<Storm3D_Scene_PicList*>::iterator ip=piclist.begin();ip!=piclist.end();++ip)
		{
			// Typecast to simplify code
			Storm3D_Scene_PicList *pl=*ip;

			// Render it
			pl->Render();

			// Delete it
			delete pl;
		}
	}

	Storm3D2->D3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	Storm3D2->D3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	frozenbyte::storm::enableMipFiltering(*Storm3D2->D3DDevice, 0, 0, true);

	// Clear picturelist
	if(!render_mirrored)
	{
		piclist.clear();
		debugTriangles.clear();
		debugLines.clear();
		debugPoints.clear();
	}

	Storm3D2->D3DDevice->SetRenderState(D3DRS_ZENABLE,TRUE);
	Storm3D2->D3DDevice->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);
	Storm3D2->D3DDevice->SetTexture(0,0);

	// End REAL scene rendering
	Storm3D2->D3DDevice->EndScene();
}


void Storm3D_Scene::RenderVideo(const char *fileName, IStorm3D_StreamBuilder *streamBuilder)
{
	IStorm3D_Scene *tempScene = Storm3D2->CreateNewScene();
	Storm3D_Scene *scene = reinterpret_cast<Storm3D_Scene *> (tempScene);

	Storm3D_VideoPlayer videoPlayer(*Storm3D2, *scene, fileName, streamBuilder);
	videoPlayer.play();

	delete tempScene;
}


//------------------------------------------------------------------
// Model add/remove
//------------------------------------------------------------------
void Storm3D_Scene::AddModel(IStorm3D_Model *mod)
{
	if(!mod)
		return;

	if(models.find((Storm3D_Model*)mod) != models.end())
		return;

	models.insert((Storm3D_Model*)mod);
	if(terrains.empty())
		return;

	std::set<IStorm3D_Terrain *>::iterator it = terrains.begin();
	for(; it != terrains.end(); ++it)
	{
		Storm3D_Terrain &terrain = static_cast<Storm3D_Terrain &> (**it);
		Storm3D_TerrainModels &models = terrain.getModels();

		models.addModel(*mod);
	}
}


void Storm3D_Scene::RemoveModel(IStorm3D_Model *mod)
{
	std::set<IStorm3D_Terrain *>::iterator it = terrains.begin();
	for(; it != terrains.end(); ++it)
	{
		Storm3D_Terrain &terrain = static_cast<Storm3D_Terrain &> (**it);
		Storm3D_TerrainModels &models = terrain.getModels();

		if(mod)
			models.removeModel(*mod);
	}

	models.erase((Storm3D_Model*)mod);
}

void Storm3D_Scene::EnableCulling(IStorm3D_Model *mod, bool enable)
{
	if(!mod)
		return;

	std::set<IStorm3D_Terrain *>::iterator it = terrains.begin();
	for(; it != terrains.end(); ++it)
	{
		Storm3D_Terrain &terrain = static_cast<Storm3D_Terrain &> (**it);
		Storm3D_TerrainModels &models = terrain.getModels();

		Storm3D_Model *model = static_cast<Storm3D_Model *> (mod);
		models.enableCulling(*model, enable);
	}
}


// Stop internal updates (time based)
void Storm3D_Scene::SetPauseState(bool scene_paused_)
{
	scene_paused = scene_paused_;
}

void Storm3D_Scene::DrawBones(bool draw)
{
	draw_bones = draw;
}


//------------------------------------------------------------------
// Terrain add/remove
//------------------------------------------------------------------
void Storm3D_Scene::AddTerrain(IStorm3D_Terrain *ter)
{
	Storm3D_Terrain *terrain = static_cast<Storm3D_Terrain*> (ter);
	terrain->setAmbient(ambient);

	Storm3D_TerrainRenderer &renderer = static_cast<Storm3D_TerrainRenderer &> (terrain->getRenderer());
	renderer.setSkyBox(bg_model);
	renderer.setParticleSystem(particlesystem);

	terrains.insert(terrain);

	std::set<IStorm3D_Model *>::iterator it = models.begin();
	for(; it != models.end(); ++it)
		terrain->getModels().addModel(**it);
}


void Storm3D_Scene::RemoveTerrain(IStorm3D_Terrain *ter)
{
	terrains.erase((Storm3D_Terrain*)ter);
}

//------------------------------------------------------------------
// Lines add/remove
//------------------------------------------------------------------
void Storm3D_Scene::AddLine(IStorm3D_Line *line, bool depth_test)
{
	// Ugh
	Storm3D_Line *l = reinterpret_cast<Storm3D_Line *> (line);

	if(depth_test == true)
		depth_lines.push_back(l);
	else
		no_depth_lines.push_back(l);
}

void Storm3D_Scene::RemoveLine(IStorm3D_Line *line)
{
	// Ugh
	Storm3D_Line *l = reinterpret_cast<Storm3D_Line *> (line);

	for(unsigned int i = 0; i < depth_lines.size(); ++i)
	{
		if(depth_lines[i] == l)
		{
			depth_lines.erase(depth_lines.begin() + i);
			return;
		}
	}

	for(unsigned int i = 0; i < no_depth_lines.size(); ++i)
	{
		if(no_depth_lines[i] == l)
		{
			no_depth_lines.erase(no_depth_lines.begin() + i);
			return;
		}
	}
}

//------------------------------------------------------------------
// Background Model (v2.3 new)
//------------------------------------------------------------------
void Storm3D_Scene::SetBackGround(IStorm3D_Model *mod)
{
	bg_model=(Storm3D_Model*)mod;
	//((Storm3D_TerrainRenderer &) terrain->getRenderer()).setSkyBox(bg_model);

	for(set<IStorm3D_Terrain*>::iterator itr=terrains.begin();itr!=terrains.end();++itr)
	{
		Storm3D_Terrain *terrain = static_cast<Storm3D_Terrain *> (*itr);
		((Storm3D_TerrainRenderer &) terrain->getRenderer()).setSkyBox(bg_model);
	}
}


void Storm3D_Scene::RemoveBackGround()
{
	bg_model=NULL;
	//((Storm3D_TerrainRenderer &) terrain->getRenderer()).setSkyBox(0);

	for(set<IStorm3D_Terrain*>::iterator itr=terrains.begin();itr!=terrains.end();++itr)
	{
		Storm3D_Terrain *terrain = static_cast<Storm3D_Terrain *> (*itr);
		((Storm3D_TerrainRenderer &) terrain->getRenderer()).setSkyBox(0);
	}
}



//------------------------------------------------------------------
// Storm3D_Scene::Render2D_Picture
//------------------------------------------------------------------
void Storm3D_Scene::Render2D_Picture(IStorm3D_Material *mat,VC2 position,VC2 size,float alpha,float rotation,float x1,float y1,float x2,float y2,bool wrap)
{
	// Create new
	Storm3D_Scene_PicList *pl=new Storm3D_Scene_PicList_Picture(Storm3D2,this,(Storm3D_Material*)mat,position,size,alpha,rotation,x1,y1,x2,y2,wrap);

	// Add to list

	// CHANGED: was...	
		// piclist.insert(pl);
	piclist.push_back(pl);
}



//------------------------------------------------------------------
// Storm3D_Scene::Render2D_Picture
//------------------------------------------------------------------
void Storm3D_Scene::Render2D_Picture(IStorm3D_Material *mat,struct VXFORMAT_2D *vertices, int numVertices, float alpha, bool wrap)
{
	// Create new
	Storm3D_Scene_PicList_Picture *pl=new Storm3D_Scene_PicList_Picture(Storm3D2,this,(Storm3D_Material*)mat,VC2(0,0),VC2(1,1),alpha,0,0,0,1,1,wrap);
	pl->createCustomShape(vertices, numVertices);

	// Add to list

	// CHANGED: was...	
		// piclist.insert(pl);
	piclist.push_back(pl);
}



//------------------------------------------------------------------
// Storm3D_Scene::Render3D_Picture
//------------------------------------------------------------------
void Storm3D_Scene::Render3D_Picture(IStorm3D_Material *mat,VC3 position,VC2 size)
{
	// Create new
	Storm3D_Scene_PicList *pl=new Storm3D_Scene_PicList_Picture3D(Storm3D2,this,(Storm3D_Material*)mat,position,size);

	// Add to list
	// CHANGED: was...
		// piclist.insert(pl);
	piclist.push_back(pl);
}



//------------------------------------------------------------------
// Storm3D_Scene::Render2D_Text
//------------------------------------------------------------------
void Storm3D_Scene::Render2D_Text(IStorm3D_Font *font,VC2 position,VC2 size,const char *text,float alpha,const COL &colorFactor)
{
	// Create new
	Storm3D_Scene_PicList *pl=new Storm3D_Scene_PicList_Font(Storm3D2,this,(Storm3D_Font*)font,position,size,text,alpha,colorFactor);

	// Add to list
	// CHANGED: was...
		// piclist.insert(pl);
	piclist.push_back(pl);
}

void Storm3D_Scene::Render2D_Text(IStorm3D_Font *font,VC2 position,VC2 size,const wchar_t *text,float alpha,const COL &colorFactor)
{
	// Create new
	Storm3D_Scene_PicList *pl=new Storm3D_Scene_PicList_Font(Storm3D2,this,(Storm3D_Font*)font,position,size,text,alpha,colorFactor);

	// Add to list
	// CHANGED: was...
		// piclist.insert(pl);
	piclist.push_back(pl);
}


//------------------------------------------------------------------
// Storm3D_Scene::RayTrace
//------------------------------------------------------------------
void Storm3D_Scene::RayTrace(const VC3 &position,const VC3 &direction_normalized,float ray_length,Storm3D_CollisionInfo &rti, bool accurate)
{
	std::set<IStorm3D_Terrain *>::iterator it = terrains.begin();
	for(; it != terrains.end(); ++it)
	{
		Storm3D_Terrain &terrain = static_cast<Storm3D_Terrain &> (**it);
		Storm3D_TerrainModels &models = terrain.getModels();

		if(models.hasTree())
		{
			models.RayTrace(position, direction_normalized, ray_length, rti, accurate);
			return;
		}
	}

	// Raytrace to each model
	for(set<IStorm3D_Model*>::iterator im=models.begin();im!=models.end();im++)
	{
		// Typecast (to simplify code)
		//IStorm3D_Model *md=*im;
		Storm3D_Model *md = static_cast<Storm3D_Model *> (*im);

		// Raytrace
		md->RayTrace(position,direction_normalized,ray_length,rti,accurate);
	}

	// TODO: Add terrain stuff
}



//------------------------------------------------------------------
// Storm3D_Scene::RayTrace
//------------------------------------------------------------------
void Storm3D_Scene::SphereCollision(const VC3 &position,float radius,Storm3D_CollisionInfo &cinf, bool accurate)
{
	std::set<IStorm3D_Terrain *>::iterator it = terrains.begin();
	for(; it != terrains.end(); ++it)
	{
		Storm3D_Terrain &terrain = static_cast<Storm3D_Terrain &> (**it);
		Storm3D_TerrainModels &models = terrain.getModels();

		if(models.hasTree())
		{
			models.SphereCollision(position, radius, cinf, accurate);
			return;
		}
	}

	// Spherecollide to each model
	for(set<IStorm3D_Model*>::iterator im=models.begin();im!=models.end();im++)
	{
		// Typecast (to simplify code)
		IStorm3D_Model *md=*im;

		// Spherecollide
		md->SphereCollision(position,radius,cinf, accurate);
	}

	// TODO: Add terrain stuff
}

void Storm3D_Scene::GetEyeVectors(const VC2I &screen_position, Vector &position_, Vector &direction_)
{
	static const float NEAR_Z = 2.f;

	D3DXMATRIX pProjection;
	D3DXMATRIX pView;

	VC3 camera_up = camera.GetUpVec();
	VC3 camera_position = camera.GetPosition();
	VC3 camera_target = camera.GetTarget();
	
	D3DXMatrixLookAtLH(&pView,(D3DXVECTOR3*)&camera_position, (D3DXVECTOR3*)&camera_target,(D3DXVECTOR3*)&camera_up);

	//Storm3D_SurfaceInfo ss = Storm3D2->GetScreenSize();
	//float aspect=(float)ss.width/(float)ss.height;
	RECT windowSize = { 0 };
	GetClientRect(Storm3D2->window_handle, &windowSize);
	float aspect=(float)windowSize.right/(float)windowSize.bottom;
	
	float fov = camera.GetFieldOfView();
	float vis_range = camera.GetVisibilityRange();

	D3DXVECTOR3 pV;
	D3DXMatrixPerspectiveFovLH(&pProjection,fov,aspect,1.0f,vis_range);

	pV.x =  ( ( ( 2.0f * (float)screen_position.x ) / windowSize.right  ) - 1 ) / pProjection._11;
	pV.y = -( ( ( 2.0f * (float)screen_position.y ) / windowSize.bottom ) - 1 ) / pProjection._22;
	pV.z =  1.0f;

	D3DXMATRIX m;
	D3DXMatrixInverse(&m, NULL, &pView);

	D3DXVECTOR3 vPickRayDir;
	D3DXVECTOR3 vPickRayOrig;

	vPickRayDir.x  = pV.x*m._11 + pV.y*m._21 + pV.z*m._31;
	vPickRayDir.y  = pV.x*m._12 + pV.y*m._22 + pV.z*m._32;
	vPickRayDir.z  = pV.x*m._13 + pV.y*m._23 + pV.z*m._33;
	D3DXVec3Normalize(&vPickRayDir,&vPickRayDir);
	vPickRayOrig.x = m._41;
	vPickRayOrig.y = m._42;
	vPickRayOrig.z = m._43;

	vPickRayOrig+=vPickRayDir*NEAR_Z;

	direction_.x = vPickRayDir.x;
	direction_.y = vPickRayDir.y;
	direction_.z = vPickRayDir.z;
	position_.x = vPickRayOrig.x;
	position_.y = vPickRayOrig.y;
	position_.z = vPickRayOrig.z;
}


//------------------------------------------------------------------
// Storm3D_Scene::GetCamera
//------------------------------------------------------------------
IStorm3D_Camera *Storm3D_Scene::GetCamera()
{
	return &camera;
}



//------------------------------------------------------------------
// Storm3D_Scene::GetParticleSystem
//------------------------------------------------------------------
IStorm3D_ParticleSystem *Storm3D_Scene::GetParticleSystem()
{
	return particlesystem;
}



//------------------------------------------------------------------
// Storm3D_Scene - parameter settings
//------------------------------------------------------------------
void Storm3D_Scene::SetAmbientLight(const COL &color)
{
	this->ambient=color;
	Storm3D_ShaderManager::GetSingleton()->SetAmbient(color);

	for(set<IStorm3D_Terrain*>::iterator itr=terrains.begin();itr!=terrains.end();++itr)
		static_cast<Storm3D_Terrain *> (*itr)->setAmbient(color);
}


void Storm3D_Scene::SetBackgroundColor(const COL &color)
{
	this->bgcolor=color;

	for(set<IStorm3D_Terrain*>::iterator itr=terrains.begin();itr!=terrains.end();++itr)
		static_cast<Storm3D_Terrain *> (*itr)->setClearColor(color);
}


void Storm3D_Scene::SetFogParameters(bool _fog_active,const COL &color,float fog_start_range,float fog_end_range)
{
	/*
	fog_active=_fog_active;
	fog_color=color;
	fog_start=fog_start_range;
	fog_end=fog_end_range;
	Storm3D_ShaderManager::GetSingleton()->SetFog(fog_start, fog_end);
	*/

	for(set<IStorm3D_Terrain*>::iterator itr=terrains.begin();itr!=terrains.end();++itr)
		static_cast<Storm3D_TerrainRenderer &> ((*itr)->getRenderer()).setFog(_fog_active, fog_start_range, fog_end_range, color);
}


//------------------------------------------------------------------
// Storm3D_Scene - Reset routines
//------------------------------------------------------------------
void Storm3D_Scene::ReleaseDynamicDXBuffers()
{
	particlesystem->ReleaseDynamicDXBuffers();
}


void Storm3D_Scene::ReCreateDynamicDXBuffers()
{
	particlesystem->ReCreateDynamicDXBuffers();
}


extern bool enableLocalReflection;
extern int active_visibility;
extern float reflection_height;

//------------------------------------------------------------------
// Storm3D_Scene - Scene rendering
//------------------------------------------------------------------
int Storm3D_Scene::RenderScene(bool present)
{
	// Reset polygon counter
	poly_counter = 0;
	
	{
		static int haxValue = 0;
		++haxValue;

		IStorm3D_Texture *target = Storm3D2->getReflectionTexture();
		if(target /*&& haxValue > 1*/) 
		{
			haxValue = 0;

			enableLocalReflection = true;
			frozenbyte::storm::setInverseCulling(true);
			active_visibility = 1;

			IDirect3DDevice9 &device = *Storm3D2->GetD3DDevice();
			Storm3D_Camera camback = camera;
			camback.Apply();

			Storm3D_Texture *render_target = (Storm3D_Texture *) target;
			if (!render_target->IsCube() && render_target->dx_handle)
			{
				bool renderHalved = true;
				bool renderDistortion = true;
				bool renderGlow = true;
				bool renderGlowImproved = true;
				bool renderFakes = true;
				bool renderFakeShadows = true;
				bool renderSpotShadows = true;
				bool renderParticles = true;

				// ToDo: disable spots / cones as well

				if(!terrains.empty())
				{
					IStorm3D_TerrainRenderer &renderer = (*terrains.begin())->getRenderer();
					bool shouldRenderHalved = Storm3D2->halfReflection;
					bool shouldRenderDistortion = false;
					bool shouldRenderGlow = false;
					bool shouldRenderGlowImproved = false;
					bool shouldRenderFakes = false;
					bool shouldRenderFakeShadows = false;
					bool shouldRenderSpotShadows = false;

					// Particle reflection hacky
					bool shouldRenderParticleReflection = false;
					shouldRenderParticleReflection = renderer.enableFeature(IStorm3D_TerrainRenderer::ParticleReflection, false);
					renderer.enableFeature(IStorm3D_TerrainRenderer::ParticleReflection, shouldRenderParticleReflection);
					renderParticles = renderer.enableFeature(IStorm3D_TerrainRenderer::Particles, shouldRenderParticleReflection);

					if(Storm3D2->reflectionQuality >= 100)
					{
						shouldRenderDistortion = true;
						shouldRenderGlow = true;
					}

					renderHalved = renderer.enableFeature(IStorm3D_TerrainRenderer::HalfRendering, shouldRenderHalved);
					renderDistortion = renderer.enableFeature(IStorm3D_TerrainRenderer::Distortion, shouldRenderDistortion);
					renderGlow = renderer.enableFeature(IStorm3D_TerrainRenderer::Glow, shouldRenderGlow);
					renderGlowImproved = renderer.enableFeature(IStorm3D_TerrainRenderer::BetterGlowSampling, shouldRenderGlowImproved);
					renderFakes = renderer.enableFeature(IStorm3D_TerrainRenderer::FakeLights, shouldRenderFakes);
					renderFakeShadows = renderer.enableFeature(IStorm3D_TerrainRenderer::FakeShadows, shouldRenderFakeShadows);
					renderSpotShadows = renderer.enableFeature(IStorm3D_TerrainRenderer::SpotShadows, shouldRenderSpotShadows);

					// If we already have features disabled, don't enable them for reflection
					if(renderHalved)
						renderer.enableFeature(IStorm3D_TerrainRenderer::HalfRendering, true);
					if(!renderDistortion)
						renderer.enableFeature(IStorm3D_TerrainRenderer::Distortion, false);
					if(!renderGlowImproved)
						renderer.enableFeature(IStorm3D_TerrainRenderer::BetterGlowSampling, false);
					if(!renderGlow)
						renderer.enableFeature(IStorm3D_TerrainRenderer::Glow, false);
					if(!renderFakes)
						renderer.enableFeature(IStorm3D_TerrainRenderer::FakeLights, false);
					if(!renderFakeShadows)
						renderer.enableFeature(IStorm3D_TerrainRenderer::FakeShadows, false);
					if(!renderSpotShadows)
						renderer.enableFeature(IStorm3D_TerrainRenderer::SpotShadows, false);
				}

				VC3 position = camera.GetPosition();
				VC3 target = camera.GetTarget();

				// Mirror relative to reflection plane
				position.y = reflection_height - (position.y - reflection_height);
				target.y = reflection_height - (target.y - reflection_height);

				camera.SetPosition(position);
				camera.SetTarget(target);
				camera.ForceViewProjection(&camback);
				RenderSceneWithParams(false,false,false,true);

				CComPtr<IDirect3DSurface9> surface;
				render_target->dx_handle->GetSurfaceLevel(0, &surface);
				CComPtr<IDirect3DSurface9> originalFrameBuffer;
				device.GetRenderTarget(0, &originalFrameBuffer);

				if(surface && originalFrameBuffer)
					device.StretchRect(originalFrameBuffer, 0, surface, 0, D3DTEXF_NONE);

				if(!terrains.empty())
				{
					IStorm3D_TerrainRenderer &renderer = (*terrains.begin())->getRenderer();

					renderer.enableFeature(IStorm3D_TerrainRenderer::HalfRendering, renderHalved);
					renderer.enableFeature(IStorm3D_TerrainRenderer::Distortion, renderDistortion);
					renderer.enableFeature(IStorm3D_TerrainRenderer::Glow, renderGlow);
					renderer.enableFeature(IStorm3D_TerrainRenderer::BetterGlowSampling, renderGlowImproved);
					renderer.enableFeature(IStorm3D_TerrainRenderer::FakeLights, renderFakes);
					renderer.enableFeature(IStorm3D_TerrainRenderer::FakeShadows, renderFakeShadows);
					renderer.enableFeature(IStorm3D_TerrainRenderer::SpotShadows, renderSpotShadows);
					renderer.enableFeature(IStorm3D_TerrainRenderer::Particles, renderParticles);
				}

				/*
				{
					static IStorm3D_Material *hax = this->Storm3D2->CreateNewMaterial("..");
					hax->SetBaseTexture(render_target);
					this->Render2D_Picture(hax, VC2(300,10), VC2(200,200), 1.f, 0.f, 0.f, 0.f, 1.f, 1.f);
				}
				*/
			}

			active_visibility = 0;
			enableLocalReflection = false;
			frozenbyte::storm::setInverseCulling(false);

			camera = camback;
		}

	}

	// Render with flip
	RenderSceneWithParams(present, false);

	// Return polygon count
	return poly_counter;
}

int Storm3D_Scene::RenderSceneToDynamicTexture(IStorm3D_Texture *target,int face)
{
	// Test params
	if (!target) 
		return 0;

	//enableLocalReflection = true;
	//frozenbyte::storm::setInverseCulling(true);
	//active_visibility = 1;

	// Reset polygon counter
	poly_counter=0;

	//IStorm3D_Texture * oldTarget = Storm3D2->getRenderTarget( face );

	// Set rendertarget to texture
	if (Storm3D2->SetRenderTarget((Storm3D_Texture*)target,face))
	{
		// Render without flip
		RenderSceneWithParams(false,false,true,false);
	}

	/*
	IDirect3DDevice9 &device = *Storm3D2->GetD3DDevice();
	Storm3D_Camera camback = camera;
	camback.Apply();

	Storm3D_Texture *render_target = (Storm3D_Texture *) target;
	if (!render_target->IsCube() && render_target->dx_handle)
	{
		VC3 position = camera.GetPosition();
		VC3 target = camera.GetTarget();

		// ToDo: mirror relative to reflection plane
		//position.y = -position.y;
		//target.y = -target.y;
		position.y = reflection_height - (position.y - reflection_height);
		target.y = reflection_height - (target.y - reflection_height);

		camera.SetPosition(position);
		camera.SetTarget(target);
		camera.ForceViewProjection(&camback);
		RenderSceneWithParams(false,false,false,true);

		CComPtr<IDirect3DSurface9> surface;
		render_target->dx_handle->GetSurfaceLevel(0, &surface);
		CComPtr<IDirect3DSurface9> originalFrameBuffer;
		device.GetRenderTarget(0, &originalFrameBuffer);

		if(surface && originalFrameBuffer)
			device.StretchRect(originalFrameBuffer, 0, surface, 0, D3DTEXF_NONE);
	}

	active_visibility = 0;
	enableLocalReflection = false;
	frozenbyte::storm::setInverseCulling(false);

	// Set rendertarget back to backbuffer
	Storm3D2->SetRenderTarget(NULL);

	// Return polygon count
	camera=camback;
	*/

	// Set rendertarget back to backbuffer
	Storm3D2->SetRenderTarget( NULL );
	return poly_counter;
}


void Storm3D_Scene::RenderSceneToAllDynamicCubeTexturesInScene()
{
	// Save scene's camera
	Storm3D_Camera camback=camera;

	// Loop models
	for (set<IStorm3D_Model*>::iterator mit=models.begin();mit!=models.end();mit++)
	{
		// Typecast (to simplify code)
		Storm3D_Model *mod=(Storm3D_Model*)*mit;

		// Loop objects
		for(set<IStorm3D_Model_Object*>::iterator io=mod->objects.begin();io!=mod->objects.end();io++)
		{	
			// Typecast (to simplify code)
			Storm3D_Model_Object *obj=(Storm3D_Model_Object*)*io;

			// Object must have mesh
			if (obj->mesh==NULL) continue;

			// Mesh must have material
			if (!obj->mesh->GetMaterial()) continue;

			// Get materials reflection texture
			Storm3D_Texture *tex=(Storm3D_Texture*)obj->mesh->GetMaterial()->GetReflectionTexture();
			if ((tex)&&(tex->IsCube())&&(tex->IsRenderTarget()))
			{
				// Get object global pos
				//VC3 ogb=obj->GetGlobalPosition();
				//VC3 ogb(0, 0, 10.f);
				VC3 ogb = camera.GetPosition();

				// Set camera
				camera.SetPosition(ogb);
				camera.SetFieldOfView((float)PI/2.0f);

				// Cubefaces: 0=pX, 1=nX, 2=pY, 3=nY, 4=pZ, 5=nZ (p=positive, n=negative)

				// Render pX
				camera.SetTarget(ogb+VC3(1,0,0));
				camera.SetUpVec(VC3(0,1,0));
				RenderSceneToDynamicTexture(tex,0);

				// Render nX
				camera.SetTarget(ogb+VC3(-1,0,0));
				RenderSceneToDynamicTexture(tex,1);

				// Render pY
				camera.SetTarget(ogb+VC3(0,1,0));
				camera.SetUpVec(VC3(0,0,-1));
				RenderSceneToDynamicTexture(tex,2);

				// Render nY
				camera.SetTarget(ogb+VC3(0,-1,0));
				camera.SetUpVec(VC3(0,0,1));
				RenderSceneToDynamicTexture(tex,3);

				// Render pZ
				camera.SetTarget(ogb+VC3(0,0,1));
				camera.SetUpVec(VC3(0,1,0));
				RenderSceneToDynamicTexture(tex,4);

				// Render nZ
				camera.SetTarget(ogb+VC3(0,0,-1));
				RenderSceneToDynamicTexture(tex,5);
			}

			// !!
			break;
		}

		// !!
		break;
	}

	// Return scene's camera
	camera=camback;

	// TODO: Add terrain stuff
}

//------------------------------------------------------------------
// Storm3D_Scene - Parameter changes
//------------------------------------------------------------------
void Storm3D_Scene::SetAnisotropicFilteringLevel(int level)
{
	// Get max anisotropy level
	int maxani=Storm3D2->adapters[Storm3D2->active_adapter].max_anisotropy;

	// Set level
	anisotropic_level=pick_min(level,maxani);
	if (anisotropic_level<0) anisotropic_level=0;
}

void Storm3D_Scene::AddTriangle(const VC3 &p1, const VC3 &p2, const VC3 &p3, const COL &color)
{
	if(!camera.TestPointVisibility(p1))
		return;
	if(!camera.TestPointVisibility(p2))
		return;
	if(!camera.TestPointVisibility(p3))
		return;

	Debug3 d;
	d.p1 = p1;
	d.p2 = p2;
	d.p3 = p3;
	d.color = color;

	if(debugTriangles.size() < 20000)
		debugTriangles.push_back(d);
}

void Storm3D_Scene::AddLine(const VC3 &p1, const VC3 &p2, const COL &color)
{
//	if(!camera.TestPointVisibility(p1) && !camera.TestPointVisibility(p2))
//		return;

	Debug2 d;
	d.p1 = p1;
	d.p2 = p2;
	d.color = color;

	if(debugLines.size() < 20000)
		debugLines.push_back(d);
}

void Storm3D_Scene::AddPoint(const VC3 &p1, const COL &color)
{
	if(!camera.TestPointVisibility(p1))
		return;

	Debug1 d;
	d.p1 = p1;
	d.color = color;

	if(debugPoints.size() < 20000)
		debugPoints.push_back(d);
}

void Storm3D_Scene::setWorldFoldCenter(const VC3 &position)
{
#ifdef WORLD_FOLDING_ENABLED
	WorldFold::setWorldFoldCenter(position);
#else
	assert(!"Storm3D_Scene::setWorldFoldCenter - world folding not enabled.");
#endif
}

void Storm3D_Scene::addWorldFoldAtPosition(const VC3 &position, const MAT &fold)
{
#ifdef WORLD_FOLDING_ENABLED
	WorldFold::addWorldFoldAtPosition(position, fold);
#else
	assert(!"Storm3D_Scene::addWorldFoldAtPosition - world folding not enabled.");
#endif
}

void Storm3D_Scene::changeWorldFoldAtPosition(const VC3 &position, const MAT &fold)
{
#ifdef WORLD_FOLDING_ENABLED
	WorldFold::changeWorldFoldAtPosition(position, fold);
#else
	assert(!"Storm3D_Scene::changeWorldFoldAtPosition - world folding not enabled.");
#endif
}

void Storm3D_Scene::resetWorldFold()
{
#ifdef WORLD_FOLDING_ENABLED
	WorldFold::resetWorldFold();
#else
	assert(!"Storm3D_Scene::resetWorldFold - world folding not enabled.");
#endif
}
