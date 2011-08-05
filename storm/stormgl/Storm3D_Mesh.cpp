// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d.h"
#include "storm3d_mesh.h"
#include "storm3d_model.h"
#include "storm3d_model_object.h"
#include "storm3d_material.h"
#include "storm3d_texture.h"
#include "storm3d_scene.h"
#include "VertexFormats.h"
#include "igios3D.h"

#include "Storm3D_Bone.h"
#include "Storm3D_ShaderManager.h"
#include <algorithm>
#include "../../util/Debug_MemoryManager.h"

int storm3d_mesh_allocs = 0;
int storm3d_dip_calls = 0;

//! Constructor
Storm3D_Mesh::Storm3D_Mesh(Storm3D *s2, Storm3D_ResourceManager &resourceManager_) :
	Storm3D2(s2),
	resourceManager(resourceManager_),
	material(NULL),
	vertex_amount(0),
	render_vertex_amount(0),
	hasLods(false),
	vertexes(NULL),
	bone_weights(NULL),
	vbuf(0),
	radius(0),
	sq_radius(0),
	radius2d(0),
	rb_update_needed(true),
	update_vx(true),
	update_vx_amount(true),
	update_fc(true),
	update_fc_amount(true),
	col_rebuild_needed(true),
	sphere_ok(false),
	box_ok(false)
{
	for(int i = 0; i < LOD_AMOUNT; ++i)
	{
		render_face_amount[i] = 0;
		face_amount[i] = 0;
		faces[i] = 0;
		ibuf[i] = 0;
	}
	storm3d_mesh_allocs++;
}

//! Get bounding sphere of mesh
/*!
	\return bounding sphere
*/
const Sphere &Storm3D_Mesh::getBoundingSphere() const
{
	if(!sphere_ok)
	{
		VC3 minSize(1000000.f, 1000000.f, 1000000.f);
		VC3 maxSize(-1000000.f, -1000000.f, -1000000.f);

		for(int i = 0; i < vertex_amount; ++i)
		{
			const VC3 &v = vertexes[i].position;
			if(v.x < minSize.x)
				minSize.x = v.x;
			if(v.y < minSize.y)
				minSize.y = v.y;
			if(v.z < minSize.z)
				minSize.z = v.z;

			if(v.x > maxSize.x)
				maxSize.x = v.x;
			if(v.y > maxSize.y)
				maxSize.y = v.y;
			if(v.z > maxSize.z)
				maxSize.z = v.z;
		}

		VC3 center = (minSize + maxSize) / 2.f;
		bounding_sphere.position = center;
		bounding_sphere.radius = center.GetRangeTo(maxSize);

		sphere_ok = true;
	}

	return bounding_sphere;
}

//! Get bounding box of mesh
/*!
	\return bounding box
*/
const AABB &Storm3D_Mesh::getBoundingBox() const
{
	if(!box_ok)
	{
		VC3 minSize(1000000.f, 1000000.f, 1000000.f);
		VC3 maxSize(-1000000.f, -1000000.f, -1000000.f);

		for(int i = 0; i < vertex_amount; ++i)
		{
			const VC3 &v = vertexes[i].position;
			if(v.x < minSize.x)
				minSize.x = v.x;
			if(v.y < minSize.y)
				minSize.y = v.y;
			if(v.z < minSize.z)
				minSize.z = v.z;

			if(v.x > maxSize.x)
				maxSize.x = v.x;
			if(v.y > maxSize.y)
				maxSize.y = v.y;
			if(v.z > maxSize.z)
				maxSize.z = v.z;
		}

		bounding_box.mmin = minSize;
		bounding_box.mmax = maxSize;
		box_ok = true;
	}

	return bounding_box;
}

//! Destructor
Storm3D_Mesh::~Storm3D_Mesh()
{
	storm3d_mesh_allocs--;

	// Remove from Storm3D's list
	Storm3D2->Remove(this, 0);

	// Delete arrays
	if (vertexes) 
		delete[] vertexes;

	delete[] bone_weights;

	// Release buffers
	if (glIsBuffer(vbuf)) {
		glDeleteBuffers(1, &vbuf);
		vbuf = 0;
	}

	for (int i = 0; i < LOD_AMOUNT; i++) {
		if (glIsBuffer(ibuf[i]))
			glDeleteBuffers(1, &ibuf[i]);
	}

	for(int i = 0; i < LOD_AMOUNT; ++i)
	{
		delete[] faces[i];
	}

	if(material)
		resourceManager.removeUser(material, this);
}

//! Make a clone of the mesh
/*!
	\return clone
*/
IStorm3D_Mesh *Storm3D_Mesh::CreateNewClone()
{
	Storm3D_Mesh *ret = (Storm3D_Mesh *)Storm3D2->CreateNewMesh();

	if (material)
	{
		ret->UseMaterial((Storm3D_Material *)material->CreateNewClone());
	}

	ret->vertex_amount = this->vertex_amount;

	if (vertexes)
	{
		ret->vertexes = new Storm3D_Vertex[vertex_amount];
		for (int i = 0; i < vertex_amount; i++)
		{
			ret->vertexes[i] = this->vertexes[i];
		}
	}

	for(int i = 0; i < LOD_AMOUNT; ++i)
	{
		ret->face_amount[i] = this->face_amount[i];

		if(faces[i])
		{
			ret->faces[i] = new Storm3D_Face[face_amount[i]];
			for (int j = 0; j < face_amount[i]; j++)
				ret->faces[i][j] = this->faces[i][j];
		}
	}

	if(bone_weights)
	{
		ret->bone_weights = new Storm3D_Weight[vertex_amount];
		for(int i = 0; i < vertex_amount; ++i)
			ret->bone_weights[i] = bone_weights[i];
	}

	return ret;
}

//! Prepare mesh for render
/*!
	\param scene scene
	\param object model object
	You can set scene=NULL, object=NULL if you don't need animation
*/
void Storm3D_Mesh::PrepareForRender(Storm3D_Scene *scene,Storm3D_Model_Object *object)
{
	// Check if material is changed in a way that forces object
	// buffers to be rebuilt.
	if(bone_weights)
	{
		if (vbuf_fvf!=FVF_VXFORMAT_BLEND)
		{
			update_vx=true;
			update_vx_amount=true;
		}
	}	
	else if (vbuf_fvf!=FVF_VXFORMAT_TC2)
	{
		update_vx=true;
		update_vx_amount=true;
	}

	// If rebuild is needed: do it!
	ReBuild();
}

//! Prepare material for render
/*!
	\param scene scene
	\param object model object
*/
void Storm3D_Mesh::PrepareMaterialForRender(Storm3D_Scene *scene,Storm3D_Model_Object *object)
{
	// Apply material (if it is not already active)
	if (material!=Storm3D2->active_material)
	{
		// Create world matrix
		MAT mxx;
		mxx = object->GetMXG();

		if (material)
			material->Apply(scene,0,vbuf_fvf,(D3DMATRIX*)&mxx);
		else
		{
			// Set stages (color only)
			glEnable(GL_ALPHA_TEST);
			glDepthMask(GL_TRUE);
			glEnable(GL_BLEND);
			glActiveTexture(GL_TEXTURE0);
			glClientActiveTexture(GL_TEXTURE0);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_TEXTURE_3D);
			glDisable(GL_TEXTURE_CUBE_MAP);
			glBindTexture(GL_TEXTURE_2D, 0);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
			glActiveTexture(GL_TEXTURE1);
			glClientActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, 0);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_TEXTURE_3D);
			glDisable(GL_TEXTURE_CUBE_MAP);

			// Set active material
			Storm3D2->active_material=NULL;
		}
	}

	// If object is scaled use normalizenormals, otherwise don't
	if ((fabsf(object->scale.x-1.0f)>=0.001)|| (fabsf(object->scale.y-1.0f)>=0.001) || (fabsf(object->scale.z-1.0f)>=0.001))
		glEnable(GL_NORMALIZE);
	else
		glDisable(GL_NORMALIZE);
}

//! Render buffers
/*!
	\param object model object
*/
void Storm3D_Mesh::RenderBuffers(Storm3D_Model_Object *object)
{
	// Test
	if (!glIsBuffer(vbuf)) 
		return;
	if (!glIsBuffer(ibuf[0])) 
		return;

	int lod = object->parent_model->lodLevel;
	if(!hasLods)
		lod = 0;

	if(bone_weights)
	{
		for(unsigned int i = 0; i < bone_chunks[lod].size(); ++i)
		{
			Storm3D_ShaderManager *manager = Storm3D_ShaderManager::GetSingleton();
			manager->SetShader(bone_chunks[lod][i].bone_indices);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bone_chunks[lod][i].index_buffer);
			setStreamSource(0, bone_chunks[lod][i].vertex_buffer, 0, vbuf_vsize);

			glDrawRangeElements(GL_TRIANGLES, 0, bone_chunks[lod][i].vertex_count, bone_chunks[lod][i].index_count*3, GL_UNSIGNED_SHORT, NULL);

			++storm3d_dip_calls;
		}
	}
	else
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuf[lod]);
		setStreamSource(0, vbuf, 0, vbuf_vsize);

		glDrawRangeElements(GL_TRIANGLES, 0, render_vertex_amount, render_face_amount[lod]*3, GL_UNSIGNED_SHORT, NULL);

		++storm3d_dip_calls;
	}
}

//! Render buffers without transformation
void Storm3D_Mesh::RenderBuffersWithoutTransformation()
{
	// Test
	if (!glIsBuffer(vbuf)) 
		return;
	if (!glIsBuffer(ibuf[0])) 
		return;

	// Mesh buffer change optimization (v2.6)
	if (Storm3D2->active_mesh!=this)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuf[0]);
		setStreamSource(0, vbuf, 0, vbuf_vsize);
	}

	// Render it!
	glDrawRangeElements(GL_TRIANGLES, 0, vertex_amount, face_amount[0]*3, GL_UNSIGNED_SHORT, NULL);

	// Mesh buffer change optimization (v2.6)
	Storm3D2->active_mesh=this;
	++storm3d_dip_calls;
}

//! Render
/*!
	\param scene
	\param mirrored
	\param object
*/
void Storm3D_Mesh::Render(Storm3D_Scene *scene,bool mirrored,Storm3D_Model_Object *object)
{
	// Prepare for rendering (v3)
	PrepareForRender(scene,object);

	// Test
	if (!glIsBuffer(vbuf)) 
		return;
	if (!glIsBuffer(ibuf[0])) 
		return;

	// Prepare material for rendering (v3)
	PrepareMaterialForRender(scene,object);

	// Reverse culling if mirrored
	if (mirrored)
	{
		if (material)
		{
			bool ds,wf;
			material->GetSpecial(ds,wf);
		}
	}

	int lod = object->parent_model->lodLevel;
	if(!hasLods)
		lod = 0;

	// Render vertex buffers
	RenderBuffers(object);
	scene->AddPolyCounter(face_amount[lod]);
}

//! Render without material
/*!
	\param scene
	\param mirrored
	\param object
*/
void Storm3D_Mesh::RenderWithoutMaterial(Storm3D_Scene *scene,bool mirrored,Storm3D_Model_Object *object)
{
	// Prepare for rendering (v3)
	PrepareForRender(scene,object);

	// Test
	if (!glIsBuffer(vbuf)) 
		return;
	if (!glIsBuffer(ibuf[0])) 
		return;

	// Reverse culling if mirrored
	if (mirrored)
	{
		if (material)
		{
			bool ds,wf;
			material->GetSpecial(ds,wf);
		}
	}

	// Render vertex buffers
	RenderBuffers(object);
	scene->AddPolyCounter(face_amount[0]);
}

//! Render mesh to background
/*!
	\param scene
	\param object
*/
void Storm3D_Mesh::RenderToBackground(Storm3D_Scene *scene,Storm3D_Model_Object *object)
{
	// Prepare for rendering (v3)
	PrepareForRender(scene,object);

	// Test
	if (!glIsBuffer(vbuf)) 
		return;
	if (!glIsBuffer(ibuf[0])) 
		return;

	// Prepare material for rendering (v3)
	PrepareMaterialForRender(scene,object);

	// Disable some states
	//Storm3D2->D3DDevice->SetRenderState(D3DRS_SPECULARENABLE,FALSE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_NORMALIZE);

	// Render vertex buffers
	RenderBuffers(object);
	scene->AddPolyCounter(face_amount[0]);

	// Return states
	glEnable(GL_DEPTH_TEST);
}

//! Rebuild mesh
void Storm3D_Mesh::ReBuild()
{
	// Test if rebuilding is needed
	if ((!update_vx)&&(!update_vx_amount)&&
		(!update_fc)&&(!update_fc_amount)) return;

	// Test array sizes
	if ((face_amount[0]<1)) return;
	if (vertex_amount<1) return;

	// Select format for vertexbuffer
	int size=sizeof(VXFORMAT_TC2);
	DWORD fvf=FVF_VXFORMAT_TC2;

	if(bone_weights)
	{
		size=sizeof(VXFORMAT_BLEND);
		fvf=FVF_VXFORMAT_BLEND;
	}

	// Test if failed
	if (size<1) return;
	if (fvf==0) return;

	// Save size/fvf to object
	vbuf_vsize=size;
	vbuf_fvf=fvf;

	// Create vertex/index buffers
	if (update_vx_amount)
	{
		// Create new vertexbuffer
		// on GL we don't need to release old
		if (!glIsBuffer(vbuf)) {
			glGenBuffers(1, &vbuf);
		}

		glBindBuffer(GL_ARRAY_BUFFER, vbuf);
		glBufferData(GL_ARRAY_BUFFER, vertex_amount*size, NULL, GL_DYNAMIC_DRAW);
	}

	int lodLevels = hasLods ? LOD_AMOUNT : 1;

	// Build bone chunks
	if((bone_weights) && ((update_vx) || (update_fc)))
	{
		for(int lod = 0; lod < lodLevels; ++lod)
		{
			bone_chunks[lod].clear();		 

			// Find used bones indices
			vector<int> bone_indices;
			int max_bone_index = 0;

			vector<vector<int> > chunk_faces;

			for(int v = 0; v < vertex_amount; ++v)
			{
				int index = bone_weights[v].index1;
				if(index == -1)
					continue;

				if(index > max_bone_index)
					max_bone_index = index;
				if(find(bone_indices.begin(), bone_indices.end(), index) == bone_indices.end())
					bone_indices.push_back(index);

				index = bone_weights[v].index2;
				if(index == -1)
					continue;

				if(index > max_bone_index)
					max_bone_index = index;
				if(find(bone_indices.begin(), bone_indices.end(), index) == bone_indices.end())
					bone_indices.push_back(index);
			}

			sort(bone_indices.begin(), bone_indices.end());

			for(int i = 0; i < face_amount[lod]; ++i)
			{
				// Find all used weights
				const Storm3D_Face &f = faces[lod][i];
				int weights[6] = { -1, -1, -1, -1, -1, -1 };
				int weight_index = 0;

				for(int k = 0; k < 3; ++k)
				{
					int vindex = f.vertex_index[k];

					int bone_index = bone_weights[vindex].index1;
					if(bone_index >= 0)
					{
						if(find(&weights[0], &weights[6], bone_index) == &weights[6])
							weights[weight_index++] = bone_index;
					}

					bone_index = bone_weights[vindex].index2;
					if(bone_index >= 0)
					{
						if(find(&weights[0], &weights[6], bone_index) == &weights[6])
							weights[weight_index++] = bone_index;
					}
				}

				// If some chunk (index) already contains needed bones or contains enough spaces for them (could_insert_chunk)
				int index = -1;
				int could_insert_chunk = -1;

				for(unsigned int j = 0; j < bone_chunks[lod].size(); ++j)
				{
					Storm3D_BoneChunk &chunk = bone_chunks[lod][j];

					int bones_found = 0;
					for(int k = 0; k < weight_index; ++k)
					{
						for(unsigned int l = 0; l < chunk.bone_indices.size(); ++l)
						{
							if(weights[k] == chunk.bone_indices[l])
								++bones_found;
						}
					}

					if(int(chunk.bone_indices.size() + weight_index - bones_found) < Storm3D_ShaderManager::BONE_INDICES)
						could_insert_chunk = j;

					if(bones_found == weight_index)
						index = j;
				}

				// Create new chunk if needed
				if(index == -1)
				{
					index = could_insert_chunk;
					if(index == -1)
					{
						index = bone_chunks[lod].size();
						bone_chunks[lod].resize(index + 1);
					}

					Storm3D_BoneChunk &insert_chunk = bone_chunks[lod][index];
					for(int k = 0; k < weight_index; ++k)
					{
						if(find(insert_chunk.bone_indices.begin(), insert_chunk.bone_indices.end(), weights[k]) != insert_chunk.bone_indices.end())
							continue;

						int new_index = weights[k];
						insert_chunk.bone_indices.push_back(new_index);
					}
				}

				if(int(chunk_faces.size()) <= index)
					chunk_faces.resize(index + 1);

				chunk_faces[index].push_back(i);
			}

			// Ok, we now have chunks with bone lists and faces attached on them
			int bone_chunk_amount = bone_chunks[lod].size();
			for(int i = 0; i < bone_chunk_amount; ++i)
			{
				const vector<int> &chunk_face_list = chunk_faces[i];
				Storm3D_BoneChunk &chunk = bone_chunks[lod][i];

				vector<int> vertex_list;
				vector<int> vertex_convert_list(vertex_amount, -1);

				// Create index buffer
				{
					if(!glIsBuffer(chunk.index_buffer)) {
						glGenBuffers(1, &chunk.index_buffer);
					}

					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk.index_buffer);
					glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * chunk_face_list.size() * 3, NULL, GL_DYNAMIC_DRAW);

					GLushort *ip = reinterpret_cast<unsigned short *> (glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));
					if(ip)
					{
						for(unsigned int j = 0; j < chunk_face_list.size(); ++j)
						{
							const Storm3D_Face &f = faces[lod][chunk_face_list[j]];
							for(int k = 0; k < 3; ++k)
							{
								int vertex_index = f.vertex_index[k];
								if(vertex_convert_list[vertex_index] == -1)
								{
									vertex_convert_list[vertex_index] = vertex_list.size();
									vertex_list.push_back(vertex_index);
								}

								*ip++ = vertex_convert_list[vertex_index];
							}
						}
					}

					glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
					chunk.index_count = chunk_face_list.size();
				}

				// Create vertex buffer
				{
					if(!glIsBuffer(chunk.vertex_buffer)) {
						glGenBuffers(1, &chunk.vertex_buffer);
					}

					glBindBuffer(GL_ARRAY_BUFFER, chunk.vertex_buffer);
					glBufferData(GL_ARRAY_BUFFER, vertex_list.size() * size, NULL, GL_DYNAMIC_DRAW);

					GLubyte *vp = reinterpret_cast<GLubyte *> (glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
					glVertexPointer(3, GL_FLOAT, size, 0);
					
					VXFORMAT_BLEND *p=(VXFORMAT_BLEND*)vp;
					float weight[4] = { 0 };

					for(unsigned int i = 0; i < vertex_list.size(); ++i)
					{
						int vertex_index = vertex_list[i];
						int index1 = bone_weights[vertex_index].index1;
						int index2 = bone_weights[vertex_index].index2;

						if(index1 != -1)
						{
							for(unsigned int j = 0; j < chunk.bone_indices.size(); ++j)
							{
								if(index1 == chunk.bone_indices[j])
								{
									index1 = j;
									break;
								}
							}

							assert(index1 >= 0 && index1 <= Storm3D_ShaderManager::BONE_INDICES);
						}
						if(index2 != -1)
						{
							for(unsigned int j = 0; j < chunk.bone_indices.size(); ++j)
							{
								if(index2 == chunk.bone_indices[j])
								{
									index2 = j;
									break;
								}
							}

							assert(index1 >= 0 && index1 <= Storm3D_ShaderManager::BONE_INDICES);
						}

						weight[0] = float((index1) * 3);
						weight[0] += Storm3D_ShaderManager::BONE_INDEX_START;
						weight[1] = bone_weights[vertex_index].weight1 / 100.f;

						weight[2] = float(Storm3D_ShaderManager::BONE_INDEX_START);
						weight[3] = 0;
						
						if(index2 >= 0)
						{
							weight[2] = float((index2) * 3);
							weight[2] += Storm3D_ShaderManager::BONE_INDEX_START;
							weight[3] = bone_weights[vertex_index].weight2 / 100.f;
						}
			
						p[i]=VXFORMAT_BLEND(vertexes[vertex_index].position,vertexes[vertex_index].normal,vertexes[vertex_index].texturecoordinates, vertexes[vertex_index].texturecoordinates2, weight);
					}

					glUnmapBuffer(GL_ARRAY_BUFFER);
					chunk.vertex_count = vertex_list.size();
				}
			}
		}
	}

	for(int i = 0; i < lodLevels; ++i)
	{
		if(faces[i])
		{
			if (update_fc_amount)
			{
				if (!glIsBuffer(ibuf[i])) {
					glGenBuffers(1, &ibuf[i]);
				}
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuf[i]);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, face_amount[i] * 3 * sizeof(GLushort), NULL, GL_DYNAMIC_DRAW);
			}

			if (update_fc)
			{
				// Copy data to indexbuffer
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuf[i]);
				GLushort *ip = reinterpret_cast<unsigned short *> (glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));
				if (ip)
				{
					for(int j=0;j<face_amount[i];j++)
					{
						*ip++=faces[i][j].vertex_index[0];
						*ip++=faces[i][j].vertex_index[1];
						*ip++=faces[i][j].vertex_index[2];
					}
				}

				glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
			}
		}
	}

	if((update_vx) && (!bone_weights))
	{
		// Copy data to vertexbuffer
		glErrors();
		glBindBuffer(GL_ARRAY_BUFFER, vbuf);
		glErrors();

		GLint temp;
		glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &temp);
		if (temp < (int)(vertex_amount*sizeof(VXFORMAT_TC2)))
		{
			igios_unimplemented(); // FIXME: this if BUGGY! should not be necessary
			igiosWarning("vbuf %d %d\n", vbuf, glIsBuffer(vbuf));
			igiosWarning("size: %d vertex amount: %d %d\n", temp, vertex_amount, vertex_amount*sizeof(VXFORMAT_TC2));
		} else {
			// Typecast (to simplify code)
			VXFORMAT_TC2 *p=(VXFORMAT_TC2*) glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
			if (p == NULL) {
				igiosWarning("MapBuffer failed\n");
				glErrors();
				return;
			}

			for(int i=0;i<vertex_amount;i++)
			{
				p[i]=VXFORMAT_TC2(vertexes[i].position,vertexes[i].normal,
					vertexes[i].texturecoordinates,vertexes[i].texturecoordinates2);
			}
		}

		glUnmapBuffer(GL_ARRAY_BUFFER);
	}

	// Clear the markers
	update_vx=false;
	update_vx_amount=false;
	update_fc=false;
	update_fc_amount=false;

	render_face_amount[0] = face_amount[0];
	render_vertex_amount = vertex_amount;
}

//! Change mesh material
/*!
	\param material material
*/
void Storm3D_Mesh::UseMaterial(IStorm3D_Material *_material)
{
	if(material)
		resourceManager.removeUser(material, this);

	// Set the material
	material=(Storm3D_Material*)_material;

	// Some times material change means that object buffers
	// must be rebuilt. However now the testing is done when
	// object is rendered, because material may change itself.

	if(material)
		resourceManager.addUser(material, this);
}

//! Get mesh material
/*!
	\return material
*/
IStorm3D_Material *Storm3D_Mesh::GetMaterial()
{
	return material;
}

//! Delete all faces of mesh
void Storm3D_Mesh::DeleteAllFaces()
{
	for(int i = 0; i < LOD_AMOUNT; ++i)
	{
		delete[] faces[i];
		faces[i] = 0;
		face_amount[i] = 0;
	}

	// Set rebuild
	update_fc=true;
	update_fc_amount=true;
	sphere_ok = false;
	box_ok = false;
}

//! Delete all vertices of mesh
void Storm3D_Mesh::DeleteAllVertexes()
{
	if (vertexes) delete[] vertexes;
	vertexes=NULL;
	vertex_amount=0;

	// Set rebuild
	update_vx=true;
	update_vx_amount=true;
	rb_update_needed=true;

	hasLods = false;
	sphere_ok = false;
	box_ok = false;
}

//! Dynamic geometry edit (New in v2.2B)
/*!
	\return face buffer
*/
Storm3D_Face *Storm3D_Mesh::GetFaceBuffer()
{
	// Set rebuild
	update_fc=true;
	col_rebuild_needed=true;
	sphere_ok = false;
	box_ok = false;

	return faces[0];
}

//! Get vertex buffer
/*!
	\return vertex buffer
*/
Storm3D_Vertex *Storm3D_Mesh::GetVertexBuffer()
{
	// Set rebuild
	update_vx=true;
	col_rebuild_needed=true;
	rb_update_needed=true;
	sphere_ok = false;
	box_ok = false;

	return vertexes;
}

//! Get read-only face buffer
/*!
	\return face buffer
*/
const Storm3D_Face *Storm3D_Mesh::GetFaceBufferReadOnly()
{
	return faces[0];
}

//! Get read-only face buffer from specified index
/*!
	\param lodIndex index
	\return face buffer
*/
const Storm3D_Face *Storm3D_Mesh::GetFaceBufferReadOnly(int lodIndex)
{
	assert(lodIndex >= 0 && lodIndex < LOD_AMOUNT);

	if(face_amount[lodIndex] == 0)
		return faces[0];
	else
		return faces[lodIndex];
}

//! Get read-only vertex buffer
/*!
	\return vertex buffer
*/
const Storm3D_Vertex *Storm3D_Mesh::GetVertexBufferReadOnly()
{
	return vertexes;
}

//! Get number of faces
/*!
	\return face count
*/
int Storm3D_Mesh::GetFaceCount()
{
	return face_amount[0];
}

//! Get number of faces from specified index
/*!
	\param lodIndex index
	\return face count
*/
int Storm3D_Mesh::GetFaceCount(int lodIndex)
{
	assert(lodIndex >= 0 && lodIndex < LOD_AMOUNT);

	if(face_amount[lodIndex] == 0)
		return face_amount[0];
	else
		return face_amount[lodIndex];
}

//! Get number of vertices
/*!
	\return vertex count
*/
int Storm3D_Mesh::GetVertexCount()
{
	return vertex_amount;
}

//! Change number of faces
/*!
	\param new_face_count new number of faces
*/
void Storm3D_Mesh::ChangeFaceCount(int new_face_count)
{
	// Delete old
	DeleteAllFaces();

	// Set faceamount
	face_amount[0]=new_face_count;
	if (face_amount[0] < 1) 
		return;

	// Allocate memory for new ones
	faces[0]=new Storm3D_Face[face_amount[0]];

	// Set rebuild
	update_fc=true;
	col_rebuild_needed=true;
	sphere_ok = false;
	box_ok = false;
}

//! Change number of vertices
/*!
	\param new_vertex_count new number of vertices
*/
void Storm3D_Mesh::ChangeVertexCount(int new_vertex_count)
{
	// Delete old
	DeleteAllVertexes();

	// Set faceamount
	vertex_amount=new_vertex_count;
	if (vertex_amount<1) return;

	// Allocate memory for new ones
	vertexes=new Storm3D_Vertex[vertex_amount];

	// Set rebuild
	update_vx=true;
	col_rebuild_needed=true;
	rb_update_needed=true;
	sphere_ok = false;
	box_ok = false;
}

//! Update mesh collision table
void Storm3D_Mesh::UpdateCollisionTable()
{
	if (col_rebuild_needed) 
		collision.ReBuild(this);
	col_rebuild_needed=false;
}

//! Claculate radius and bounding box
void Storm3D_Mesh::CalculateRadiusAndBox()
{
	// Test
	if (vertexes==NULL)
	{
		radius=0;
		radius2d = 0;
		sq_radius=0;
		rb_update_needed=false;
		return;
	}

	// Set values to init state
	sq_radius=0;
	radius2d = 0;

	// Loop through all vertexes
	for (int vx=0;vx<vertex_amount;vx++)
	{
		// Update sq_radius
		const VC3 &pos = vertexes[vx].position;
		float r = pos.GetSquareLength();
		if (r > sq_radius) 
			sq_radius = r;

		float r2 = (pos.x * pos.x) + (pos.z * pos.z);
		if(r2 > radius2d)
			radius2d = r2;
	}

	// Calculate radius
	radius = sqrtf(sq_radius);
	radius2d = sqrtf(radius2d);

	// Clear marker
	rb_update_needed=false;
}

//! Get radius of mesh
/*!
	\return radius
*/
float Storm3D_Mesh::GetRadius()
{
	if (rb_update_needed) CalculateRadiusAndBox();
	return radius;
}

//! Get square radius of mesh
/*!
	\return radius
*/
float Storm3D_Mesh::GetSquareRadius()
{
	if (rb_update_needed) CalculateRadiusAndBox();
	return sq_radius;
}

//! Raytrace mesh
/*!
	\param position
	\param direction_normalized
	\param ray_length
	\param rti
	\param accurate
	\return true if success
*/
bool Storm3D_Mesh::RayTrace(const VC3 &position,const VC3 &direction_normalized,float ray_length,Storm3D_CollisionInfo &rti, bool accurate)
{
	// No collision table! (do not test collision)
	if (col_rebuild_needed) return false;

	// Raytrace
	return collision.RayTrace(position,direction_normalized,ray_length,rti, accurate);
}

//! Test if mesh collides with given sphere
/*!
	\param position center of sphere
	\param radius radius of sphere
	\param cinf collision info
	\param accurate
	\return 
*/
bool Storm3D_Mesh::SphereCollision(const VC3 &position,float radius,Storm3D_CollisionInfo &cinf, bool accurate)
{
	// No collision table! (do not test collision)
	if (col_rebuild_needed) return false;

	// Test collision
	return collision.SphereCollision(position,radius,cinf, accurate);
}

//! Do bones have weights?
/*!
	\return true if they have weights
*/
bool Storm3D_Mesh::HasWeights() const
{
	if(bone_weights)
		return true;
	else
		return false;
}

#define VectorToRGBA(x,y,z) \
(( ((UINT)((UINT)((x)*127)+127)) << 16 ) | ( ((UINT)((UINT)((y)*127)+127)) << 8 ) | ( ((UINT)((UINT)((z)*127)+127)) ))

//! Apply vertex buffer
void Storm3D_Mesh::applyBuffers()
{
	assert(vbuf);
	assert(vbuf_vsize);

	setStreamSource(0, vbuf, 0, vbuf_vsize);
}

//! Render primitives
int Storm3D_Mesh::renderPrimitives(float range)
{
	int lod = 0;

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuf[lod]);

	glVertexPointer(3, GL_FLOAT, 0, 0);
	glDrawRangeElements(GL_TRIANGLES, 0, vertex_amount, face_amount[lod]*3, GL_UNSIGNED_INT, NULL);

	++storm3d_dip_calls;
	return face_amount[lod];
}
