/*

  Storm3D v2.0 T&L Graphics Engine
  (C) Sebastian Aaltonen 2000

  Class: Storm3D_Mesh_LOD

	- Mesh's level of detail (LOD)

*/



//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include <Storm3DV2.h>



//------------------------------------------------------------------
// Storm3D_Mesh_LOD::Storm3D_Mesh_LOD
//------------------------------------------------------------------
Storm3D_Mesh_LOD::Storm3D_Mesh_LOD() :
	face_amount(0),
	//vertex_amount(0),
	dx8_vbuf(0),
	dx8_ibuf(0),
	faces(NULL),
	//fstrip_len(0),
	//fstrip(NULL)
	lod_vertex_amount(0),
	vx_reindex(NULL),
	vx_update_needed(true),
	fc_update_needed(true),
	vxfacelists(NULL),
	obj_vx_amount(0)
{
}



//------------------------------------------------------------------
// Storm3D_Mesh_LOD::~Storm3D_Mesh_LOD
//------------------------------------------------------------------
Storm3D_Mesh_LOD::~Storm3D_Mesh_LOD()
{
	SAFE_RELEASE(dx8_vbuf);
	SAFE_RELEASE(dx8_ibuf);
	if (faces) delete[] faces;
	if (vxfacelists)
	{
		// Delete temp lists
		for (int vx=0;vx<obj_vx_amount;vx++) SAFE_DELETE(vxfacelists[vx]);
		delete[] vxfacelists;
	}
	//if (fstrip) delete[] fstrip;
}



//------------------------------------------------------------------
// Storm3D_Mesh_LOD::SetFaceAmount
//------------------------------------------------------------------
void Storm3D_Mesh_LOD::SetFaceAmount(int num)
{
	// Free old buffer first
	//SAFE_RELEASE(dx8_ibuf);
	
	// Set new amount
	face_amount=num;

	// Create new indexbuffer
	/*Storm3D2->D3DDevice->CreateIndexBuffer(sizeof(WORD)*face_amount*3,
		D3DUSAGE_WRITEONLY,D3DFMT_INDEX16,D3DPOOL_MANAGED,&dx8_ibuf);*/

	// Create new face buffer
	if (faces) delete[] faces;
	faces=new LodFace[face_amount];

	// Set flag
	fc_update_needed=true;
	vx_update_needed=true;
}



//------------------------------------------------------------------
// Storm3D_Mesh_LOD::BuildFromFaces
//------------------------------------------------------------------
void Storm3D_Mesh_LOD::BuildFromFaces(Storm3D_Face *ofaces,int face_num)
{
	// Set face amount
	face_amount=face_num;

	// Release old buffers
	//if (dx8_ibuf) dx8_ibuf->Release();
	if (faces) delete[] faces;
	faces=new LodFace[face_amount];

	// Create new indexbuffer
	/*Storm3D2->D3DDevice->CreateIndexBuffer(sizeof(WORD)*face_amount*3,
		D3DUSAGE_WRITEONLY,D3DFMT_INDEX16,D3DPOOL_MANAGED,&dx8_ibuf);*/

	// Copy data to indexbuffer
	//WORD *ip=NULL;
	//dx8_ibuf->Lock(0,sizeof(WORD)*face_amount*3,(BYTE**)&ip,0);
	//if (ip)
	{
		for(int i=0;i<face_amount;i++)
		{
			/**ip++=*/faces[i].vertex_index[0]=ofaces[i].vertex_index[0];
			/**ip++=*/faces[i].vertex_index[1]=ofaces[i].vertex_index[1];
			/**ip++=*/faces[i].vertex_index[2]=ofaces[i].vertex_index[2];
			faces[i].normal=ofaces[i].normal;
		}
	}
	//dx8_ibuf->Unlock();

	// Set flag
	fc_update_needed=true;
	vx_update_needed=true;
}



//------------------------------------------------------------------
// Storm3D_Mesh_LOD::UpdateFaceAdjacency
//------------------------------------------------------------------
void Storm3D_Mesh_LOD::UpdateFaceAdjacency(int vertex_amount)
{
	// Delete old temp lists
	if (vxfacelists)
	{
		// Delete temp lists
		for (int vx=0;vx<obj_vx_amount;vx++) SAFE_DELETE(vxfacelists[vx]);
		delete[] vxfacelists;
	}

	// Temp lists
	obj_vx_amount=vertex_amount;
	vxfacelists=new PILL[vertex_amount];
	for (int vx=0;vx<vertex_amount;vx++) vxfacelists[vx]=NULL;

	// Faceloop: Create vertex's face lists. O(n)
	for (int fc=0;fc<face_amount;fc++)
	{
		// Get vertexes
		int v0=faces[fc].vertex_index[0];
		int v1=faces[fc].vertex_index[1];
		int v2=faces[fc].vertex_index[2];

		// Add face to vertices lists
		IntLiList *temp=vxfacelists[v0];
		vxfacelists[v0]=new IntLiList(fc);
		vxfacelists[v0]->next=temp;
		
		temp=vxfacelists[v1];
		vxfacelists[v1]=new IntLiList(fc);
		vxfacelists[v1]->next=temp;
		
		temp=vxfacelists[v2];
		vxfacelists[v2]=new IntLiList(fc);
		vxfacelists[v2]->next=temp;
	}

	// Faceloop 2: Search for face neighbours
	for (fc=0;fc<face_amount;fc++)
	{
		// Get vertexes
		int v0=faces[fc].vertex_index[0];
		int v1=faces[fc].vertex_index[1];
		int v2=faces[fc].vertex_index[2];

		// Loop for test... (edge 01)
		for (IntLiList *vf0=vxfacelists[v0];vf0!=NULL;vf0=vf0->next)
		if (vf0->data!=fc)
		{
			for (IntLiList *vf1=vxfacelists[v1];vf1!=NULL;vf1=vf1->next)
			{
				if (vf0->data==vf1->data)
				{
					// 2 vertexes shared. This is the neigthbour
					// Direction 0 (edge01)
					faces[fc].adjacency[0]=vf0->data;
					goto ohi;
				}
			}
		}
		ohi:	// Bad code... but fast :)

		// Loop for test... (edge 12)
		for (vf0=vxfacelists[v1];vf0!=NULL;vf0=vf0->next)
		if (vf0->data!=fc)
		{
			for (IntLiList *vf1=vxfacelists[v2];vf1!=NULL;vf1=vf1->next)
			{
				if (vf0->data==vf1->data)
				{
					// 2 vertexes shared. This is the neigthbour
					// Direction 1 (edge12)
					faces[fc].adjacency[1]=vf0->data;
					goto ohi2;
				}
			}
		}
		ohi2:	// Bad code... but fast :)

		// Loop for test... (edge 20)
		for (vf0=vxfacelists[v2];vf0!=NULL;vf0=vf0->next)
		if (vf0->data!=fc)
		{
			for (IntLiList *vf1=vxfacelists[v0];vf1!=NULL;vf1=vf1->next)
			{
				if (vf0->data==vf1->data)
				{
					// 2 vertexes shared. This is the neigthbour
					// Direction 2 (edge20)
					faces[fc].adjacency[2]=vf0->data;
					goto ohi3;
				}
			}
		}
		ohi3:;	// Bad code... but fast :)
	}
}



//------------------------------------------------------------------
// Storm3D_Mesh_LOD::UpdateFaceStrip
//------------------------------------------------------------------
/*void Storm3D_Mesh_LOD::UpdateFaceStrip()
{
	// Do not use now!!! (BETA)
	return;

	// Start making strip
	fstrip_len=0;

	// Alloc memory (EXTRA MUCH;)
	fstrip=new WORD[face_amount*5];

	// Used poly list
	bool *pused=new bool[face_amount];
	memset(pused,0,sizeof(bool)*face_amount);

	// Put the first face start in strip (in GOOD order)
	int curfc=0;
	pused[curfc]=true;
	if ((faces[curfc].adjacency[1]>=0)&&(pused[faces[curfc].adjacency[1]]==false))
	{
		fstrip[fstrip_len++]=faces[curfc].vertex_index[1];
		fstrip[fstrip_len++]=faces[curfc].vertex_index[2];
	}
	else
	if ((faces[curfc].adjacency[2]>=0)&&(pused[faces[curfc].adjacency[2]]==false))
	{
		fstrip[fstrip_len++]=faces[curfc].vertex_index[2];
		fstrip[fstrip_len++]=faces[curfc].vertex_index[0];
	}
	else	// edge01 or if no free adjacent faces
	{
		fstrip[fstrip_len++]=faces[curfc].vertex_index[0];
		fstrip[fstrip_len++]=faces[curfc].vertex_index[1];
	}

	// "Endless" loop. Creates strip
	while (1)
	{
		for (int vix=0;vix<3;vix++)
		{
			// First search the vertex which is not used in strip
			if ((faces[curfc].vertex_index[vix]!=fstrip[fstrip_len-2])&&
				(faces[curfc].vertex_index[vix]!=fstrip[fstrip_len-1]))
			{
				// Found!
				// Add it to the strip
				fstrip[fstrip_len++]=faces[curfc].vertex_index[vix];
				pused[curfc]=true;

				// Search for the right edge to move on...

				// Edge 0-1 ??
				if (((faces[curfc].vertex_index[0]==fstrip[fstrip_len-1])||
					(faces[curfc].vertex_index[0]==fstrip[fstrip_len-2]))&&
					((faces[curfc].vertex_index[1]==fstrip[fstrip_len-1])||
					(faces[curfc].vertex_index[1]==fstrip[fstrip_len-2])))
				{
					// Is there a face in that direction??
					if ((faces[curfc].adjacency[0]>=0)&&
						(pused[faces[curfc].adjacency[0]]==false))
					{
						// OK, here we go!
						curfc=faces[curfc].adjacency[0];
						break;
					}
					// else: Strip ends here
				}
				
				// Edge 1-2 ??
				if (((faces[curfc].vertex_index[1]==fstrip[fstrip_len-1])||
					(faces[curfc].vertex_index[1]==fstrip[fstrip_len-2]))&&
					((faces[curfc].vertex_index[2]==fstrip[fstrip_len-1])||
					(faces[curfc].vertex_index[2]==fstrip[fstrip_len-2])))
				{
					// Is there a face in that direction??
					if ((faces[curfc].adjacency[1]>=0)&&
						(pused[faces[curfc].adjacency[1]]==false))
					{
						// OK, here we go!
						curfc=faces[curfc].adjacency[1];
						break;
					}
					// else: Strip ends here
				}
				
				// Edge 2-0 ??
				if (((faces[curfc].vertex_index[2]==fstrip[fstrip_len-1])||
					(faces[curfc].vertex_index[2]==fstrip[fstrip_len-2]))&&
					((faces[curfc].vertex_index[0]==fstrip[fstrip_len-1])||
					(faces[curfc].vertex_index[0]==fstrip[fstrip_len-2])))
				{
					// Is there a face in that direction??
					if ((faces[curfc].adjacency[2]>=0)&&
						(pused[faces[curfc].adjacency[2]]==false))
					{
						// OK, here we go!
						curfc=faces[curfc].adjacency[2];
						break;
					}
					// else: Strip ends here
				}
				

				// Strip ends here... (there are breaks in other cases)

				// Do ending
				fstrip[fstrip_len++]=fstrip[fstrip_len-1];

				// Search a new unused face
				for (curfc=0;(curfc<face_amount)&&(pused[curfc]);curfc++);	//OK

				// Stripping complete??
				if (curfc>=face_amount)
				{
					fstrip_len--;	// Remove last ending
					goto all_done;
				}

				// Do start of the new strip (in GOOD order)
				pused[curfc]=true;
				if ((faces[curfc].adjacency[1]>=0)&&(pused[faces[curfc].adjacency[1]]==false))
				{
					fstrip[fstrip_len++]=faces[curfc].vertex_index[1];
					fstrip[fstrip_len++]=faces[curfc].vertex_index[1];
					fstrip[fstrip_len++]=faces[curfc].vertex_index[2];
				}
				else
				if ((faces[curfc].adjacency[2]>=0)&&(pused[faces[curfc].adjacency[2]]==false))
				{
					fstrip[fstrip_len++]=faces[curfc].vertex_index[2];
					fstrip[fstrip_len++]=faces[curfc].vertex_index[2];
					fstrip[fstrip_len++]=faces[curfc].vertex_index[0];
				}
				else	// edge01 or if no free adjacent faces
				{
					fstrip[fstrip_len++]=faces[curfc].vertex_index[0];
					fstrip[fstrip_len++]=faces[curfc].vertex_index[0];
					fstrip[fstrip_len++]=faces[curfc].vertex_index[1];
				}
				break;								
			}
		}
	}
	all_done:	// Shitty code again... ;)

	char s[100];
	sprintf(s,"%d faces, %d length strip (%d sfaces)\r\nMemeff:%d%%\r\nPolygon amount:%d%%",
		face_amount,fstrip_len,fstrip_len-2,
		(300*face_amount)/fstrip_len,(100*(fstrip_len-2))/(face_amount));
	MessageBox(NULL,s,"Stripper results",0);

	// Delete temp stuff
	delete[] pused;
}*/



//------------------------------------------------------------------
// Storm3D_Mesh_LOD::UpdateVertices
//------------------------------------------------------------------
void Storm3D_Mesh_LOD::UpdateVertices(Storm3D_Mesh *owner)
{
	// Test update need
	if (!vx_update_needed) return;
	if (face_amount<1) return;
	if (lod_vertex_amount<3) return;
	vx_update_needed=false;

		// Copy data to vertexbuffer
		BYTE *vp;
		dx8_vbuf->Lock(0,0,(BYTE**)&vp,0);
		if (vp==NULL) return;

		if (owner->material)
		{
			if (owner->vbuf_fvf==FVF_VXFORMAT_DOT3_TC1)	// DOT3 only
			{
				// Typecast (to simplify code)
				VXFORMAT_DOT3_TC1 *p=(VXFORMAT_DOT3_TC1*)vp;

				for(int i=0;i<lod_vertex_amount;i++)
				{
					int xi=vx_reindex[i];
					p[i]=VXFORMAT_DOT3_TC1(owner->vertexes[xi].position,
						0,owner->vertexes[xi].texturecoordinates);
				}
			}
			else if (owner->vbuf_fvf==FVF_VXFORMAT_TC2)	// TEX_EMBM_REF only
			{
				// Typecast (to simplify code)
				VXFORMAT_TC2 *p=(VXFORMAT_TC2*)vp;

				for(int i=0;i<lod_vertex_amount;i++)
				{
					int xi=vx_reindex[i];
					p[i]=VXFORMAT_TC2(owner->vertexes[xi].position,
						owner->vertexes[xi].normal,
						owner->vertexes[xi].texturecoordinates,
						owner->vertexes[xi].texturecoordinates);
				}
			} 
			else	// Other than DOT3 and TEX_EMBM_REF
			{
				if (owner->vbuf_fvf==FVF_VXFORMAT_TC0)
				{
					// Typecast (to simplify code)
					VXFORMAT_TC0 *p=(VXFORMAT_TC0*)vp;

					for(int i=0;i<lod_vertex_amount;i++)
					{
						int xi=vx_reindex[i];
						p[i]=VXFORMAT_TC0(owner->vertexes[xi].position,
							owner->vertexes[xi].normal);
					}
				}
				else if (owner->vbuf_fvf==FVF_VXFORMAT_TC1)
				{
					// Typecast (to simplify code)
					VXFORMAT_TC1 *p=(VXFORMAT_TC1*)vp;

					for(int i=0;i<lod_vertex_amount;i++)
					{
						int xi=vx_reindex[i];
						p[i]=VXFORMAT_TC1(owner->vertexes[xi].position,
							owner->vertexes[xi].normal,
							owner->vertexes[xi].texturecoordinates);
					}
				}
			}
		}
		else	// No material
		{
			// Typecast (to simplify code)
			VXFORMAT_TC0 *p=(VXFORMAT_TC0*)vp;

			for(int i=0;i<lod_vertex_amount;i++)
			{
				int xi=vx_reindex[i];
				p[i]=VXFORMAT_TC0(owner->vertexes[xi].position,
					owner->vertexes[xi].normal);
			}
		}

		dx8_vbuf->Unlock();
}



//------------------------------------------------------------------
// Storm3D_Mesh_LOD::UpdateDOT3LightVectors
//------------------------------------------------------------------
void Storm3D_Mesh_LOD::UpdateDOT3LightVectors(Storm3D_Mesh *owner,Storm3D_Scene_LightHandler &active_lights,Storm3D_Model_Object *object)
{
	// Test values
	if (face_amount<1) return;
	if (lod_vertex_amount<3) return;
	
	// Test if object uses DOT3
	if (!owner->material) return;
	if (owner->material->GetBumpType()!=Storm3D_Material::BUMPTYPE_DOT3) return;

	// Get first light (nearest/brightest)
	//IStorm3D_Light *lgt=*active_lights.lights.begin();
	IStorm3D_Light *lgt=object->mi_lights[0];
	if (!lgt) return;

	// Lock vertex buffer
	BYTE *dpt;
	if (FAILED(dx8_vbuf->Lock(0,0,(BYTE**)&dpt,0))) return;

	// Typecast
	VXFORMAT_DOT3_TC1 *vxp=(VXFORMAT_DOT3_TC1*)dpt;

	// Calculate light direction
	VC3 ldir_orig;
	if (lgt->GetLightType()==IStorm3D_Light::LTYPE_DIRECTIONAL)
	{
		IStorm3D_Light_Directional *lgt_d=(IStorm3D_Light_Directional*)lgt;
		ldir_orig=-lgt_d->GetGlobalDirection();
		ldir_orig.Transform(object->GetMXG().GetInverse());
	}
	else	// Point/Spot
	{
		IStorm3D_Light_Point *lgt_p=(IStorm3D_Light_Point*)lgt;
		ldir_orig=lgt_p->GetGlobalPosition();
		ldir_orig.Transform(object->GetMXG().GetInverse());
	}
	
	// Fill lightvectors only (diffuse component)
	int p9=0;	// optimization: save some multiplys
	for (int vx=0;vx<lod_vertex_amount;vx++)
	{
		// Calc vertex index
		int vix=vx_reindex[vx];

		// Calculate light direction
		VC3 ldir;
		if (lgt->GetLightType()==IStorm3D_Light::LTYPE_DIRECTIONAL)
		{	
			// Just copy (directional light stays at the same direction always)
			ldir=ldir_orig;
		}
		else // Point/Spot
		{
			// Calculate direction from vertex position and ldir_orig 
			ldir=ldir_orig-owner->vertexes[vix].position;
		}

		// Transform light direction with vertex DOT3 matrix and normalize it
		VC3 temp;
		int pr=vix*9;
		temp.x=ldir.x*owner->DOT3_VertexMatrix[pr+0]+ldir.y*owner->DOT3_VertexMatrix[pr+3]+ldir.z*owner->DOT3_VertexMatrix[pr+6];
		temp.y=ldir.x*owner->DOT3_VertexMatrix[pr+1]+ldir.y*owner->DOT3_VertexMatrix[pr+4]+ldir.z*owner->DOT3_VertexMatrix[pr+7];
		temp.z=ldir.x*owner->DOT3_VertexMatrix[pr+2]+ldir.y*owner->DOT3_VertexMatrix[pr+5]+ldir.z*owner->DOT3_VertexMatrix[pr+8];
		temp.Normalize();
		
		// Save light direction to diffuse component
		vxp->lightvector=(((int)(temp.x*127)+127)<<16) + (((int)(-temp.z*127)+127)<<8) + ((int)(temp.y*127)+127);

		// Next vertex
		vxp++;
	}

	// Unlock buffer
	dx8_vbuf->Unlock();
}



//------------------------------------------------------------------
// Storm3D_Mesh_LOD::OptimizeAndBuildBuffers
//------------------------------------------------------------------
void Storm3D_Mesh_LOD::OptimizeAndBuildBuffers(Storm3D_Mesh *owner)
{
	// Test update need
	if (!fc_update_needed) return;
	fc_update_needed=false;

	// Alloc a vx reindex list (and clear it)
	int *ovx_rix=new int[owner->vertex_amount];
	for (int i=0;i<owner->vertex_amount;i++) ovx_rix[i]=-1;

	// VertexCache optimizer v1.0
	// LODs Faceloop (to create new indexes)
	int vix_counter=0;
	for (int fc=0;fc<face_amount;fc++)
	{
		// Set new vertex indexes and new face indexes
		for (int vx=0;vx<3;vx++)
		{
			if (ovx_rix[faces[fc].vertex_index[vx]]<0)
			{
				// Reindex face and save vertex reindex
				ovx_rix[faces[fc].vertex_index[vx]]=vix_counter;
				faces[fc].vertex_index[vx]=vix_counter;
				vix_counter++;
			}
			else
			{
				// Reindex face
				faces[fc].vertex_index[vx]=ovx_rix[faces[fc].vertex_index[vx]];
			}
		}
	}

	// If no vertexes
	if (vix_counter<1) return;
	lod_vertex_amount=vix_counter;

	// Alloc LODs reindex vertex array
	SAFE_DELETE_ARRAY(vx_reindex);
	vx_reindex=new WORD[lod_vertex_amount];

	// Fill reindex array
	for (int vx=0;vx<owner->vertex_amount;vx++)
		if (ovx_rix[vx]>=0) vx_reindex[ovx_rix[vx]]=vx;

	// Create new indexbuffer
	SAFE_RELEASE(dx8_ibuf)
	owner->Storm3D2->D3DDevice->CreateIndexBuffer(sizeof(WORD)*face_amount*3,
		D3DUSAGE_WRITEONLY,D3DFMT_INDEX16,D3DPOOL_MANAGED,&dx8_ibuf);

	// Copy data to indexbuffer
	WORD *ip=NULL;
	dx8_ibuf->Lock(0,sizeof(WORD)*face_amount*3,(BYTE**)&ip,0);
	if (ip)
	{
		for(int i=0;i<face_amount;i++)
		{
			*ip++=faces[i].vertex_index[0];
			*ip++=faces[i].vertex_index[1];
			*ip++=faces[i].vertex_index[2];
		}
	}
	dx8_ibuf->Unlock();

	// Create new vertexbuffer (and release old)
	if (dx8_vbuf) dx8_vbuf->Release();
	owner->Storm3D2->D3DDevice->CreateVertexBuffer(lod_vertex_amount*owner->vbuf_vsize,
		D3DUSAGE_WRITEONLY,owner->vbuf_fvf,D3DPOOL_MANAGED,&dx8_vbuf);
}



//------------------------------------------------------------------
// Storm3D_Mesh_LOD::Create
//------------------------------------------------------------------
/*void Storm3D_Mesh_LOD::Create(Storm3D_Mesh *owner,float remove_value)
{
	// Vertex loop
	for (int vxn=0;vxn<owner->vertex_amount;vxn++)
	{
		// Calculate "angle" value
		VC3 fn=owner->vertexes[vxn].normal;
		float max_angle=0;

		// Loop vertex's faces and search biggest "angle"
		bool is_on_edge=false;
		for (IntLiList *ll=vxfacelists[vxn];ll!=NULL;ll=ll->next)
		{
			// Calculate "angle"
			float ang=(faces[ll->data].normal-fn).GetLength();
			if (ang>max_angle) max_angle=ang;

			// Check if vertex is on the "edge" (never remove edge vertexes)
			if (faces[ll->data].adjacency[0]==-1)
			{
				if (faces[ll->data].vertex_index[0]==vxn) is_on_edge=true;
				if (faces[ll->data].vertex_index[1]==vxn) is_on_edge=true;
			}
			if (faces[ll->data].adjacency[1]==-1)
			{
				if (faces[ll->data].vertex_index[1]==vxn) is_on_edge=true;
				if (faces[ll->data].vertex_index[2]==vxn) is_on_edge=true;
			}
			if (faces[ll->data].adjacency[2]==-1)
			{
				if (faces[ll->data].vertex_index[2]==vxn) is_on_edge=true;
				if (faces[ll->data].vertex_index[0]==vxn) is_on_edge=true;
			}
		}

		// Is the angle small enough?
		if (max_angle>remove_value) continue;		

		// Is the vertex on the edge?
		if (is_on_edge) continue;
		
		// Temp edge list for polygon
		IntLiList edgelist;

		// Create polygon edge list
		for (ll=vxfacelists[vxn];ll!=NULL;ll=ll->next)
		{
			// Take the edges which are not using this vertex...
			if (vxn==faces[ll->data].vertex_index[0])
			{
				//edgelist... jatka tästä!
			}
		}

	}
}*/



