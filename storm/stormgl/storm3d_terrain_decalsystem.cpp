// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

#include <boost/shared_ptr.hpp>
#include <vector>
#include <deque>

#include "storm3d_terrain_decalsystem.h"
#include "storm3d_terrain_utils.h"
#include "storm3d_texture.h"
#include "storm3d_material.h"
#include "Storm3D_ShaderManager.h"
#include "storm3d_spotlight.h"
#include <c2_sphere.h>
#include <c2_frustum.h>
#include <c2_qtree.h>
#include "storm3d.h"
#include "storm3d_scene.h"
#include "igios3D.h"

#include "../../util/Debug_MemoryManager.h"

using namespace std;
using namespace boost;
using namespace frozenbyte::storm;

	struct StormDecal;
	struct DecalMaterial;
	typedef deque<StormDecal> StormDecalList;
	typedef vector<VertexBuffer> VertexBufferList;
	typedef vector<DecalMaterial> DecalMaterialList;


	// position + normal + texcoord + color 
	static const int VERTEX_SIZE = 3*4 + 3*4 + 4*4 + 1*4;
	static const int STORM_MAX_DECAL_AMOUNT = 10000;

	enum RenderMode
	{
		Textures,
		Light
	};

	static const int DECAL_FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2;
	struct VXFORMAT_DECAL
	{
		VC3 position;
		VC3 normal;
		DWORD color;
		VC2 texcoords;
		VC2 texcoords2;

		// Constructor
		VXFORMAT_DECAL(const VC3 &position_, const VC3 &normal_, DWORD color_, const VC2 &texcoords_, const VC2 &texcoords2_)
		:	position(position_),
			normal(normal_),
			color(color_),
			texcoords(texcoords_),
			texcoords2(texcoords2_)
		{
		}
	};

	typedef Quadtree<StormDecal> Tree;

	struct StormDecal
	{
		VC3 position;
		VC2 size;
		QUAT rotation;
		COL light;
		VC2 special;

		unsigned char alpha;
		int materialIndex;

		VC3 normal;
		VC3 vertices[4];
		mutable DWORD vertexColor;

		bool deleted;
		Tree::Entity *entity;

		int id;

		StormDecal()
		:	alpha(0),
			materialIndex(-1),
			vertexColor(0),
			deleted(false),
			entity(0),
			id(0)
		{
		}

		float getRadius() const
		{
			return vertices[0].GetRangeTo(vertices[2]) * .5f;
		}

		void calculateValues()
		{
			MAT tm;
			tm.CreateRotationMatrix(rotation);

			// A bit optimized version, use base vectors directly

			normal.x = tm.Get(8);
			normal.y = tm.Get(9);
			normal.z = tm.Get(10);
			VC3 biasNormal = normal;
			biasNormal *= .07f;

			VC3 right(tm.Get(0), tm.Get(1), tm.Get(2));
			VC3 up(tm.Get(4), tm.Get(5), tm.Get(6));

			right *= .5f * size.x;
			up *= .5f * size.y;

			assert(fabsf(up.GetDotWith(right)) < 0.0001f);

			vertices[0] = position;
			vertices[0] -= right;
			vertices[0] -= up;
			vertices[0] += biasNormal;

			vertices[1] = position;
			vertices[1] -= right;
			vertices[1] += up;
			vertices[1] += biasNormal;

			vertices[2] = position;
			vertices[2] += right;
			vertices[2] -= up;
			vertices[2] += biasNormal;

			vertices[3] = position;
			vertices[3] += right;
			vertices[3] += up;
			vertices[3] += biasNormal;

			COL finalColor = light;
			finalColor.Clamp();
			vertexColor = finalColor.GetAsD3DCompatibleARGB() & 0x00FFFFFF;
			vertexColor |= alpha << 24;

			if(entity)
				entity->setRadius(getRadius());
		}

		void insert(VXFORMAT_DECAL *buffer) const
		{
			buffer->position = vertices[0];
			buffer->normal = normal;
			buffer->color = vertexColor;
			buffer->texcoords = VC2(0.f, 0.f);
			buffer->texcoords2 = special;

			++buffer;
			buffer->position = vertices[1];
			buffer->normal = normal;
			buffer->color = vertexColor;
			buffer->texcoords = VC2(0.f, 1.f);
			buffer->texcoords2 = special;

			++buffer;
			buffer->position = vertices[2];
			buffer->normal = normal;
			buffer->color = vertexColor;
			buffer->texcoords = VC2(1.f, 0.f);
			buffer->texcoords2 = special;

			++buffer;
			buffer->position = vertices[3];
			buffer->normal = normal;
			buffer->color = vertexColor;
			buffer->texcoords = VC2(1.f, 1.f);
			buffer->texcoords2 = special;
		}

		bool fits(const AABB &area) const
		{
			if(!contains2D(area, vertices[0]))
				return false;
			if(!contains2D(area, vertices[1]))
				return false;
			if(!contains2D(area, vertices[2]))
				return false;
			if(!contains2D(area, vertices[3]))
				return false;
			
			return true;
		}

		bool SphereCollision(const VC3 &pos, float radius, Storm3D_CollisionInfo &info, bool accurate)
		{
			if(pos.GetRangeTo(position) < radius + getRadius())
			{
				info.hit = true;
				return true;
			}

			return false;
		}
	};

	struct DecalMaterial
	{
		COL diffuseColor;
		shared_ptr<Storm3D_Texture> baseTexture;

		StormDecalList decals;
		int materialIndex;

		Tree *tree;

		DecalMaterial(Tree *tree_)
		:	materialIndex(-1),
			tree(tree_)
		{
		}

		int addDecal(IStorm3D_TerrainDecalSystem::Type type, const VC3 &position, int &id, bool forceSpawn)
		{
			int index = decals.size();
			for(unsigned int i = 0; i < decals.size(); ++i)
			{
				if(decals[i].deleted)
				{
					index = i;
					break;
				}
			}

			if(index >= int(decals.size()))
				decals.resize(index + 1);

			bool canAdd = true;
			if(!forceSpawn)
			{
				static const float CHECK_RADIUS = 0.15f;
				static const int MAX_OVERLAP = 3;

				std::vector<StormDecal *> list;
				tree->collectSphere(list, position, CHECK_RADIUS);

				int overlaps = list.size();

				if(overlaps >= MAX_OVERLAP)
					canAdd = false;
			}

			StormDecal &decal = decals[index];
			id = ++decal.id;
			decal.position = position;
			decal.materialIndex = materialIndex;

			if(canAdd)
			{
				decal.deleted = false;
				if(type == IStorm3D_TerrainDecalSystem::Outside)
					decal.special.x = 8.1f;
				else
					decal.special.x = 9.1f;

				decal.entity = tree->insert(&decal, position, 0.1f);
			}

			return index;
		}

		void eraseDecal(int index, int id)
		{
			assert(id);

			StormDecal &decal = decals[index];
			if(decal.id == id && decal.entity)
			{
				tree->erase(decal.entity);
				decal.entity = 0;
				decal.deleted = true;
			}
		}

		void updateDecal(int index)
		{
			StormDecal &decal = decals[index];
			decal.calculateValues();
		}

		void apply()
		{
			if(baseTexture)
				baseTexture->Apply(1);

			float diffuse[4] = { diffuseColor.r, diffuseColor.g, diffuseColor.b, 1.0f };
			glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 7, diffuse);
		}

		void applyProjection(const COL &spotColor)
		{
			if(baseTexture)
				baseTexture->Apply(2);

			float factor = 1.0f;
			float diffuse[4] = { diffuseColor.r * factor * spotColor.r, diffuseColor.g * factor * spotColor.g, diffuseColor.b * factor * spotColor.b, 1.0f };
			glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 17, diffuse);
		}

		void applyShadow()
		{
			if(baseTexture)
				baseTexture->Apply(0);
		}
	};

	static DecalMaterial createMaterial(Storm3D_Material *material, Tree *tree)
	{
		DecalMaterial result(tree);
		result.diffuseColor = material->GetColor();

		Storm3D_Texture *base = static_cast<Storm3D_Texture *> (material->GetBaseTexture());
		if(base)
			result.baseTexture = createSharedTexture(base);

		return result;
	}

	static bool close(const COL &a, const COL &b)
	{
		if(fabsf(a.r - b.r) > 0.01f)
			return false;
		if(fabsf(a.g - b.g) > 0.01f)
			return false;
		if(fabsf(a.b - b.b) > 0.01f)
			return false;

		return true;
	}

	static bool equals(const DecalMaterial &a, const DecalMaterial &b)
	{
		if(a.baseTexture != b.baseTexture)
			return false;
		if(!close(a.diffuseColor, b.diffuseColor))
			return false;

		return true;
	}

	struct DecalSorter
	{
		bool operator() (const StormDecal *a, const StormDecal *b) const
		{
			if(a->materialIndex == b->materialIndex)
				return a < b;
			else
				return a->materialIndex < b->materialIndex;
		}
	};

	typedef std::vector<StormDecal *> DecalPtrList;

struct Storm3D_TerrainDecalSystem::Data
{
	Storm3D &storm;
	DecalMaterialList materials;
	VC2I blockAmount;

	PixelShader pixelShader;
	VertexShader vertexShader;
	VertexShader pointVertexShader;
	VertexShader dirVertexShader;
	VertexShader flatVertexShader;

	VertexBuffer vertices;
	VertexBuffer shadowVertices;
	IndexBuffer indices;

	scoped_ptr<Tree> tree;
	DecalPtrList decals;
	StormDecalList shadowDecals;

	COL outFactor;
	COL inFactor;

	shared_ptr<DecalMaterial> shadowMaterial;

	float fogEnd;
	float fogRange;

	Data(Storm3D &storm_)
	:	storm(storm_),
		pixelShader(),
		vertexShader(),
		pointVertexShader(),
		dirVertexShader(),
		flatVertexShader(),
		tree(0),
		outFactor(1.f, 1.f, 1.f),
		inFactor(1.f, 1.f, 1.f),
		fogEnd(-1000000000000.f),
		fogRange(10.f)
	{
		pixelShader.createDecalPixelShader();
		vertexShader.createDecalShader();

		pointVertexShader.createDecalPointShader();
		dirVertexShader.createDecalDirShader();
		flatVertexShader.createDecalFlatShader();
	}

	void setSceneSize(const VC3 &size)
	{
		assert(materials.empty());
		tree.reset(new Tree(VC2(-size.x, -size.z), VC2(size.x, size.z)));
	}

	int addMaterial(IStorm3D_Material *stormMaterial)
	{
		assert(stormMaterial);
		DecalMaterial material = createMaterial(static_cast<Storm3D_Material *> (stormMaterial), tree.get());

		for(unsigned int i = 0; i < materials.size(); ++i)
		{
			if(equals(materials[i], material))
				return i;
		}

		int index = materials.size();
		material.materialIndex = index;

		materials.push_back(material);
		return index;
	}

	int addDecal(int materialIndex, Type type, const VC3 &position, int &id, bool forceSpawn)
	{
		assert(materialIndex >= 0 && materialIndex < int(materials.size()));

		DecalMaterial &material = materials[materialIndex];
		return material.addDecal(type, position, id, forceSpawn);
	}

	void eraseDecal(int materialIndex, int decalIndex, int id)
	{
		if(!id)
			return;

		DecalMaterial &material = materials[materialIndex];
		assert(decalIndex >= 0 && decalIndex < int(material.decals.size()));

		material.eraseDecal(decalIndex, id);
	}

	void setRotation(int materialIndex, int decalIndex, int id, const QUAT &rotation)
	{
		DecalMaterial &material = materials[materialIndex];
		assert(decalIndex >= 0 && decalIndex < int(material.decals.size()));
		
		StormDecal &decal = material.decals[decalIndex];
		if(decal.id != id)
			return;

		decal.rotation = rotation;
		material.updateDecal(decalIndex);
	}

	void setSize(int materialIndex, int decalIndex, int id, const VC2 &size)
	{
		DecalMaterial &material = materials[materialIndex];
		assert(decalIndex >= 0 && decalIndex < int(material.decals.size()));
		
		StormDecal &decal = material.decals[decalIndex];
		if(decal.id != id)
			return;

		decal.size = size;
		material.updateDecal(decalIndex);
	}

	void setAlpha(int materialIndex, int decalIndex, int id, float alpha)
	{
		DecalMaterial &material = materials[materialIndex];
		assert(decalIndex >= 0 && decalIndex < int(material.decals.size()));
		
		if(alpha < 0.f)
			alpha = 0.f;
		if(alpha > 1.f)
			alpha = 1.f;

		StormDecal &decal = material.decals[decalIndex];
		if(decal.id != id)
			return;

		decal.alpha = int(alpha * 255.f);
		material.updateDecal(decalIndex);
	}

	void setLighting(int materialIndex, int decalIndex, int id, const COL &color)
	{
		DecalMaterial &material = materials[materialIndex];
		assert(decalIndex >= 0 && decalIndex < int(material.decals.size()));
		
		StormDecal &decal = material.decals[decalIndex];
		if(decal.id != id)
			return;

		decal.light = color;
		material.updateDecal(decalIndex);
	}

	void setShadowDecal(const VC3 &position, const QUAT &rotation, const VC2 &size, float alpha)
	{
		StormDecal decal;
		decal.position = position;
		decal.size = size;
		decal.alpha = (unsigned char)(alpha * 255.f);
		decal.rotation = rotation;
		decal.light = COL(1.f - alpha, 1.f - alpha, 1.f - alpha);

		decal.calculateValues();
		shadowDecals.push_back(decal);
	}

	void findDecals(Storm3D_Scene &scene)
	{
		decals.clear();

		// Find decals
		{
			IStorm3D_Camera *camera = scene.GetCamera();
			Storm3D_Camera *stormCamera = reinterpret_cast<Storm3D_Camera *> (camera);

			Frustum frustum = stormCamera->getFrustum();
			Tree::FrustumIterator itf(*tree, frustum);
			for(; !itf.end(); itf.next())
				decals.push_back(*itf);

			std::sort(decals.begin(), decals.end(), DecalSorter());
		}
	}

	void createIndexBuffers()
	{
		if(!indices)
		{
			indices.create(STORM_MAX_DECAL_AMOUNT * 2, false);
			unsigned short int *buffer = indices.lock();

			for(int i = 0; i < STORM_MAX_DECAL_AMOUNT; ++i)
			{
				unsigned short int base = i * 4;
				int index = i * 6;

				buffer[index] = base;
				buffer[index + 1] = base + 2;
				buffer[index + 2] = base + 1;
				buffer[index + 3] = base + 1;
				buffer[index + 4] = base + 2;
				buffer[index + 5] = base + 3;
			}

			indices.unlock();
		}
	}

	void createVertexBuffers()
	{
		if(!decals.empty())
		{
			vertices.create(decals.size() * 4, VERTEX_SIZE, true);

			int rawSize = decals.size() * 4 * VERTEX_SIZE;
			void *ramBuffer = malloc(rawSize);
			void *lockPointer = vertices.lock();
			VXFORMAT_DECAL *buffer = reinterpret_cast<VXFORMAT_DECAL *> (ramBuffer);

			for(unsigned int i = 0; i < decals.size(); ++i)
			{
				decals[i]->insert(buffer);
				buffer += 4;
			}

			memcpy(lockPointer, ramBuffer, rawSize);
			free(ramBuffer);

			vertices.unlock();
		}
	}

	void render(Storm3D_Scene &scene)
	{
		findDecals(scene);

		createIndexBuffers();
		createVertexBuffers();

		// Render
		if(!decals.empty())
		{
			glDepthMask(GL_FALSE);
			glEnable(GL_BLEND);
			glDisable(GL_ALPHA_TEST);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			D3DXMATRIX tm;

			pixelShader.apply();
			vertexShader.apply();
			vertices.apply(0);
			// ugly HACK!
			int prevBindOffs = 0;
			Storm3D_ShaderManager::GetSingleton()->SetWorldTransform(tm);

			float outfactor[4] = { outFactor.r, outFactor.g, outFactor.b, 1.0f };
			glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 8, outfactor);
			float infactor[4] = { inFactor.r, inFactor.g, inFactor.b, 1.0f };
			glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 9, infactor);

			int materialIndex = 0;
			int startIndex = 0;
			int endIndex = 0;

			for(;;)
			{
				materialIndex = decals[startIndex]->materialIndex;
				materials[materialIndex].apply();

				int decalAmount = decals.size();
				for(int i = startIndex + 1; i < decalAmount; ++i)
				{
					if(decals[i]->materialIndex != materialIndex)
						break;

					endIndex = i;
				}

				int renderAmount = endIndex - startIndex + 1;

				if (prevBindOffs != startIndex * 4) {
					vertices.apply(0, (startIndex * 4) * VERTEX_SIZE);
					prevBindOffs = startIndex * 4;
				}
				indices.render(renderAmount * 2, renderAmount * 4);
				startIndex = ++endIndex;

				scene.AddPolyCounter(renderAmount * 2);
				if(startIndex >= decalAmount)
					break;
			}

			glDisable(GL_BLEND);
		}
	}

	void renderShadows(Storm3D_Scene &scene)
	{
		// this is BROKEN
		// FIXME
		// just clear the list
		shadowDecals.clear();
		return;

		createIndexBuffers();

		int renderAmount = 0;
		if(!shadowDecals.empty())
		{
			IStorm3D_Camera *camera = scene.GetCamera();
			Storm3D_Camera *stormCamera = reinterpret_cast<Storm3D_Camera *> (camera);
			Frustum frustum = stormCamera->getFrustum();

			shadowVertices.create(shadowDecals.size() * 4, VERTEX_SIZE, true);

			int rawSize = shadowDecals.size() * 4 * VERTEX_SIZE;
			void *ramBuffer = malloc(rawSize);
			void *lockPointer = shadowVertices.lock();
			VXFORMAT_DECAL *buffer = reinterpret_cast<VXFORMAT_DECAL *> (ramBuffer);

			float inverseRange =1.f / fogRange;
			for(unsigned int i = 0; i < shadowDecals.size(); ++i)
			{
				const StormDecal &decal = shadowDecals[i];
				Sphere sphere(decal.position, decal.getRadius());
				if(frustum.visibility(sphere))
				{
					float factor = decal.position.y - fogEnd;
					factor *= inverseRange;
					if(factor < 0.f)
						factor = 0.f;
					if(factor > 1.f)
						factor = 1.f;
					factor = 1.f - factor;

					COL color = decal.light;
					color.r += factor * (1.f - color.r);
					color.g += factor * (1.f - color.g);
					color.b += factor * (1.f - color.b);

					DWORD vertexColor = color.GetAsD3DCompatibleARGB() & 0x00FFFFFF;

					DWORD oldColor = decal.vertexColor;
					decal.vertexColor = vertexColor;
					decal.insert(buffer);
					decal.vertexColor = oldColor;

					buffer += 4;
					++renderAmount;
				}
			}

			if(renderAmount)
			{
				if(renderAmount > STORM_MAX_DECAL_AMOUNT)
					renderAmount = STORM_MAX_DECAL_AMOUNT;
				memcpy(lockPointer, ramBuffer, renderAmount * 4 * VERTEX_SIZE);
			}

			free(ramBuffer);
			shadowVertices.unlock();
		}

		D3DXMATRIX tm;

		frozenbyte::storm::VertexShader::disable();
		frozenbyte::storm::PixelShader::disable();
		Storm3D_ShaderManager::GetSingleton()->SetWorldTransform(tm);

		applyFVF(DECAL_FVF, VERTEX_SIZE);
		glActiveTexture(GL_TEXTURE0);
		glClientActiveTexture(GL_TEXTURE0);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
		glActiveTexture(GL_TEXTURE1);
		glClientActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_3D);
		glDisable(GL_TEXTURE_CUBE_MAP);

		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
		glBlendFunc(GL_ZERO, GL_SRC_COLOR);

		if(renderAmount && shadowMaterial)
		{
			shadowVertices.apply(0);
			shadowMaterial->applyShadow();

			indices.render(renderAmount * 2, renderAmount * 4);
			scene.AddPolyCounter(renderAmount * 2);
		}

		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		shadowDecals.clear();
	}

	void renderProjection(Storm3D_Scene &scene, Storm3D_Spotlight *spot)
	{
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);

		if(!decals.empty())
		{
			glDepthMask(GL_FALSE);
			glEnable(GL_BLEND);
			glDisable(GL_ALPHA_TEST);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);

			D3DXMATRIX tm;

			int materialIndex = 0;
			int startIndex = 0;
			int endIndex = 0;

			if(spot->getType() == IStorm3D_Spotlight::Point)
				pointVertexShader.apply();
			else if(spot->getType() == IStorm3D_Spotlight::Directional)
				dirVertexShader.apply();
			else if(spot->getType() == IStorm3D_Spotlight::Flat)
				flatVertexShader.apply();

			vertices.apply(0);
			int prevBindOffs = 0;

			Storm3D_ShaderManager::GetSingleton()->SetWorldTransform(tm);
			Storm3D_ShaderManager::GetSingleton()->SetTransparencyFactor(0.75f);

			for(;;)
			{
				materialIndex = decals[startIndex]->materialIndex;
				materials[materialIndex].applyProjection(spot->getColorMultiplier());

				int decalAmount = decals.size();
				for(int i = startIndex + 1; i < decalAmount; ++i)
				{
					if(decals[i]->materialIndex != materialIndex)
						break;

					endIndex = i;
				}

				int renderAmount = endIndex - startIndex + 1;

				if (prevBindOffs != startIndex * 4) {
					vertices.apply(0, (startIndex * 4) * VERTEX_SIZE);
					prevBindOffs = startIndex * 4;
				}
				indices.render(renderAmount * 2, renderAmount * 4);
				startIndex = ++endIndex;

				scene.AddPolyCounter(renderAmount * 2);
				if(startIndex >= decalAmount)
					break;
			}

			Storm3D_ShaderManager::GetSingleton()->SetTransparencyFactor(1.f);

			glBlendFunc(GL_ONE, GL_ONE);
			glEnable(GL_BLEND);
		}

		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
	}

	void releaseDynamic()
	{
		vertices.release();
		shadowVertices.release();
		indices.release();
	}

	void recreateDynamic()
	{
	}
};

//! Constructor
Storm3D_TerrainDecalSystem::Storm3D_TerrainDecalSystem(Storm3D &storm)
:	data(new Data(storm))
{
}

//! Destructor
Storm3D_TerrainDecalSystem::~Storm3D_TerrainDecalSystem()
{
}

//! Set scene size
/*!
	\param size scene size
*/
void Storm3D_TerrainDecalSystem::setSceneSize(const VC3 &size)
{
	data->setSceneSize(size);
}

//! Add material
/*!
	\param material material
	\return
*/
int Storm3D_TerrainDecalSystem::addMaterial(IStorm3D_Material *material)
{
	return data->addMaterial(material);
}

//! Add decal
/*!
	\param materialId material ID
	\param type decal type
	\param position position
	\param id ID
	\param forceSpawn
	\return
*/
int Storm3D_TerrainDecalSystem::addDecal(int materialId, Type type, const VC3 &position, int &id, bool forceSpawn)
{
	return data->addDecal(materialId, type, position, id, forceSpawn);
}

//! Erase decal
/*!
	\param materialId material ID
	\param decalId decal ID
	\param id ID
*/
void Storm3D_TerrainDecalSystem::eraseDecal(int materialId, int decalId, int id)
{
	if(!id)
		return;

	data->eraseDecal(materialId, decalId, id);
}

//! Set rotation
/*!
	\param materialId material ID
	\param decalId decal ID
	\param id ID
	\param rotation rotation
*/
void Storm3D_TerrainDecalSystem::setRotation(int materialId, int decalId, int id, const QUAT &rotation)
{
	data->setRotation(materialId, decalId, id, rotation);
}

//! Set size
/*!
	\param materialId material ID
	\param decalId decal ID
	\param id ID
	\param size size
*/
void Storm3D_TerrainDecalSystem::setSize(int materialId, int decalId, int id, const VC2 &size)
{
	data->setSize(materialId, decalId, id, size);
}

//! Set alpha
/*!
	\param materialId material ID
	\param decalId decal ID
	\param id ID
	\param alpha alpha
*/
void Storm3D_TerrainDecalSystem::setAlpha(int materialId, int decalId, int id, float alpha)
{
	data->setAlpha(materialId, decalId, id, alpha);
}

//! Set lighting color
/*!
	\param materialId material ID
	\param decalId decal ID
	\param id ID
	\param color color
*/
void Storm3D_TerrainDecalSystem::setLighting(int materialId, int decalId, int id, const COL &color)
{
	data->setLighting(materialId, decalId, id, color);
}

//! Set lightmap factor
/*!
	\param factor factor
*/
void Storm3D_TerrainDecalSystem::setLightmapFactor(const COL &factor)
{
	data->inFactor = factor;
}

//! Set outdoor lightmap factor
/*!
	\param factor factor
*/
void Storm3D_TerrainDecalSystem::setOutdoorLightmapFactor(const COL &factor)
{
	data->outFactor = factor;
}

//! Set fog properties
/*!
	\param end fog end
	\param range fog range
*/
void Storm3D_TerrainDecalSystem::setFog(float end, float range)
{
	data->fogEnd = end;
	data->fogRange = range;
}

//! Render decal textures
/*!
	\param scene Storm3D scene
*/
void Storm3D_TerrainDecalSystem::renderTextures(Storm3D_Scene &scene)
{
	data->render(scene);
}

//! Render shadows
/*!
	\param scene Storm3D scene
*/
void Storm3D_TerrainDecalSystem::renderShadows(Storm3D_Scene &scene)
{
	data->renderShadows(scene);
}

//! Render spotlight projection
/*!
	\param scene Storm3D scene
	\param spot spotlight
*/
void Storm3D_TerrainDecalSystem::renderProjection(Storm3D_Scene &scene, Storm3D_Spotlight *spot)
{
	data->renderProjection(scene, spot);
}

//! Set shadow material
/*!
	\param material material
*/
void Storm3D_TerrainDecalSystem::setShadowMaterial(IStorm3D_Material *material)
{
	assert(data->tree);
	Storm3D_Material *m = static_cast<Storm3D_Material *> (material);

	DecalMaterial newMaterial = createMaterial(m, data->tree.get());
	data->shadowMaterial.reset(new DecalMaterial(newMaterial));
}

//! Set shadow decal
/*!
	\param position position
	\param rotation rotation
	\param size size
	\param alpha alpha value
*/
void Storm3D_TerrainDecalSystem::setShadowDecal(const VC3 &position, const QUAT &rotation, const VC2 &size, float alpha)
{
	data->setShadowDecal(position, rotation, size, alpha);
}

//! Clear shadow decals
void Storm3D_TerrainDecalSystem::clearShadowDecals()
{
	data->shadowDecals.clear();
}

//! Release dynamic resources
void Storm3D_TerrainDecalSystem::releaseDynamicResources()
{
	data->releaseDynamic();
}

//! Recreate dynamic resources
void Storm3D_TerrainDecalSystem::recreateDynamicResources()
{
	data->recreateDynamic();
}
