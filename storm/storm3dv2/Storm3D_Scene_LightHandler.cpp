/*

  Storm3D v2.0 T&L Graphics Engine
  (C) Sebastian Aaltonen 2000

  Class: Storm3D_Scene_LightHandler

  Does all lighting stuff (in a optimized way)
*/



//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d.h"
#include "storm3d_mesh.h"
#include "storm3d_texture.h"
#include "storm3d_lensflare.h"
#include "storm3d_model_object.h"
#include "storm3d_model.h"
#include "storm3d_light.h"
#include "storm3d_scene_lighthandler.h"
#include "storm3d_scene.h"
#include "Clipper.h"
#include "VertexFormats.h"
#include "../../util/Debug_MemoryManager.h"



//------------------------------------------------------------------
// Storm3D_Scene_LightHandler::UpdateLights
//------------------------------------------------------------------
Storm3D_Scene_LightHandler::Storm3D_Scene_LightHandler(Storm3D *s2,Storm3D_Scene *scene) :
	Storm3D2(s2)
{
	UpdateLights(scene);
}



//------------------------------------------------------------------
// Storm3D_Scene_LightHandler::UpdateLights
//------------------------------------------------------------------
Storm3D_Scene_LightHandler::~Storm3D_Scene_LightHandler()
{
	lights.clear();
}



//------------------------------------------------------------------
// Storm3D_Scene_LightHandler::UpdateLights
//------------------------------------------------------------------
void Storm3D_Scene_LightHandler::UpdateLights(Storm3D_Scene *scene)
{
	lights.clear();

	// Loop through each model in scene
	for (set<IStorm3D_Model*>::iterator it=scene->models.begin();it!=scene->models.end();it++)
	{
		// Typecast (to simplify code)
		Storm3D_Model *md=(Storm3D_Model*)*it;

		// Loop through each light in model
		for (set<IStorm3D_Light*>::iterator il=md->lights.begin();il!=md->lights.end();il++)
		{
			// Typecast (to simplify code)
			IStorm3D_Light *lgt=(*il);

			// Add the light into the set (OK)
			lights.insert(lgt);
		}
	}
}



//------------------------------------------------------------------
// Storm3D_Scene_LightHandler::ApplyLights
//------------------------------------------------------------------
void Storm3D_Scene_LightHandler::ApplyLights(Storm3D_Model_Object *obj)
{	
	IStorm3D_Light **tlights=obj->GetMILights();	// New: stores lights into the object
	int points[STORM3DV2_MAX_ACTIVELIGHTS];
	int light_num=0;
	int lowest_points=999999;

	// Clear
	for (int i=0;i<STORM3DV2_MAX_ACTIVELIGHTS;i++) tlights[i]=NULL;

	// No mesh, no lighting
	IStorm3D_Mesh *mesh=obj->GetMesh();
	if (mesh==NULL)
	{
		Storm3D2->lighthandler.SetActiveLightAmount(0);
		return;
	}

	// Loop through each light in set
	for (set<IStorm3D_Light*>::iterator it=lights.begin();it!=lights.end();it++)
	{
		// Typecast (to simplify code)
		IStorm3D_Light *lgt=(*it);

		// Calculate points for light
		int pts=0;
		COL col=lgt->GetColor();
		float colp=(col.r+col.g+col.b)*lgt->GetMultiplier();
		if (lgt->GetLightType()==IStorm3D_Light::LTYPE_POINT)
		{
			Storm3D_Light_Point *pl=(Storm3D_Light_Point*)lgt;

			float ran=obj->GetGlobalPosition().GetRangeTo(pl->GetGlobalPosition());
			if (ran-mesh->GetRadius()>10*pl->decay) continue;	// Out of range?
			float a=pl->decay/ran;
			pts=(int)(colp*100.0f*a);
		}
		else if (lgt->GetLightType()==IStorm3D_Light::LTYPE_SPOT)
		{
			Storm3D_Light_Spot *sl=(Storm3D_Light_Spot*)lgt;
			
			float ran=obj->GetGlobalPosition().GetRangeTo(sl->GetGlobalPosition());
			if (ran-mesh->GetRadius()>10*sl->decay) continue;	// Out of range?
			float a=sl->decay/ran;
			pts=(int)(colp*100.0f*a);
		}
		else if (lgt->GetLightType()==IStorm3D_Light::LTYPE_DIRECTIONAL)
		{
			pts=(int)(100.0f*colp);
		}

		// If the light is at range
		//if (pts>0)
		{
			// Is the array full yet?
			if (light_num<STORM3DV2_MAX_ACTIVELIGHTS)
			{
				if (pts<lowest_points) lowest_points=pts;
				tlights[light_num]=lgt;
				points[light_num]=pts;
				light_num++;
			}
			else	// Look if the light got more points than the lowest in the array
			if (pts>lowest_points)
			{
				// If is has:
				// 1. Overwrite the one with lowest points with this
				for (int lp=0;(lp<(STORM3DV2_MAX_ACTIVELIGHTS-1))&&(points[lp]==lowest_points);lp++); //OK
				tlights[lp]=lgt;
				points[lp]=pts;

				// 2. Update lowest points number
				lowest_points=pts;
				for (lp=0;lp<STORM3DV2_MAX_ACTIVELIGHTS;lp++)
					if (points[lp]<lowest_points) lowest_points=points[lp];
			}
		}		
	}

	// Apply all lights
	for (int ln=0;ln<light_num;ln++)
	{
		switch (tlights[ln]->GetLightType())
		{
			case IStorm3D_Light::LTYPE_POINT:
				((Storm3D_Light_Point*)tlights[ln])->Apply(ln);
				break;

			case IStorm3D_Light::LTYPE_SPOT:
				((Storm3D_Light_Spot*)tlights[ln])->Apply(ln);
				break;
			
			case IStorm3D_Light::LTYPE_DIRECTIONAL:
				((Storm3D_Light_Directional*)tlights[ln])->Apply(ln);
				break;
		}
	}

	// Tell Storm3D to use correct number of lights
	Storm3D2->lighthandler.SetActiveLightAmount(light_num);
	
}



//------------------------------------------------------------------
// Storm3D_Scene_LightHandler::RenderLensFlares
//------------------------------------------------------------------
void Storm3D_Scene_LightHandler::RenderLensFlares(Storm3D_Scene *scene)
{	
	// RStates on
	//Storm3D2->D3DDevice->SetRenderState(D3DRS_ZENABLE,FALSE);
	Storm3D2->D3DDevice->SetRenderState(D3DRS_ZENABLE,TRUE);
	Storm3D2->D3DDevice->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);

	// Loop through each light in set
	for (set<IStorm3D_Light*>::iterator it=lights.begin();it!=lights.end();it++)
	{
		// Typecast (to simplify code)
		IStorm3D_Light *lgt=(*it);

		// Skip directional lights
		if (lgt->GetLightType()==IStorm3D_Light::LTYPE_DIRECTIONAL) continue;

		// Typecasts (to simplify code)
		IStorm3D_Light_Point *pl=(IStorm3D_Light_Point*)lgt;
		Storm3D_LensFlare *lfl=(Storm3D_LensFlare*)pl->GetLensFlare();

		// If has lensflare: render it
		if (lfl)
		{
			// Calculate position on screen
			VC3 scpos;
			float w,rz;

			// Calculate position, and render only if flare is visible
			if ((scene->camera.GetTransformedToScreen(pl->GetGlobalPosition(),scpos,w,rz))
				&&(rz<scene->camera.vis_range))
			// BETA: here should be x/y visibility test
			{
				// Flare vis-multiplier
				//float sizemul=0;

				//Storm3D_CollisionInfo cinf;
				/*
				VC3 dir=pl->GetGlobalPosition()-scene->camera.GetPosition();
				float len=dir.GetLength();
				dir*=1/len;
				scene->RayTrace(scene->camera.GetPosition(),dir,len,cinf);
				*/
				
				// Test visibility (visbuffer)
				/*if (cinf.hit)
				{
					// Flare visibility (fades out)
					if (pl->GetLightType()==IStorm3D_Light::LTYPE_SPOT)
					{
						((Storm3D_Light_Spot*)pl)->flarevis-=scene->time_dif/100.0f;
						if (((Storm3D_Light_Spot*)pl)->flarevis<0)
							((Storm3D_Light_Spot*)pl)->flarevis=0;
						sizemul=((Storm3D_Light_Spot*)pl)->flarevis;
					}
					else
					{
						((Storm3D_Light_Point*)pl)->flarevis-=scene->time_dif/100.0f;
						if (((Storm3D_Light_Point*)pl)->flarevis<0)
							((Storm3D_Light_Point*)pl)->flarevis=0;
						sizemul=((Storm3D_Light_Point*)pl)->flarevis;
					}
				}
				else
				{
					// Flare visibility (fades in)
					if (pl->GetLightType()==IStorm3D_Light::LTYPE_SPOT)
					{
						((Storm3D_Light_Spot*)pl)->flarevis+=scene->time_dif/100.0f;
						if (((Storm3D_Light_Spot*)pl)->flarevis>1) 
							((Storm3D_Light_Spot*)pl)->flarevis=1;
						sizemul=((Storm3D_Light_Spot*)pl)->flarevis;
					}
					else
					{
						((Storm3D_Light_Point*)pl)->flarevis+=scene->time_dif/100.0f;
						if (((Storm3D_Light_Point*)pl)->flarevis>1) 
							((Storm3D_Light_Point*)pl)->flarevis=1;
						sizemul=((Storm3D_Light_Point*)pl)->flarevis;
					}
				}*/

				// Do not render invisible flares
				//if (sizemul<0.01f) continue;

				// Get viewport size
				Storm3D_SurfaceInfo ss=Storm3D2->GetScreenSize();

				// Create 3d-vector
				VC3 pos=VC3(scpos.x*ss.width,scpos.y*ss.height,scpos.z);

				// Render glow
				if (lfl->tex_glow)
				{
					// Calculate size
					//float hsizex=(lfl->glow_size/rz)*sizemul*width;
					//float hsizey=(lfl->glow_size/rz)*sizemul*height;
					float hsizex=(lfl->glow_size/rz)*ss.width;
					float hsizey=(lfl->glow_size/rz)*ss.height;

					// Create color (color+alpha)
					DWORD col=lgt->GetColor().GetAsD3DCompatibleARGB();

					// Apply the texture
					lfl->tex_glow->Apply(0);

					// Create a quad
					VXFORMAT_2D vx[4];
					vx[0]=VXFORMAT_2D(pos+VC3(-hsizex,hsizey,0),1,col,VC2(0,1));
					vx[1]=VXFORMAT_2D(pos+VC3(-hsizex,-hsizey,0),1,col,VC2(0,0));
					vx[2]=VXFORMAT_2D(pos+VC3(hsizex,hsizey,0),1,col,VC2(1,1));
					vx[3]=VXFORMAT_2D(pos+VC3(hsizex,-hsizey,0),1,col,VC2(1,0));

					// Clip
					if (Clip2DRectangle(Storm3D2,vx[1],vx[2])) 
					{
						// Copy clipping
						vx[0].position.x=vx[1].position.x;
						vx[0].texcoords.x=vx[1].texcoords.x;
						vx[3].position.y=vx[1].position.y;
						vx[3].texcoords.y=vx[1].texcoords.y;
						vx[0].position.y=vx[2].position.y;
						vx[0].texcoords.y=vx[2].texcoords.y;
						vx[3].position.x=vx[2].position.x;
						vx[3].texcoords.x=vx[2].texcoords.x;

						// Render it
						Storm3D2->D3DDevice->SetVertexShader(FVF_VXFORMAT_2D);
						Storm3D2->D3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,vx,sizeof(VXFORMAT_2D));
						scene->AddPolyCounter(2);
					}
				}

				// Calc position (origin transformed to center of screen)
				//VC3 gp=pos-VC3(width*0.5f,height*0.5f,0);

				// Render ring
				/*if (lfl->tex_ring)
				{
					pos=gp*(0.6f)+VC3(width*0.5f,height*0.5f,0);

					// Calculate size
					float hsizex=(lfl->ring_size/rz)*sizemul*width;
					float hsizey=(lfl->ring_size/rz)*sizemul*height;

					// Create color (color+alpha)
					DWORD col=(lgt->GetColor()*0.5f).GetAsD3DCompatibleARGB();

					// Apply the texture
					lfl->tex_ring->Apply(0);

					// Create a quad
					VXFORMAT_2D vx[4];
					vx[0]=VXFORMAT_2D(pos+VC3(-hsizex,hsizey,0),1,col,VC2(0,1));
					vx[1]=VXFORMAT_2D(pos+VC3(-hsizex,-hsizey,0),1,col,VC2(0,0));
					vx[2]=VXFORMAT_2D(pos+VC3(hsizex,hsizey,0),1,col,VC2(1,1));
					vx[3]=VXFORMAT_2D(pos+VC3(hsizex,-hsizey,0),1,col,VC2(1,0));

					// Clip
					if (Clip2DRectangle(Storm3D2,vx[1],vx[2])) 
					{
						// Copy clipping
						vx[0].position.x=vx[1].position.x;
						vx[0].texcoords.x=vx[1].texcoords.x;
						vx[3].position.y=vx[1].position.y;
						vx[3].texcoords.y=vx[1].texcoords.y;
						vx[0].position.y=vx[2].position.y;
						vx[0].texcoords.y=vx[2].texcoords.y;
						vx[3].position.x=vx[2].position.x;
						vx[3].texcoords.x=vx[2].texcoords.x;

						// Render it
						Storm3D2->D3DDevice->SetVertexShader(FVF_VXFORMAT_2D);
						Storm3D2->D3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,vx,sizeof(VXFORMAT_2D));	
					}
				}*/

					// Renderoidaan ympyrät (pienet)
					/*if (lflare[dxlight_handle[lg].owner->lensflare].pic_circle>=0)
					for (float f=-2.3f;f<2.5f;f+=0.7f)
					{
						sx=0.5f+(gxp*f);
						sy=0.5f+(gyp*f);
						float xsiz=xl*(fabsf(f)*0.2f+0.1f);
						float ysiz=yl*(fabsf(f)*0.2f+0.1f);
						RenderTexture(lflare[dxlight_handle[lg].owner->lensflare].pic_circle,sx-(xsiz/2.0f),sy-(ysiz/2.0f),-1,xsiz,ysiz,
							dxlight_handle[lg].owner->r/4.0f,dxlight_handle[lg].owner->g/4.0f,dxlight_handle[lg].owner->b/4.0f,0,0,1,1,0,ST3D_ALPHA_ADD);
					}*/

			}
			/*else
			{
				// Flare visibility (fades out)
				if (pl->GetLightType()==IStorm3D_Light::LTYPE_SPOT)
				{
					((Storm3D_Light_Spot*)pl)->flarevis-=scene->time_dif/100.0f;
					if (((Storm3D_Light_Spot*)pl)->flarevis<0)
						((Storm3D_Light_Spot*)pl)->flarevis=0;
				}
				else
				{
					((Storm3D_Light_Point*)pl)->flarevis-=scene->time_dif/100.0f;
					if (((Storm3D_Light_Point*)pl)->flarevis<0)
						((Storm3D_Light_Point*)pl)->flarevis=0;
				}
			}*/
		}
	}

	// RStates off
	Storm3D2->D3DDevice->SetRenderState(D3DRS_ZENABLE,TRUE);
}


