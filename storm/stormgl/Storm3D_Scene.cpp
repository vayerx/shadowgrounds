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
#ifdef _MSC_VER
#include <windows.h>
#endif

#include <GL/glew.h>
#include "storm3d.h"
#include "storm3d_scene_piclist.h"
#include "storm3d_adapter.h"
#include "storm3d_material.h"
#include "storm3d_mesh.h"
#include "storm3d_helper.h"
#include "storm3d_model_object.h"
#include "storm3d_model.h"
#include "storm3d_texture.h"
#include "storm3d_particle.h"
#include "storm3d_scene.h"
#include "storm3d_terrain.h"
#include "storm3d_terrain_renderer.h"
#include "storm3d_terrain_models.h"
#include "storm3d_video_player.h"
#include "Iterator.h"

#include "Storm3D_Bone.h"
#include "Storm3D_ShaderManager.h"
#include "Storm3D_Line.h"
#include "igios3D.h"

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

//! Constructor
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
	basic_shader(),
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

//! Destructor
Storm3D_Scene::~Storm3D_Scene()
{
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
		igiosWarning("%i: %d \n", i, bottlenecks[i]);
	}
#endif
}

//! Renders the scene with parameters
/*!
	\param flip
	\param disable_hsr
	\param update_time
	\param render_mirrored
*/
void Storm3D_Scene::RenderSceneWithParams(bool flip,bool disable_hsr, bool update_time, bool render_mirrored, IStorm3D_Texture *target)
{
	storm3d_dip_calls = 0;

	// Calculate time difference
	static DWORD last_time=SDL_GetTicks();
	DWORD time_now=SDL_GetTicks();
	if (flip)
	{
		time_dif=time_now-last_time;
		// added use of timing factor...
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
		if (this->Storm3D2->timeFactor != 1.0f)
		{
			// FIXME: may have a small error on some values
			// should work just fine for factor values like 0.5 though.
			time_dif = (int)(float(time_dif) * this->Storm3D2->timeFactor);
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
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DITHER);
	glDisable(GL_NORMALIZE);
	//enable camera-relative specular highlights?

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

		// psd: Is the whole model visible (no animation/object stuff)
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

			// Calculate list position (optimize)
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

	// Start REAL scene rendering
#ifdef NVPERFSDK
	if (flip) {
		int nCount = 0;
		NVPMBeginExperiment(&nCount);
		if (nCount > 0) {
			igiosWarning("begin experiment, %d cycles\n", nCount);
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
			igiosWarning("GPU Bottleneck value: %lu cycles: %lu\n", value, cycles);
			igiosWarning("Bottleneck : %s\n", bname);
			delete[] bname;
		} else {
			Storm3D_Texture *tgt = static_cast<Storm3D_Texture*>(target);
			renderRealScene(flip, render_mirrored, tgt);
		}
	} else {
		Storm3D_Texture *tgt = static_cast<Storm3D_Texture*>(target);
		renderRealScene(flip, render_mirrored, tgt);
	}
#else
	Storm3D_Texture *tgt = static_cast<Storm3D_Texture*>(target);
	renderRealScene(flip, render_mirrored, tgt);
#endif
}


void Storm3D_Scene::renderRealScene(bool flip, bool render_mirrored, Storm3D_Texture *target) {
	glDisable(GL_SCISSOR_TEST);

	if(!flip && !render_mirrored)
	{
		int clearFlag = GL_DEPTH_BUFFER_BIT;
		if(Storm3D2->support_stencil)
			clearFlag |= GL_STENCIL_BUFFER_BIT;

		// Clear screen and depth buffer
		glClearColor(bgcolor.r, bgcolor.g, bgcolor.b, 1.0f);
		glClearDepth(1.0f);
		glClearStencil(0);
		glClear(clearFlag | GL_COLOR_BUFFER_BIT);
	}
	else
	{
		Storm3D2->getProceduralManagerImp().update(time_dif);
		// Clear screen
		glClearColor(bgcolor.r, bgcolor.g, bgcolor.b, 1.0f);
		glClearDepth(1.0f);
		glClearStencil(0);
		glClear(GL_COLOR_BUFFER_BIT);
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

	if (target)
	{
		Storm3D2->SetRenderTarget(target);

		glClearColor(bgcolor.r, bgcolor.g, bgcolor.b, 1.0f);
		glClearDepth(1.0f);
		glClearStencil(0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	// Render terrains
	for(set<IStorm3D_Terrain*>::iterator itr=terrains.begin();itr!=terrains.end();++itr)
	{
		// Typecast (to simplify code)
		Storm3D_Terrain *terra=(Storm3D_Terrain*)*itr;
		Storm3D_TerrainRenderer &renderer = static_cast<Storm3D_TerrainRenderer &> (terra->getRenderer());

		// Render it!
		renderer.renderBase(*this);
	}

	// Fix flickering (random stuff as textures)
	Storm3D2->active_material=0;

	// Set fog
	if (fog_active)
	{
		// psd: no pixel fog for shaders
		// psd: disable fog table
		GLfloat fogc[4];
		fogc[0] = fog_color.r;
		fogc[1] = fog_color.g;
		fogc[2] = fog_color.b;
		fogc[3] = 0;
		glFogfv(GL_FOG_COLOR, fogc);
	}

	Storm3D_ShaderManager::GetSingleton()->ResetShader();
	Storm3D_ShaderManager::GetSingleton()->ClearCache();
	Storm3D_ShaderManager::GetSingleton()->setLightingShaders();

	// Render objects in list (to screen)
	for (int i=0;renderlist_obj[i];i++)
	{
		if(i == 0)
		{
			glActiveTexture(GL_TEXTURE0);
			glClientActiveTexture(GL_TEXTURE0);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
			glActiveTexture(GL_TEXTURE1);
			glClientActiveTexture(GL_TEXTURE1);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_TEXTURE_3D);
			glDisable(GL_TEXTURE_CUBE_MAP);
			glDisable(GL_ALPHA_TEST);
			glDisable(GL_BLEND);

			this->camera.Apply();
		}

		// Set shader constants
		Storm3D_Material *m = static_cast<Storm3D_Material *> (renderlist_obj[i]->mesh->GetMaterial());
		Storm3D_Model *mod = renderlist_obj[i]->parent_model;

		if(m)
		{
			Storm3D_ShaderManager::GetSingleton()->SetObjectAmbient(m->GetSelfIllumination());
			Storm3D_ShaderManager::GetSingleton()->SetObjectDiffuse(m->GetColor());
			Storm3D_Texture *t = (Storm3D_Texture *) m->GetBaseTexture();
		
			if(t)
			{
				t->Apply(0);
				glActiveTexture(GL_TEXTURE0);
				glClientActiveTexture(GL_TEXTURE0);
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
			}
			else
			{
				glActiveTexture(GL_TEXTURE0);
				glClientActiveTexture(GL_TEXTURE0);
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
			}

			int alphaType = m->GetAlphaType();
			if(alphaType == IStorm3D_Material::ATYPE_NONE)
			{
				glDisable(GL_BLEND);
			}
			else
			{
				glEnable(GL_BLEND);

				if(alphaType == IStorm3D_Material::ATYPE_ADD)
				{
					glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				}
				else if(alphaType == IStorm3D_Material::ATYPE_MUL)
				{
					glBlendFunc(GL_ZERO, GL_SRC_COLOR);
				}
				else if(alphaType == IStorm3D_Material::ATYPE_USE_TRANSPARENCY)
				{
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				}
				else if(alphaType == IStorm3D_Material::ATYPE_USE_TEXTRANSPARENCY || renderlist_obj[i]->force_alpha > 0.0001f)
				{
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
		
		// Horrible ...
		{
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
		Storm3D_ShaderManager::GetSingleton()->SetShader(renderlist_obj[i]);
		basic_shader.apply();

		if(!renderlist_obj[i]->parent_model->bones.empty())
		{
			renderlist_obj[i]->mesh->ReBuild();
			renderlist_obj[i]->mesh->RenderBuffers(renderlist_obj[i]);
		}

		glDisable(GL_BLEND);
	}

	glActiveTexture(GL_TEXTURE0);
	glClientActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_3D);
	glDisable(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_2D, 0);
	frozenbyte::storm::VertexShader::disable();

	// Set renderstates (for shadows/particles/sprites)
	//SPECULARENABLE OFF?
	glDisable(GL_FOG);
	//GL_NORMALIZE OFF?
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Apply the original camera back!
	camera.Apply();

	// Render particles
	glEnable(GL_SCISSOR_TEST);
	if(terrains.empty())
		particlesystem->Render(this);
	glDisable(GL_SCISSOR_TEST);

	// Set renderstates for sprite rendering
	glActiveTexture(GL_TEXTURE0);
	glClientActiveTexture(GL_TEXTURE0);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
	glActiveTexture(GL_TEXTURE1);
	glClientActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_3D);
	glDisable(GL_TEXTURE_CUBE_MAP);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Renderstate for lines
	glDisable(GL_ALPHA_TEST);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	// Textures off
	for (int i=0;i<3;i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glClientActiveTexture(GL_TEXTURE0 + i);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_3D);
		glDisable(GL_TEXTURE_CUBE_MAP);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// Render lines
	{
		frozenbyte::storm::setCulling(CULL_NONE);

		D3DXMATRIX dm;
		D3DXMatrixIdentity(dm);
		Storm3D_ShaderManager::GetSingleton()->SetWorldTransform(dm);

		if(!depth_lines.empty())
		{
			for(unsigned int i = 0; i < depth_lines.size(); ++i)
				depth_lines[i]->Render();
		}

		if(!no_depth_lines.empty())
		{
			glDisable(GL_DEPTH_TEST);

			for(unsigned int i = 0; i < no_depth_lines.size(); ++i)
				no_depth_lines[i]->Render();
			
			glEnable(GL_DEPTH_TEST);
		}

		frozenbyte::storm::setCulling(CULL_CCW);
	}

	// Debug rendering
	if(!debugTriangles.empty() || !debugLines.empty() || !debugPoints.empty())
	{
		D3DXMATRIX dm;
		D3DXMatrixIdentity(dm);
		Storm3D_ShaderManager::GetSingleton()->SetWorldTransform(dm);

		int vertexAmount = (debugTriangles.size() * 3) + (debugLines.size() * 2) + (debugPoints.size());

		frozenbyte::storm::VertexBuffer vertexBuffer;
		vertexBuffer.create(vertexAmount, sizeof(VXFORMAT_PSD), true);
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
		applyFVF(FVF_VXFORMAT_PSD, sizeof(VXFORMAT_PSD));
		vertexBuffer.apply(0);

		frozenbyte::storm::VertexShader::disable();

		glActiveTexture(GL_TEXTURE0);
		glClientActiveTexture(GL_TEXTURE0);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
		glDisable(GL_BLEND);

		if(!debugTriangles.empty())
			glDrawArrays(GL_TRIANGLES, 0, 3 * debugTriangles.size());
		if(!debugLines.empty())
			glDrawArrays(GL_LINES, lineOffset, 2 * debugLines.size());
		if(!debugPoints.empty())
			glDrawArrays(GL_POINTS, pointOffset, debugPoints.size());

		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
		glEnable(GL_BLEND);
	}

	// Render bones. SLOOOOOOOOW!
	//draw_bones = true;

	// Ugly hack anyway
	//		-- psd
	if(draw_bones)
	{
		D3DXMATRIX dm;
		D3DXMatrixIdentity(dm);
		Storm3D_ShaderManager::GetSingleton()->SetWorldTransform(dm);

		for(set<IStorm3D_Model *>::iterator mit=models.begin();mit!=models.end();++mit)
		{
			// Typecast (to simplify code)
			Storm3D_Model *mod=(Storm3D_Model*)*mit;

			if(mod->bones.size() > 0)
			{
				glDisable(GL_DEPTH_TEST);
				glColor3f(1.0, 1.0, 1.0);

				if(true)
				{
					// applyFVF(D3DFVF_XYZ|D3DFVF_NORMAL, sizeof(VXFORMAT_PSD));
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

					for(unsigned int i = 0; i < mod->bones.size(); ++i)
					{
						Storm3D_Bone *b = mod->bones[i];
						float thickness = b->GetThickness();
						thickness = 0.01f;

						GLUquadric* mesh;
						mesh = gluNewQuadric();

						Matrix global_tm = b->GetTM();
						D3DXMATRIX tm;
						global_tm.GetAsD3DCompatible4x4(&tm.m[0][0]);
						tm._41 += (-thickness + (0.5f * b->GetLenght())) * tm._31;
						tm._42 += (-thickness + (0.5f * b->GetLenght())) * tm._32;
						tm._43 += (-thickness + (0.5f * b->GetLenght())) * tm._33;

						Storm3D_ShaderManager::GetSingleton()->SetWorldTransform(tm);
						gluCylinder(mesh, thickness, thickness, b->GetLenght() + 2*thickness, 4, 4);
					}

					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				}
			}
		}

		Storm3D_ShaderManager::GetSingleton()->SetWorldTransform(dm);
	}

	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_ALPHA_TEST);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	Storm3D_SurfaceInfo screen = Storm3D2->GetScreenSize();
	glOrtho(0, screen.width, screen.height, 0, -1, 1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Render picturelist
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

	// Clear picturelist
	if(!render_mirrored)
	{
		piclist.clear();
		debugTriangles.clear();
		debugLines.clear();
		debugPoints.clear();
	}

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);

	// End REAL scene rendering

	// Present the scene (flip)
	if (target)
		Storm3D2->SetRenderTarget(NULL);
	else if (flip)
		SDL_GL_SwapBuffers();
	//else if(!render_mirrored)
		//Storm3D2->D3DDevice->SetDepthStencilSurface(originalDepthBuffer);
}


void Storm3D_Scene::RenderVideo(const char *fileName, IStorm3D_StreamBuilder *streamBuilder)
{
	IStorm3D_Scene *tempScene = Storm3D2->CreateNewScene();
	Storm3D_Scene *scene = reinterpret_cast<Storm3D_Scene *> (tempScene);

	Storm3D_VideoPlayer videoPlayer(*Storm3D2, *scene, fileName, streamBuilder);
	videoPlayer.play();

	delete tempScene;
}

//! Model add
/*
	\param mod model
*/
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

//! Model remove
/*
	\param mod model
*/
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

//! Stop internal updates (time based)
/*
	\param scene_paused_ true if paused
*/
void Storm3D_Scene::SetPauseState(bool scene_paused_)
{
	scene_paused = scene_paused_;
}

//! Draw bones
/*
	\param draw true if drawing
*/
void Storm3D_Scene::DrawBones(bool draw)
{
	draw_bones = draw;
}

//! Terrain add
/*
	\param ter terrain
*/
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

//! Terrain remove
/*
	\param ter
*/
void Storm3D_Scene::RemoveTerrain(IStorm3D_Terrain *ter)
{
	terrains.erase((Storm3D_Terrain*)ter);
}

//! Add line
/*
	\param line
	\param depth_test
*/
void Storm3D_Scene::AddLine(IStorm3D_Line *line, bool depth_test)
{
	Storm3D_Line *l = reinterpret_cast<Storm3D_Line *> (line);

	if(depth_test == true)
		depth_lines.push_back(l);
	else
		no_depth_lines.push_back(l);
}

//! Remove line
/*
	\param line
*/
void Storm3D_Scene::RemoveLine(IStorm3D_Line *line)
{
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

//! Background Model (v2.3 new)
/*
	\param mod model
*/
void Storm3D_Scene::SetBackGround(IStorm3D_Model *mod)
{
	bg_model=(Storm3D_Model*)mod;

	for(set<IStorm3D_Terrain*>::iterator itr=terrains.begin();itr!=terrains.end();++itr)
	{
		Storm3D_Terrain *terrain = static_cast<Storm3D_Terrain *> (*itr);
		((Storm3D_TerrainRenderer &) terrain->getRenderer()).setSkyBox(bg_model);
	}
}

//! Background Model remove
void Storm3D_Scene::RemoveBackGround()
{
	bg_model=NULL;

	for(set<IStorm3D_Terrain*>::iterator itr=terrains.begin();itr!=terrains.end();++itr)
	{
		Storm3D_Terrain *terrain = static_cast<Storm3D_Terrain *> (*itr);
		((Storm3D_TerrainRenderer &) terrain->getRenderer()).setSkyBox(0);
	}
}

//! Adds a 2D picture to the list to be rendered
/*!
	\param mat material
	\param position position
	\param size size
	\param alpha alpha
	\param rotation rotation
	\param x1
	\param y1
	\param x2
	\param y2
*/
void Storm3D_Scene::Render2D_Picture(IStorm3D_Material *mat,VC2 position,VC2 size,float alpha,float rotation,float x1,float y1,float x2,float y2,bool wrap)
{
	// Create new
	Storm3D_Scene_PicList *pl=new Storm3D_Scene_PicList_Picture(Storm3D2,this,(Storm3D_Material*)mat,position,size,alpha,rotation,x1,y1,x2,y2,wrap);

	// Add to list
	piclist.push_back(pl);
}

//! Adds a 2D picture to the list to be rendered
/*!
	\param mat material
	\param vertices vertices
	\param numVertices number of vertices
	\param alpha alpha
	\param wrap true to wrap
*/
void Storm3D_Scene::Render2D_Picture(IStorm3D_Material *mat,struct VXFORMAT_2D *vertices, int numVertices, float alpha, bool wrap)
{
	// Create new
	Storm3D_Scene_PicList_Picture *pl=new Storm3D_Scene_PicList_Picture(Storm3D2,this,(Storm3D_Material*)mat,VC2(0,0),VC2(1,1),alpha,0,0,0,1,1,wrap);
	pl->createCustomShape(vertices, numVertices);

	// Add to list
	piclist.push_back(pl);
}

//! Adds a 3D picture to the list to be rendered
/*!
	\param mat material
	\param position position
	\param size size
*/
void Storm3D_Scene::Render3D_Picture(IStorm3D_Material *mat,VC3 position,VC2 size)
{
	// Create new
	Storm3D_Scene_PicList *pl=new Storm3D_Scene_PicList_Picture3D(Storm3D2,this,(Storm3D_Material*)mat,position,size);

	// Add to list
	piclist.push_back(pl);
}

//! Adds 2D text to the list to be rendered
/*!
	\param font font
	\param position position
	\param size size
	\param text text
	\param alpha alpha
	\param colorFactor color factor
*/
void Storm3D_Scene::Render2D_Text(IStorm3D_Font *font,VC2 position,VC2 size,const char *text,float alpha,const COL &colorFactor)
{
	// Create new
	Storm3D_Scene_PicList *pl=new Storm3D_Scene_PicList_Font(Storm3D2,this,(Storm3D_Font*)font,position,size,text,alpha,colorFactor);

	// Add to list
	piclist.push_back(pl);
}

//! Adds 2D Unicode text to the list to be rendered
/*!
	\param font font
	\param position position
	\param size size
	\param text text
	\param alpha alpha
	\param colorFactor color factor
*/
void Storm3D_Scene::Render2D_Text(IStorm3D_Font *font,VC2 position,VC2 size,const wchar_t *text,float alpha,const COL &colorFactor)
{
	// Create new
	Storm3D_Scene_PicList *pl=new Storm3D_Scene_PicList_Font(Storm3D2,this,(Storm3D_Font*)font,position,size,text,alpha,colorFactor);

	// Add to list
	piclist.push_back(pl);
}

//! Raytrace
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
		Storm3D_Model *md = static_cast<Storm3D_Model *> (*im);

		// Raytrace
		md->RayTrace(position,direction_normalized,ray_length,rti,accurate);
	}

	// TODO: Add terrain stuff
}

//! Sphere collision
/*!
	\param position position
	\param radius radius
	\param cinf collision info
	\param accurate
*/
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
}

//! Get eye vectors
/*!
	\param screen_position screen position
	\param position_ position
	\param direction_ direction
*/
void Storm3D_Scene::GetEyeVectors(const VC2I &screen_position, Vector &position_, Vector &direction_)
{
	static const float NEAR_Z = 2.f;

	D3DXMATRIX pProjection;
	D3DXMATRIX pView;

	VC3 camera_up = camera.GetUpVec();
	VC3 camera_position = camera.GetPosition();
	VC3 camera_target = camera.GetTarget();
	
	D3DXMatrixLookAtLH(pView, camera_position, camera_target, camera_up);

	//RECT windowSize = { 0 };
	//GetClientRect(Storm3D2->window_handle, &windowSize);
	igios_unimplemented();
	Storm3D_SurfaceInfo ss = Storm3D2->GetScreenSize();
	float aspect=(float) ss.width / (float) ss.height;

	float fov = camera.GetFieldOfView();
	float vis_range = camera.GetVisibilityRange();

	VC3 pV;
	D3DXMatrixPerspectiveFovLH(pProjection,fov,aspect,1.0f,vis_range);

	pV.x = 1.0f;
	pV.y = 1.0f;
	pV.z = 1.0f;
	//pV.x =  ( ( ( 2.0f * (float)screen_position.x ) / windowSize.right  ) - 1 ) / pProjection._11;
	//pV.y = -( ( ( 2.0f * (float)screen_position.y ) / windowSize.bottom ) - 1 ) / pProjection._22;
	//pV.z =  1.0f;

	D3DXMATRIX m;
	D3DXMatrixInverse(m, NULL, pView);

	VC3 vPickRayDir;
	VC3 vPickRayOrig;

	vPickRayDir.x  = pV.x*m._11 + pV.y*m._21 + pV.z*m._31;
	vPickRayDir.y  = pV.x*m._12 + pV.y*m._22 + pV.z*m._32;
	vPickRayDir.z  = pV.x*m._13 + pV.y*m._23 + pV.z*m._33;
	vPickRayDir = vPickRayDir.GetNormalized();
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

//! Get camera
/*!
	\return camera
*/
IStorm3D_Camera *Storm3D_Scene::GetCamera()
{
	return &camera;
}

//! Get particle system
/*!
	\return particle system
*/
IStorm3D_ParticleSystem *Storm3D_Scene::GetParticleSystem()
{
	return particlesystem;
}

//! Set ambient light
/*!
	\param color light color
*/
void Storm3D_Scene::SetAmbientLight(const COL &color)
{
	this->ambient=color;
	Storm3D_ShaderManager::GetSingleton()->SetAmbient(color);

	for(set<IStorm3D_Terrain*>::iterator itr=terrains.begin();itr!=terrains.end();++itr)
		static_cast<Storm3D_Terrain *> (*itr)->setAmbient(color);
}

//! Set background color
/*!
	\param color background color
*/
void Storm3D_Scene::SetBackgroundColor(const COL &color)
{
	this->bgcolor=color;

	for(set<IStorm3D_Terrain*>::iterator itr=terrains.begin();itr!=terrains.end();++itr)
		static_cast<Storm3D_Terrain *> (*itr)->setClearColor(color);
}

//! Set fog parameters
/*!
	\param _fog_active
	\param color
	\param fog_start_range
	\param fog_end_range
*/
void Storm3D_Scene::SetFogParameters(bool _fog_active,const COL &color,float fog_start_range,float fog_end_range)
{
	for(set<IStorm3D_Terrain*>::iterator itr=terrains.begin();itr!=terrains.end();++itr)
		static_cast<Storm3D_TerrainRenderer &> ((*itr)->getRenderer()).setFog(_fog_active, fog_start_range, fog_end_range, color);
}

extern bool enableLocalReflection;
extern int active_visibility;
extern float reflection_height;

//! Renders the scene
/*!
	\param present
	\return polygon count
*/
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

			Storm3D_Camera camback = camera;
			camback.Apply();

			Storm3D_Texture *render_target = (Storm3D_Texture *) target;
			if (!render_target->IsCube())
			{
				bool renderHalved = true;
				bool renderDistortion = true;
				bool renderGlow = true;
				bool renderGlowImproved = true;
				bool renderFakes = true;
				bool renderFakeShadows = true;
				bool renderSpotShadows = true;
				bool renderParticles = true;

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
				RenderSceneWithParams(false,false,false,true,render_target);

				igios_unimplemented();

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

				// debugging code
				if(false){
					static IStorm3D_Material *hax = this->Storm3D2->CreateNewMaterial("..");
					hax->SetBaseTexture(render_target);
					this->Render2D_Picture(hax, VC2(300,10), VC2(512,512), 1.f, 0.f, 0.f, 0.f, 1.f, 1.f, false);
				}
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

//! Render scene to dynamic texture
/*!
	\param target target texture
	\param face
	\return polygon count
*/
int Storm3D_Scene::RenderSceneToDynamicTexture(IStorm3D_Texture *target,int face)
{
	// Test params
	if (!target) 
		return 0;

	// Reset polygon counter
	poly_counter=0;

	// Set rendertarget to texture
	if (Storm3D2->SetRenderTarget((Storm3D_Texture*)target,face))
	{
		// Render without flip
		RenderSceneWithParams(false,false,true,false);
	}

	// Set rendertarget back to backbuffer
	Storm3D2->SetRenderTarget( NULL );
	return poly_counter;
}

//! Render scene to all dynamic cube textures in scene
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
}

//! Set anisotropic filtering level
/*!
	\param level filtering level
*/
void Storm3D_Scene::SetAnisotropicFilteringLevel(int level)
{
	// Get max anisotropy level
	int maxani=1; //Storm3D2->maxanisotropy;

	// Set level
	anisotropic_level=pick_min(level,maxani);
	if (anisotropic_level<0)
		anisotropic_level=0;
}

//! Add triangle
/*!
	\param p1 point 1
	\param p2 point 2
	\param p3 point 3
	\param color color
*/
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

//! Add line
/*!
	\param p1 point 1
	\param p2 point 2
	\param color color
*/
void Storm3D_Scene::AddLine(const VC3 &p1, const VC3 &p2, const COL &color)
{
	Debug2 d;
	d.p1 = p1;
	d.p2 = p2;
	d.color = color;

	if(debugLines.size() < 20000)
		debugLines.push_back(d);
}

//! Add point
/*!
	\param p1 point
	\param color color
*/
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
