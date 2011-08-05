/*

  Storm3D v2.0 T&L Graphics Engine
  (C) Sebastian Aaltonen 2000

  Class: Storm3D_Texture

  Texture (materials use)

*/



//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>
#include "storm3d_scene.h"
#include "Storm3D_SystemSurface.h"



//------------------------------------------------------------------
// Storm3D_SystemSurface::Storm3D_SystemSurface
//------------------------------------------------------------------
Storm3D_SystemSurface::Storm3D_SystemSurface(int _size) :
	size(_size),
	sizesq(size*size)
{
	// Alloc data
	data=new byte[sizesq];

	// Calc shift
	if (size==1) shift=0;
	else if (size==2) shift=1;
	else if (size==4) shift=2;
	else if (size==8) shift=3;
	else if (size==16) shift=4;
	else if (size==32) shift=5;
	else if (size==64) shift=6;
	else if (size==128) shift=7;
	else if (size==256) shift=8;
	else if (size==512) shift=9;
	else if (size==1024) shift=10;
	else if (size==2048) shift=11;
}


Storm3D_SystemSurface::~Storm3D_SystemSurface()
{
	// Free data
	delete[] data;
}



//------------------------------------------------------------------
// Storm3D_SystemSurface - Misc
//------------------------------------------------------------------
void Storm3D_SystemSurface::Clear()
{
	memset(data,255,sizesq);
}


void Storm3D_SystemSurface::Fix()
{
/*	int yp=0;
	int x;
	for (int y=0;y<size;y++)
	{
		for (x=0;x<size;x++)
		{
			if (data[x+yp]!=255) break;
			data[x+yp]=0;
		}
		for (x=size-1;x>=0;x--)
		{
			if (data[x+yp]!=255) break;
			data[x+yp]=0;
		}
		yp+=size;
	}
	*/
	for (int i=0;i<sizesq;i++)
		if (data[i]==255) data[i]=20;
}


BYTE *Storm3D_SystemSurface::GetDataStart()
{
	return data;
}



//------------------------------------------------------------------
// Storm3D_SystemSurface - Rendering
//------------------------------------------------------------------
struct INTVERT
{
	int x,y;
	int z;
};


struct INTFACE
{
	INTVERT vxs[3];
};


struct TVERT
{
	//float x,y,z;
	VC3 pos;
	float irhw;		// Optimization (need only be recalculated to clipped vertexes)
};


void Storm3D_SystemSurface::RenderFaceList(Storm3D_Scene *scene,D3DMATRIX worldmx,Storm3D_Face *faces,Storm3D_Vertex *vertexes,int num_faces,int num_vertexes)
{
	// Calc size/2
	float hsx=size/2;
	float hsy=size/2;

	// Calc matrix
	D3DMATRIX mt=(D3DXMATRIX )worldmx*(D3DXMATRIX)scene->camera.GetVP();
	float *invVP=(float*)&mt;

	// Precalc 255/visrange
	float icam255=255.0f/scene->camera.vis_range;

	// Transform vertexes (do not calc. to perspective yet)
	TVERT *tverts=new TVERT[num_vertexes];
	for (int vx=0;vx<num_vertexes;vx++)
	{
		float x=vertexes[vx].position.x;
		float y=vertexes[vx].position.y;
		float z=vertexes[vx].position.z;
		tverts[vx].pos.x=x*invVP[0]+y*invVP[4]+z*invVP[8]+invVP[12];
		tverts[vx].pos.y=x*invVP[1]+y*invVP[5]+z*invVP[9]+invVP[13];
		tverts[vx].pos.z=x*invVP[2]+y*invVP[6]+z*invVP[10]+invVP[14];
		tverts[vx].irhw=1.0f/(x*invVP[3]+y*invVP[7]+z*invVP[11]+invVP[15]);
	}

	// Loop through all faces
	for (int fc=0;fc<num_faces;fc++)
	{
		// Get vertex indexes
		int vx0=faces[fc].vertex_index[0];
		int vx1=faces[fc].vertex_index[1];
		int vx2=faces[fc].vertex_index[2];

		// Created faces
		INTFACE newfaces[2];
		int fcamt=0;

		// Clip face
		float r0=tverts[vx0].pos.z-1;
		float r1=tverts[vx1].pos.z-1;
		float r2=tverts[vx2].pos.z-1;
		if (r0<0)
		{
			if (r1<0)
			{
				if (r2<0)
				{
					// Skip face because it's completely behind camera
					continue;
				}
				else // >0
				{
					// Do 1 face (r2 visible)
					VC3 e20=(tverts[vx0].pos-tverts[vx2].pos)*(r2/(r2-r0));
					VC3 e21=(tverts[vx1].pos-tverts[vx2].pos)*(r2/(r2-r1));

					newfaces[fcamt].vxs[0].x=hsx+(tverts[vx2].pos.x*tverts[vx2].irhw*hsx);
					newfaces[fcamt].vxs[0].y=hsy-(tverts[vx2].pos.y*tverts[vx2].irhw*hsy);
					newfaces[fcamt].vxs[0].z=tverts[vx2].pos.z*icam255;
					
					VC3 vxp=tverts[vx2].pos+e20;
					newfaces[fcamt].vxs[1].x=hsx+(vxp.x*hsx);
					newfaces[fcamt].vxs[1].y=hsy-(vxp.y*hsy);
					newfaces[fcamt].vxs[1].z=icam255;
					
					vxp=tverts[vx2].pos+e21;
					newfaces[fcamt].vxs[2].x=hsx+(vxp.x*hsx);
					newfaces[fcamt].vxs[2].y=hsy-(vxp.y*hsy);
					newfaces[fcamt].vxs[2].z=icam255;
					
					fcamt++;
				}
			}
			else // >0
			{
				if (r2<0)
				{
					// Do 1 face (r1 visible)
					VC3 e12=(tverts[vx2].pos-tverts[vx1].pos)*(r1/(r1-r2));
					VC3 e10=(tverts[vx0].pos-tverts[vx1].pos)*(r1/(r1-r0));

					newfaces[fcamt].vxs[0].x=hsx+(tverts[vx1].pos.x*tverts[vx1].irhw*hsx);
					newfaces[fcamt].vxs[0].y=hsy-(tverts[vx1].pos.y*tverts[vx1].irhw*hsy);
					newfaces[fcamt].vxs[0].z=tverts[vx1].pos.z*icam255;
					
					VC3 vxp=tverts[vx1].pos+e12;
					newfaces[fcamt].vxs[1].x=hsx+(vxp.x*hsx);
					newfaces[fcamt].vxs[1].y=hsy-(vxp.y*hsy);
					newfaces[fcamt].vxs[1].z=icam255;
					
					vxp=tverts[vx1].pos+e10;
					newfaces[fcamt].vxs[2].x=hsx+(vxp.x*hsx);
					newfaces[fcamt].vxs[2].y=hsy-(vxp.y*hsy);
					newfaces[fcamt].vxs[2].z=icam255;
					
					fcamt++;
				}
				else // >0
				{
					// Do 2 faces (r0 invisible)
					VC3 e10=(tverts[vx0].pos-tverts[vx1].pos)*(r1/(r1-r0));
					VC3 e20=(tverts[vx0].pos-tverts[vx2].pos)*(r2/(r2-r0));

					// Face1
					newfaces[fcamt].vxs[0].x=hsx+(tverts[vx1].pos.x*tverts[vx1].irhw*hsx);
					newfaces[fcamt].vxs[0].y=hsy-(tverts[vx1].pos.y*tverts[vx1].irhw*hsy);
					newfaces[fcamt].vxs[0].z=tverts[vx1].pos.z*icam255;

					VC3 vxp=tverts[vx2].pos+e20;
					newfaces[fcamt].vxs[1].x=hsx+(vxp.x*hsx);
					newfaces[fcamt].vxs[1].y=hsy-(vxp.y*hsy);
					newfaces[fcamt].vxs[1].z=icam255;
					
					vxp=tverts[vx1].pos+e10;
					newfaces[fcamt].vxs[2].x=hsx+(vxp.x*hsx);
					newfaces[fcamt].vxs[2].y=hsy-(vxp.y*hsy);
					newfaces[fcamt].vxs[2].z=icam255;
					
					fcamt++;

					// Face2
					newfaces[fcamt].vxs[0].x=hsx+(tverts[vx1].pos.x*tverts[vx1].irhw*hsx);
					newfaces[fcamt].vxs[0].y=hsy-(tverts[vx1].pos.y*tverts[vx1].irhw*hsy);
					newfaces[fcamt].vxs[0].z=tverts[vx1].pos.z*icam255;

					newfaces[fcamt].vxs[1].x=hsx+(tverts[vx2].pos.x*tverts[vx2].irhw*hsx);
					newfaces[fcamt].vxs[1].y=hsy-(tverts[vx2].pos.y*tverts[vx2].irhw*hsy);
					newfaces[fcamt].vxs[1].z=tverts[vx2].pos.z*icam255;
					
					vxp=tverts[vx2].pos+e20;
					newfaces[fcamt].vxs[2].x=hsx+(vxp.x*hsx);
					newfaces[fcamt].vxs[2].y=hsy-(vxp.y*hsy);
					newfaces[fcamt].vxs[2].z=icam255;
					
					fcamt++;
				}
			}
		}
		else // >0
		{
			if (r1<0)
			{
				if (r2<0)
				{					
					// Do 1 face (r0 visible)
					VC3 e01=(tverts[vx1].pos-tverts[vx0].pos)*(r0/(r0-r1));
					VC3 e02=(tverts[vx2].pos-tverts[vx0].pos)*(r0/(r0-r2));

					newfaces[fcamt].vxs[0].x=hsx+(tverts[vx0].pos.x*tverts[vx0].irhw*hsx);
					newfaces[fcamt].vxs[0].y=hsy-(tverts[vx0].pos.y*tverts[vx0].irhw*hsy);
					newfaces[fcamt].vxs[0].z=tverts[vx0].pos.z*icam255;

					VC3 vxp=tverts[vx0].pos+e01;
					newfaces[fcamt].vxs[1].x=hsx+(vxp.x*hsx);
					newfaces[fcamt].vxs[1].y=hsy-(vxp.y*hsy);
					newfaces[fcamt].vxs[1].z=icam255;
					
					vxp=tverts[vx0].pos+e02;
					newfaces[fcamt].vxs[2].x=hsx+(vxp.x*hsx);
					newfaces[fcamt].vxs[2].y=hsy-(vxp.y*hsy);
					newfaces[fcamt].vxs[2].z=icam255;
					
					fcamt++;
				}
				else // >0
				{
					// Do 2 faces (r1 invisible)
					VC3 e21=(tverts[vx1].pos-tverts[vx2].pos)*(r2/(r2-r1));
					VC3 e01=(tverts[vx1].pos-tverts[vx0].pos)*(r0/(r0-r1));

					// Face1
					newfaces[fcamt].vxs[0].x=hsx+(tverts[vx2].pos.x*tverts[vx2].irhw*hsx);
					newfaces[fcamt].vxs[0].y=hsy-(tverts[vx2].pos.y*tverts[vx2].irhw*hsy);
					newfaces[fcamt].vxs[0].z=tverts[vx2].pos.z*icam255;

					VC3 vxp=tverts[vx0].pos+e01;
					newfaces[fcamt].vxs[1].x=hsx+(vxp.x*hsx);
					newfaces[fcamt].vxs[1].y=hsy-(vxp.y*hsy);
					newfaces[fcamt].vxs[1].z=icam255;
					
					vxp=tverts[vx2].pos+e21;
					newfaces[fcamt].vxs[2].x=hsx+(vxp.x*hsx);
					newfaces[fcamt].vxs[2].y=hsy-(vxp.y*hsy);
					newfaces[fcamt].vxs[2].z=icam255;
					
					fcamt++;

					// Face2
					newfaces[fcamt].vxs[0].x=hsx+(tverts[vx2].pos.x*tverts[vx2].irhw*hsx);
					newfaces[fcamt].vxs[0].y=hsy-(tverts[vx2].pos.y*tverts[vx2].irhw*hsy);
					newfaces[fcamt].vxs[0].z=tverts[vx2].pos.z*icam255;

					newfaces[fcamt].vxs[1].x=hsx+(tverts[vx0].pos.x*tverts[vx0].irhw*hsx);
					newfaces[fcamt].vxs[1].y=hsy-(tverts[vx0].pos.y*tverts[vx0].irhw*hsy);
					newfaces[fcamt].vxs[1].z=tverts[vx0].pos.z*icam255;
					
					vxp=tverts[vx0].pos+e01;
					newfaces[fcamt].vxs[2].x=hsx+(vxp.x*hsx);
					newfaces[fcamt].vxs[2].y=hsy-(vxp.y*hsy);
					newfaces[fcamt].vxs[2].z=icam255;
					
					fcamt++;
				}
			}
			else // >0
			{
				if (r2<0)
				{
					// Do 2 faces (r2 invisible)
					VC3 e02=(tverts[vx2].pos-tverts[vx0].pos)*(r0/(r0-r2));
					VC3 e12=(tverts[vx2].pos-tverts[vx1].pos)*(r1/(r1-r2));

					// Face1
					newfaces[fcamt].vxs[0].x=hsx+(tverts[vx0].pos.x*tverts[vx0].irhw*hsx);
					newfaces[fcamt].vxs[0].y=hsy-(tverts[vx0].pos.y*tverts[vx0].irhw*hsy);
					newfaces[fcamt].vxs[0].z=tverts[vx0].pos.z*icam255;

					VC3 vxp=tverts[vx0].pos+e02;
					newfaces[fcamt].vxs[1].x=hsx+(vxp.x*hsx);
					newfaces[fcamt].vxs[1].y=hsy-(vxp.y*hsy);
					newfaces[fcamt].vxs[1].z=icam255;
					
					vxp=tverts[vx1].pos+e12;
					newfaces[fcamt].vxs[2].x=hsx+(vxp.x*hsx);
					newfaces[fcamt].vxs[2].y=hsy-(vxp.y*hsy);
					newfaces[fcamt].vxs[2].z=icam255;
					
					fcamt++;

					// Face2
					newfaces[fcamt].vxs[0].x=hsx+(tverts[vx0].pos.x*tverts[vx0].irhw*hsx);
					newfaces[fcamt].vxs[0].y=hsy-(tverts[vx0].pos.y*tverts[vx0].irhw*hsy);
					newfaces[fcamt].vxs[0].z=tverts[vx0].pos.z*icam255;

					newfaces[fcamt].vxs[1].x=hsx+(tverts[vx1].pos.x*tverts[vx1].irhw*hsx);
					newfaces[fcamt].vxs[1].y=hsy-(tverts[vx1].pos.y*tverts[vx1].irhw*hsy);
					newfaces[fcamt].vxs[1].z=tverts[vx1].pos.z*icam255;
					
					vxp=tverts[vx1].pos+e12;
					newfaces[fcamt].vxs[2].x=hsx+(vxp.x*hsx);
					newfaces[fcamt].vxs[2].y=hsy-(vxp.y*hsy);
					newfaces[fcamt].vxs[2].z=icam255;
					
					fcamt++;
				}
				else // >0
				{
					// Do 1 face (just copy, no clipping)
					newfaces[fcamt].vxs[0].x=hsx+(tverts[vx0].pos.x*tverts[vx0].irhw*hsx);
					newfaces[fcamt].vxs[0].y=hsy-(tverts[vx0].pos.y*tverts[vx0].irhw*hsy);
					newfaces[fcamt].vxs[0].z=tverts[vx0].pos.z*icam255;
					newfaces[fcamt].vxs[1].x=hsx+(tverts[vx1].pos.x*tverts[vx1].irhw*hsx);
					newfaces[fcamt].vxs[1].y=hsy-(tverts[vx1].pos.y*tverts[vx1].irhw*hsy);
					newfaces[fcamt].vxs[1].z=tverts[vx1].pos.z*icam255;
					newfaces[fcamt].vxs[2].x=hsx+(tverts[vx2].pos.x*tverts[vx2].irhw*hsx);
					newfaces[fcamt].vxs[2].y=hsy-(tverts[vx2].pos.y*tverts[vx2].irhw*hsy);
					newfaces[fcamt].vxs[2].z=tverts[vx2].pos.z*icam255;
					fcamt++;
				}
			}
		}
		

		// Loop through all clipped faces
		for (int fcc=0;fcc<fcamt;fcc++)
		{
			// Sort vertexes
			int vhi,vmd,vlo;
			if (newfaces[fcc].vxs[1].y<newfaces[fcc].vxs[0].y) {vhi=1;vlo=0;} else {vhi=0;vlo=1;}
			if (newfaces[fcc].vxs[2].y<newfaces[fcc].vxs[vhi].y) {vmd=vhi;vhi=2;} else
			{
				if (newfaces[fcc].vxs[vlo].y<newfaces[fcc].vxs[2].y) {vmd=vlo;vlo=2;} else vmd=2;
			}

			// Create edges
			int yd=(newfaces[fcc].vxs[vlo].y-newfaces[fcc].vxs[vhi].y);
			if (yd==0)
			{
				// "line", in X (DRAW)
				/*int xs=(ex1>>16);
				int xe=(ex2>>16);
				int ce=ez1;
				int zp=xe-xs;
				if (zp!=0)
				{
					zp=(ez2-ez1)/zp;
					if (xs<0) {ce+=zp*xs;xs=0;}
					if (xe>=size) xe=size-1;
				
					// Render it!
					if ((xe>0)&&(xs<size))
					for (int xx=xs;xx<xe;xx++)
					{
						int z=(ce>>16);
						if (data[yp+xx]>z) data[yp+xx]=z;
						ce+=zp;
					}
				}*/
				continue;	
			}
			int ex_hi_lo=((newfaces[fcc].vxs[vlo].x-newfaces[fcc].vxs[vhi].x)<<16)/yd;
			int ez_hi_lo=((newfaces[fcc].vxs[vlo].z-newfaces[fcc].vxs[vhi].z)<<16)/yd;
	
			yd=(newfaces[fcc].vxs[vmd].y-newfaces[fcc].vxs[vhi].y);
			if (yd==0) yd=1;
			int ex_hi_md=((newfaces[fcc].vxs[vmd].x-newfaces[fcc].vxs[vhi].x)<<16)/yd;
			int ez_hi_md=((newfaces[fcc].vxs[vmd].z-newfaces[fcc].vxs[vhi].z)<<16)/yd;

			yd=(newfaces[fcc].vxs[vlo].y-newfaces[fcc].vxs[vmd].y);
			if (yd==0) yd=1;
			int ex_md_lo=((newfaces[fcc].vxs[vlo].x-newfaces[fcc].vxs[vmd].x)<<16)/yd;
			int ez_md_lo=((newfaces[fcc].vxs[vlo].z-newfaces[fcc].vxs[vmd].z)<<16)/yd;


			// Confirm that scanline is in right order
			if (ex_hi_lo==ex_hi_md)
			{
				// Line in Y (DRAW)
				/*int ez1=tr_verts[vhi].z<<16;
				int yxp=size*tr_verts[vhi].y+tr_verts[vhi].x;
				for (int yy=tr_verts[vhi].y;yy<tr_verts[vlo].y;yy++)
				{
					int z=(ez1>>16);
					if (data[yxp]>z) data[yxp]=z;
					ez1+=ez_hi_lo;
					yxp+=size;
				}*/
			}
			else
			if (ex_hi_lo<ex_hi_md)
			{
				// Render up-md
				int ex1=newfaces[fcc].vxs[vhi].x<<16;
				int ez1=newfaces[fcc].vxs[vhi].z<<16;
				int ex2=ex1;
				int ez2=ez1;
				int yp=size*newfaces[fcc].vxs[vhi].y;
				int yend=min(newfaces[fcc].vxs[vmd].y,size);
				int ystart=newfaces[fcc].vxs[vhi].y;
				//if (ystart<-5000) abort();	// BETA!!
				if (ystart<0)
				{
					int ym=min(-ystart,yend-ystart);
					ex1+=ex_hi_lo*ym;
					ez1+=ez_hi_lo*ym;
					ex2+=ex_hi_md*ym;
					ez2+=ez_hi_md*ym;
					yp+=size*ym;
					ystart=0;
				}
				for (int yy=ystart;yy<yend;yy++)
				{
					// Clip Y (end) 
					//if (yy>=size) break;

					// Clip Y (beginning)
					/*if (yy>=0)
					{*/

						// Prepare rendering scanline
						int xs=(ex1>>16);
						int xe=(ex2>>16);
						int ce=ez1;
						int zp=xe-xs;
						if (zp!=0)
						{
							zp=(ez2-ez1)/zp;
							if (xs<0) {ce+=zp*(-xs);xs=0;}
							if (xe>size-1) xe=size-1;
				
							// Render it!
							if ((xe>0)&&(xs<size))
							for (int xx=xs;xx<=xe;xx++)
							{
								int z=(ce>>16);
								if (data[yp+xx]>z) data[yp+xx]=z;
								ce+=zp;
							}
						}
					//}

					// Add values
					ex1+=ex_hi_lo;
					ez1+=ez_hi_lo;
					ex2+=ex_hi_md;
					ez2+=ez_hi_md;
					yp+=size;
				}

				// Render md-lo
				ex2=newfaces[fcc].vxs[vmd].x<<16;
				ez2=newfaces[fcc].vxs[vmd].z<<16;
				yp=size*newfaces[fcc].vxs[vmd].y;
				yend=min(newfaces[fcc].vxs[vlo].y,size-1);
				ystart=newfaces[fcc].vxs[vmd].y;
				//if (ystart<-5000) abort();	// BETA!!
				if (ystart<0)
				{
					int ym=min(-ystart,yend-ystart);
					ex1+=ex_hi_lo*ym;
					ez1+=ez_hi_lo*ym;
					ex2+=ex_md_lo*ym;
					ez2+=ez_md_lo*ym;
					yp+=size*ym;
					ystart=0;
				}
				for (yy=ystart;yy<=yend;yy++)
				{
					// Clip Y (end) 
					//if (yy>=size) break;

					// Clip Y (beginning)
					/*if (yy>=0)
					{*/

						// Prepare rendering scanline
						int xs=(ex1>>16);
						int xe=(ex2>>16);
						int ce=ez1;
						int zp=xe-xs;
						if (zp!=0)
						{
							zp=(ez2-ez1)/zp;
							if (xs<0) {ce+=zp*(-xs);xs=0;}
							if (xe>size-1) xe=size-1;
						
							// Render it!					
							if ((xe>0)&&(xs<size))
							for (int xx=xs;xx<=xe;xx++)
							{
								BYTE z=(ce>>16);
								if (data[yp+xx]>z) data[yp+xx]=z;
								ce+=zp;
							}
						}
					//}

					// Add values
					ex1+=ex_hi_lo;
					ez1+=ez_hi_lo;
					ex2+=ex_md_lo;
					ez2+=ez_md_lo;
					yp+=size;
				}
			}
			else	// Other order
			{
				// Render up-md
				int ex1=newfaces[fcc].vxs[vhi].x<<16;
				int ez1=newfaces[fcc].vxs[vhi].z<<16;
				int ex2=ex1;
				int ez2=ez1;
				int yp=size*newfaces[fcc].vxs[vhi].y;
				int yend=min(newfaces[fcc].vxs[vmd].y,size);
				int ystart=newfaces[fcc].vxs[vhi].y;
				//if (ystart<-5000) abort();	// BETA!!
				if (ystart<0)
				{
					int ym=min(-ystart,yend-ystart);
					ex1+=ex_hi_lo*ym;
					ez1+=ez_hi_lo*ym;
					ex2+=ex_hi_md*ym;
					ez2+=ez_hi_md*ym;
					yp+=size*ym;
					ystart=0;
				}
				for (int yy=ystart;yy<yend;yy++)
				{
					// Clip Y (end) 
					//if (yy>=size) break;

					// Clip Y (beginning)
					/*if (yy>=0)
					{*/

						// Prepare rendering scanline
						int xs=(ex2>>16);
						int xe=(ex1>>16);
						int ce=ez2;
						int zp=xe-xs;
						if (zp!=0)
						{	
							zp=(ez1-ez2)/zp;
							if (xs<0) {ce+=zp*(-xs);xs=0;}
							if (xe>size-1) xe=size-1;
						
							// Render it!					
							if ((xe>0)&&(xs<size))
							for (int xx=xs;xx<=xe;xx++)
							{
								int z=(ce>>16);
								if (data[yp+xx]>z) data[yp+xx]=z;
								ce+=zp;
							}
						}
					//}

					// Add values
					ex1+=ex_hi_lo;
					ez1+=ez_hi_lo;
					ex2+=ex_hi_md;
					ez2+=ez_hi_md;
					yp+=size;
				}

				// Render md-lo
				ex2=newfaces[fcc].vxs[vmd].x<<16;
				ez2=newfaces[fcc].vxs[vmd].z<<16;
				yp=size*newfaces[fcc].vxs[vmd].y;
				yend=min(newfaces[fcc].vxs[vlo].y,size-1);
				ystart=newfaces[fcc].vxs[vmd].y;
				//if (ystart<-5000) abort();	// BETA!!
				if (ystart<0)
				{
					int ym=min(-ystart,yend-ystart);
					ex1+=ex_hi_lo*ym;
					ez1+=ez_hi_lo*ym;
					ex2+=ex_md_lo*ym;
					ez2+=ez_md_lo*ym;
					yp+=size*ym;
					ystart=0;
				}
				for (yy=ystart;yy<=yend;yy++)
				{
					// Clip Y (end) 
					//if (yy>=size) break;

					// Clip Y (beginning)
					/*if (yy>=0)
					{*/

						// Prepare rendering scanline
						int xs=(ex2>>16);
						int xe=(ex1>>16);
						int ce=ez2;
						int zp=xe-xs;
						if (zp!=0)
						{
							zp=(ez1-ez2)/zp;
							if (xs<0) {ce+=zp*(-xs);xs=0;}
							if (xe>size-1) xe=size-1;				

							// Render it!					
							if ((xe>0)&&(xs<size))
							for (int xx=xs;xx<=xe;xx++)
							{
								int z=(ce>>16);
								if (data[yp+xx]>z) data[yp+xx]=z;
								ce+=zp;
							}
						}
					//}

					// Add values
					ex1+=ex_hi_lo;
					ez1+=ez_hi_lo;
					ex2+=ex_md_lo;
					ez2+=ez_md_lo;
					yp+=size;
				}
			}
		}
	}

	// Free temp arrays
	delete[] tverts;
}


