// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

#include <GL/glew.h>
#include <queue>
#include <vector>
#include <boost/scoped_ptr.hpp>
#include <c2_common.h>
#include <c2_qtree.h>
#include <c2_oobb.h>

#include "storm3d_terrain_models.h"
#include "storm3d_terrain_utils.h"
#include "storm3d_spotlight.h"
#include "storm3d_fakespotlight.h"
#include "storm3d_model.h"
#include "storm3d_model_object.h"
#include "storm3d_helper.h"
#include "storm3d_scene.h"
#include "storm3d_camera.h"
#include "storm3d_mesh.h"
#include "storm3d_material.h"
#include "storm3d_texture.h"
#include "storm3d.h"
#include "Storm3D_ShaderManager.h"
#include <boost/lexical_cast.hpp>

#include "../../util/Debug_MemoryManager.h"



#define NORMAL_ALPHA_TEST_VALUE 0x50


// MSC6 seems to mess up object list sorting if this on
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
//#pragma optimize("", off)
#endif

int storm3d_model_objects_tested = 0;
int storm3d_model_objects_rough_passed = 0;
int storm3d_model_objects_passed = 0;
using namespace frozenbyte::storm;

static const int MAX_VISIBILITY_STRUCTURES = 2;
int active_visibility = 0;

	const int MODEL_UPDATE_RATE = 2;
	const int OBJECT_UPDATE_RATE = 2;
	const int OBJECT_CULL_RATE = 4;

	struct DataBase
	{
		virtual void update(Storm3D_Model_Object *object) = 0;
		virtual void erase(const Storm3D_Model_Object *object) = 0;
		virtual ~DataBase() {};
	};

namespace {

	struct ModelObserver: public IModelObserver
	{
		Quadtree<Storm3D_Model> *tree;
		Quadtree<Storm3D_Model>::Entity *entity;
		std::vector<Storm3D_Model *> *visibleModels[MAX_VISIBILITY_STRUCTURES];
		DataBase *data;

		ModelObserver(Quadtree<Storm3D_Model> *tree_, Quadtree<Storm3D_Model>::Entity *entity_, std::vector<Storm3D_Model *> *visibleModels_, DataBase *data_)
		:	tree(tree_),
			entity(entity_),
			data(data_)
		{
			for(int i = 0; i < MAX_VISIBILITY_STRUCTURES; ++i)
				visibleModels[i] = &visibleModels_[i];
		}

		~ModelObserver()
		{
			destroy(0);
		}

		void updatePosition(const VC3 &position)
		{
			entity->setPosition(position);
		}

		void updateRadius(float radius)
		{
			entity->setRadius(radius);
		}

		void updateVisibility(Storm3D_Model_Object *object)
		{
			data->update(object);
		}

		void remove(Storm3D_Model *model)
		{
			for(int j = 0; j < MAX_VISIBILITY_STRUCTURES; ++j)
			{
				for(unsigned int i = 0; i < visibleModels[j]->size(); ++i)
				{
					if((*visibleModels[j])[i] == model)
					{
						visibleModels[j]->erase(visibleModels[j]->begin() + i);
						break;
					}
				}
			}
		}

		void remove(Storm3D_Model_Object *object)
		{
			data->erase(object);
		}

		void destroy(Storm3D_Model *model)
		{
			tree->erase(entity);
		}
	};

	typedef std::vector<Storm3D_Model_Object *> ModelObjectList;

	enum RenderType
	{
		BaseTextures,
		BaseLighting,
		Depth,
		FakeDepth,
		SpotProjection,
		FakeProjection,
		Glow,
		Distortion
	};

	struct SolidObjectSorter:
		binary_function<Storm3D_Model_Object *, Storm3D_Model_Object *, bool>
	{
		VC3 camera;

		explicit SolidObjectSorter(const VC3 &camera_)
		:	camera(camera_)
		{
		}

		bool operator() (Storm3D_Model_Object *a, Storm3D_Model_Object *b) const
		{
			if(!a || !b)
				return a < b;

			IStorm3D_Mesh *amesh = a->GetMesh();
			IStorm3D_Mesh *bmesh = b->GetMesh();
			if(!amesh || !bmesh)
				return amesh < bmesh;

			Storm3D_Material *am = static_cast<Storm3D_Material *> (amesh->GetMaterial());
			Storm3D_Material *bm = static_cast<Storm3D_Material *> (bmesh->GetMaterial());
			if(!am || !bm)
				return am < bm;

			int aalpha = am->GetAlphaType();
			int balpha = bm->GetAlphaType();
			if(aalpha || balpha)
				return aalpha < balpha;

			void *at1 = am->GetBaseTexture();
			void *at2 = am->GetBaseTexture2();
			void *at3 = am->GetReflectionTexture();
			void *bt1 = bm->GetBaseTexture();
			void *bt2 = bm->GetBaseTexture2();
			void *bt3 = bm->GetReflectionTexture();

			if(at2 != bt2)
				return at2 < bt2;
			if(at3 != bt3)
				return at3 < bt3;
			if(at1 != bt1)
				return at1 < bt1;

			return a->sort_data < b->sort_data;
		}
	};

	struct AlphaObjectSorter:
		binary_function<Storm3D_Model_Object *, Storm3D_Model_Object *, bool>
	{
		VC3 camera;

		explicit AlphaObjectSorter(const VC3 &camera_)
		:	camera(camera_)
		{
		}

		bool operator() (Storm3D_Model_Object *a, Storm3D_Model_Object *b) const
		{
			// Just prevent flashing, distance sort crashes on VC6 for some unknown reason 
			// (only in release mode, when optimizations are enabled
			//  -> STLport/compiler bug?

			assert(a && b);
			if(!a || !b)
				return a < b;

			// Seems to work if we dont calculate actual distance here
			return a->sort_data > b->sort_data;
		}
	};

	enum RenderFlags
	{
		None,
		SkipTerrainObjects,
		SkipTerrainLightmappedObjects
	};

	bool contains2d(const VC2 &position, float radius, const VC2 &minp, const VC2 &maxp)
	{
		if(position.x + radius < minp.x)
			return false;
		if(position.x - radius > maxp.x)
			return false;
		if(position.y + radius < minp.y)
			return false;
		if(position.y - radius > maxp.y)
			return false;

		return true;
	}
}

struct Storm3D_TerrainModelsData : public DataBase
{
	Storm3D &storm;
	std::set<Storm3D_Model *> models;

	boost::scoped_ptr<Quadtree<Storm3D_Model> > tree;
	std::map<Storm3D_Model *, boost::shared_ptr<IModelObserver> > observers;

	std::vector<TerrainLight> lights;

	PixelShader lightingPixelShader_lightmap;
	PixelShader lightingPixelShader_lightmap_reflection;
	PixelShader lightingPixelShader_lightmap_localreflection;
	PixelShader lightingPixelShader_lightmap_notexture;
	PixelShader lightingPixelShader_noLightmap;
	PixelShader lightingPixelShader_noLightmap_reflection;
	PixelShader lightingPixelShader_noLightmap_localreflection;
	PixelShader lightingPixelShader_noLightmap_notexture;

	enum ObjectType
	{
		NormalObjects = 0,
		DepthObjects = 1
	};

	ModelObjectList solidObjects[MAX_VISIBILITY_STRUCTURES][2];
	ModelObjectList alphaObjects[MAX_VISIBILITY_STRUCTURES][2];

	Storm3D_TerrainModels::FillMode fillMode;
	bool filterLightmap;
	bool renderCollision;
	bool enableAlphaTest;
	bool renderBoned;
	bool enableMaterialAmbient;
	bool forceWhiteBase;
	bool enableGlow;
	bool enableDistortion;
	bool enableReflection;
	bool additionalAlphaTestPassAllowed;
	bool skyModelGlowAllowed;

	std::vector<Storm3D_Model *> visibleModels[MAX_VISIBILITY_STRUCTURES];

	COL terrainObjectColorFactorBuilding;
	COL terrainObjectColorFactorOutside;

	bool frustumsEnabled;
	Frustum lastFrustum[MAX_VISIBILITY_STRUCTURES];
	Frustum lastRealFrustum[MAX_VISIBILITY_STRUCTURES];
	VC3 lastUpdateCameraPosition[MAX_VISIBILITY_STRUCTURES];
	VC3 lastUpdateCameraTarget[MAX_VISIBILITY_STRUCTURES];
	int object_counter[MAX_VISIBILITY_STRUCTURES];
	int model_counter[MAX_VISIBILITY_STRUCTURES];

	bool foundLocalReflection;
	float localReflectionHeight;

	bool forcedDirectionalLightEnabled;
	VC3 forcedDirectionalLightDirection;
	float forcedDirectionalLightStrength;

	std::vector<Storm3D_Model *> disabledCullingModels;

	Storm3D_TerrainModelsData(Storm3D &storm_)
	:	storm(storm_),
		lightingPixelShader_lightmap(),
		lightingPixelShader_lightmap_reflection(),
		lightingPixelShader_lightmap_localreflection(),
		lightingPixelShader_lightmap_notexture(),
		lightingPixelShader_noLightmap(),
		lightingPixelShader_noLightmap_reflection(),
		lightingPixelShader_noLightmap_localreflection(),
		lightingPixelShader_noLightmap_notexture(),
		fillMode(Storm3D_TerrainModels::Solid),
		filterLightmap(false),
		renderCollision(false),
		enableAlphaTest(true),
		renderBoned(true),
		enableMaterialAmbient(true),
		forceWhiteBase(false),
		enableGlow(true),
		enableDistortion(false),
		enableReflection(false),
		additionalAlphaTestPassAllowed(false),
		skyModelGlowAllowed(true),
		terrainObjectColorFactorBuilding(1.f, 1.f, 1.f),
		terrainObjectColorFactorOutside(1.f, 1.f, 1.f),
		frustumsEnabled(false),
		foundLocalReflection(false),
		localReflectionHeight(0.f),

		forcedDirectionalLightEnabled(false),
		forcedDirectionalLightDirection(0, 1.f, 0),
		forcedDirectionalLightStrength(0.f)
	{
		for(int i = 0; i < MAX_VISIBILITY_STRUCTURES; ++i)
		{
			object_counter[i] = 0;
			model_counter[i] = 0;
		}

		lightingPixelShader_lightmap.createLightingPixelShader_Lightmap();
		lightingPixelShader_lightmap_reflection.createLightingPixelShader_Lightmap_Reflection();
		lightingPixelShader_lightmap_localreflection.createLightingPixelShader_Lightmap_LocalReflection();
		lightingPixelShader_lightmap_notexture.createLightingPixelShader_LightmapNoTexture();
		lightingPixelShader_noLightmap.createLightingPixelShader_NoLightmap();
		lightingPixelShader_noLightmap_reflection.createLightingPixelShader_NoLightmap_Reflection();
		lightingPixelShader_noLightmap_localreflection.createLightingPixelShader_NoLightmap_LocalReflection();
		lightingPixelShader_noLightmap_notexture.createLightingPixelShader_NoLightmapNoTexture();
	}

	~Storm3D_TerrainModelsData()
	{
		eraseModelTree();
	}

	void eraseModelTree()
	{
		observers.clear();

		std::set<Storm3D_Model *>::iterator it = models.begin();
		for(; it != models.end(); ++it)
		{
			Storm3D_Model *model = *it;
			model->observer = 0;
		}
	}

	void insert(Storm3D_Model *model)
	{
		if(!tree)
			return;

		Quadtree<Storm3D_Model>::Entity *entity = tree->insert(model, model->GetPosition(), model->bounding_radius);
		boost::shared_ptr<ModelObserver> observer(new ModelObserver(tree.get(), entity, &visibleModels[0], this));
		observers[model] = observer;

		model->observer = observer.get();

		if(frustumsEnabled)
		{
			for(int i = 0; i < MAX_VISIBILITY_STRUCTURES; ++i)
			{
				visibleModels[i].push_back(model);

				std::vector<Storm3D_Model_Object *>::iterator it = model->objects_array.begin();
				std::vector<Storm3D_Model_Object *>::iterator end = model->objects_array.end();
				for(; it != end; ++it)
				{
					Storm3D_Model_Object *o = static_cast<Storm3D_Model_Object *> (*it);
					if(!o)
						continue;
					Storm3D_Mesh *mesh = static_cast<Storm3D_Mesh *> (o->GetMesh());
					if(!mesh)
						continue;

					o->visibilityFlag = 0;
					if(!testVisibility(*o, *mesh, *model, lastFrustum[i]))
						continue;

					Storm3D_Material *material = static_cast<Storm3D_Material *> (mesh->GetMaterial());
					if(!material)
						continue;

					int alphaType = material->GetAlphaType();
					bool hasAlpha = false;
					if(alphaType != IStorm3D_Material::ATYPE_NONE && alphaType != IStorm3D_Material::ATYPE_USE_ALPHATEST)
						hasAlpha = true;
					if(o->force_alpha > 0.0001f || o->force_lighting_alpha_enable)
						hasAlpha = true;


					if(testVisibility(*o, *mesh, *model, lastRealFrustum[i]))
					{
						if(hasAlpha)
						{
							alphaObjects[i][NormalObjects].push_back(o);
							if(o->renderPassMask & (1<<RENDER_PASS_BIT_ADDITIONAL_ALPHA_TEST_PASS)
								&& (additionalAlphaTestPassAllowed || !o->alphaTestPassConditional))
								solidObjects[i][NormalObjects].push_back(o);
						}
						else
							solidObjects[i][NormalObjects].push_back(o);
						
					}
					
					if(!hasAlpha)
						solidObjects[i][DepthObjects].push_back(o);
				}
			}
		}
	}

	void update(Storm3D_Model_Object *o)
	{
		if(!o)
			return;
		Storm3D_Mesh *mesh = static_cast<Storm3D_Mesh *> (o->GetMesh());
		if(!mesh)
			return;

		o->visibilityFlag = 0;
		
		for(int i = 0; i < MAX_VISIBILITY_STRUCTURES; ++i)
		{
			Storm3D_Model *m = o->parent_model;
			if(!testVisibility(*o, *mesh, *m, lastFrustum[i]))
				continue;

			Storm3D_Material *material = static_cast<Storm3D_Material *> (mesh->GetMaterial());
			if(!material)
				return;

			int alphaType = material->GetAlphaType();
			bool hasAlpha = false;
			if(alphaType != IStorm3D_Material::ATYPE_NONE && alphaType != IStorm3D_Material::ATYPE_USE_ALPHATEST)
				hasAlpha = true;
			if(o->force_alpha > 0.0001f || o->force_lighting_alpha_enable)
				hasAlpha = true;

			bool passedTesting = false;
			if(testVisibility(*o, *mesh, *m, lastRealFrustum[i]))
				passedTesting = true;

			if(passedTesting)
			{
				if(hasAlpha)
				{
					alphaObjects[i][NormalObjects].push_back(o);
					if(o->renderPassMask & (1<<RENDER_PASS_BIT_ADDITIONAL_ALPHA_TEST_PASS)
						&& (additionalAlphaTestPassAllowed || !o->alphaTestPassConditional))
						solidObjects[i][NormalObjects].push_back(o);
				}
				else
					solidObjects[i][NormalObjects].push_back(o);
			}
			
			if(!hasAlpha)
				solidObjects[i][DepthObjects].push_back(o);
		}
	}

	void erase(const Storm3D_Model_Object *object)
	{
		for(int j = 0; j < MAX_VISIBILITY_STRUCTURES; ++j)
		for(int i = 0; i < 2; ++i)
		{
			{
				ModelObjectList &solidList = solidObjects[j][i];
				if(!solidList.empty())
				{
					ModelObjectList::iterator it = solidList.begin();
					for(; it != solidList.end(); ++it)
					{
						if(*it == object)
							*it = 0;
					}
				}
			}

			{
				ModelObjectList &alphaList = alphaObjects[j][i];
				if(!alphaList.empty())
				{
					ModelObjectList::iterator it = alphaList.begin();
					for(; it != alphaList.end(); ++it)
					{
						if(*it == object)
							*it = 0;
					}
				}
			}
		}
	}

	void erase(const Storm3D_Model *model)
	{
		for(int j = 0; j < MAX_VISIBILITY_STRUCTURES; ++j)
		{
			// Clear model list
			{
				std::vector<Storm3D_Model *>::iterator it = visibleModels[j].begin();
				for(; it != visibleModels[j].end(); ++it)
				{
					if(model == *it)
						*it = 0;
				}
			}

			// Clear object list
			{
				for(int i = 0; i < 2; ++i)
				{
					{
						ModelObjectList &solidList = solidObjects[j][i];
						if(!solidList.empty())
						{
							ModelObjectList::iterator it = solidList.begin();
							for(; it != solidList.end(); ++it)
							{
								Storm3D_Model_Object *o = *it;
								if(!o)
									continue;

								if(o->parent_model == model)
									*it = 0;
							}
						}
					}

					{
						ModelObjectList &alphaList = alphaObjects[j][i];
						if(!alphaList.empty())
						{
							ModelObjectList::iterator it = alphaList.begin();
							for(; it != alphaList.end(); ++it)
							{
								Storm3D_Model_Object *o = *it;
								if(!o)
									continue;

								if(o->parent_model == model)
									*it = 0;
							}
						}
					}
				}
			}
		}
	}

	void buildTree(const VC3 &size)
	{
		VC2 mmin(-size.x, -size.z);
		VC2 mmax = -mmin;

		eraseModelTree();

		boost::scoped_ptr<Quadtree<Storm3D_Model> > newTree(new Quadtree<Storm3D_Model> (mmin, mmax));
		tree.swap(newTree);

		std::set<Storm3D_Model *>::iterator it = models.begin();
		for(; it != models.end(); ++it)
			insert(*it);
	}

	bool testVisibility(Storm3D_Model_Object &o, Storm3D_Mesh &mesh, Storm3D_Model &model, const Frustum &frustum) const
	{
		if(renderCollision)
		{
			if(o.GetNoCollision())
				return false;
		}
		else
		{
			if(o.GetNoRender())
				return false;
		}

		Sphere objectSphere(o.GetGlobalPosition(), mesh.GetRadius() * model.max_scale);
		if(!frustum.visibility(objectSphere, true))
			return false;

		Sphere accurateSphere = o.GetBoundingSphere();
		if(o.SphereOk())
		{
			const MAT &modelTM = model.GetMX();
			modelTM.TransformVector(accurateSphere.position);

			if(!frustum.visibility(accurateSphere, true))
				return false;
		}

		{
			OOBB oobb = o.GetObjectBoundingBox();
			if(o.ObjectBoundingBoxOk())
			{
				const MAT &modelTM = o.GetMXG();
				modelTM.TransformVector(oobb.center);
				modelTM.RotateVector(oobb.axes[0]);
				modelTM.RotateVector(oobb.axes[1]);
				modelTM.RotateVector(oobb.axes[2]);

				if(!frustum.visibility(oobb))
					return false;
			}
		}

		return true;
	}

	void findVisibleModels(Storm3D_Camera &camera, Storm3D_Camera &haxCamera, int timeDelta_)
	{
		if(!tree)
			return;

		const Frustum &frustum = haxCamera.getFrustum();
		const Frustum &realFrustum = camera.getFrustum();
		static int frame = 0;
		if(active_visibility == 0)
			++frame;

		frustumsEnabled = true;
		lastFrustum[active_visibility] = frustum;
		lastRealFrustum[active_visibility] = realFrustum;

		float timeDelta = timeDelta_ / 1000.f;
		string activeEffect;
		float closestEffect = 100000000000000000.f;

		VC3 cameraPosition = camera.GetPosition();
		bool forceCulling = false;
		if(cameraPosition.GetRangeTo(lastUpdateCameraPosition[active_visibility]) > 10.f)
		{
			lastUpdateCameraPosition[active_visibility] = cameraPosition;
			forceCulling = true;
		}

		VC3 cameraTarget = camera.GetTarget();
		if(cameraTarget.GetRangeTo(lastUpdateCameraTarget[active_visibility]) > 20.f)
		{
			lastUpdateCameraTarget[active_visibility] = cameraTarget;
			forceCulling = true;
		}

		// Models
		{
			if(forceCulling || ++model_counter[active_visibility] == MODEL_UPDATE_RATE)
			{
				model_counter[active_visibility] = 0;

				for(unsigned int i = 0; i < disabledCullingModels.size(); ++i)
					disabledCullingModels[i]->need_cull_adding = true;

				visibleModels[active_visibility].clear();
				Quadtree<Storm3D_Model>::FrustumIterator itf(*tree.get(), frustum);
				for(; !itf.end(); itf.next())
				{
					Storm3D_Model *m = *itf;

					// new, external occlusion culling --jpk
					if (m->GetOccluded())
					{
						continue;
					}
					
					m->need_cull_adding = false;
					visibleModels[active_visibility].push_back(m);
				}

				for(unsigned int i = 0; i < disabledCullingModels.size(); ++i)
				{
					Storm3D_Model *model = disabledCullingModels[i];
					if(model->need_cull_adding)
						visibleModels[active_visibility].push_back(model);
				}
			}
		}

		bool updateCulling = false;
		if(forceCulling || ++object_counter[active_visibility] == OBJECT_UPDATE_RATE)
		{
			if(active_visibility == 0)
			{
				storm3d_model_objects_tested = 0;
				storm3d_model_objects_passed = 0;
				storm3d_model_objects_rough_passed = 0;
			}

			object_counter[active_visibility] = 0;
			updateCulling = true;
		}

		{
			if(updateCulling)
			{
				solidObjects[active_visibility][DepthObjects].clear();
				solidObjects[active_visibility][NormalObjects].clear();
				alphaObjects[active_visibility][NormalObjects].clear();
			}

			std::vector<Storm3D_Model *>::iterator it = visibleModels[active_visibility].begin();
			std::vector<Storm3D_Model *>::iterator end = visibleModels[active_visibility].end();
			for(; it != end; ++it)
			{
				Storm3D_Model *m = *it;
				if(!m)
					continue;

				// Animate
				{
					m->AdvanceAnimation(timeDelta_);
				}

				if(updateCulling)
				{
					std::vector<Storm3D_Model_Object *>::iterator it = m->objects_array.begin();
					std::vector<Storm3D_Model_Object *>::iterator end = m->objects_array.end();
					for(; it != end; ++it)
					{
						++storm3d_model_objects_tested;
						Storm3D_Model_Object *o = static_cast<Storm3D_Model_Object *> (*it);
						if(!o)
							continue;
						Storm3D_Mesh *mesh = static_cast<Storm3D_Mesh *> (o->GetMesh());
						if(!mesh)
							continue;

						o->visibilityFlag = 0;

						bool needRoughTesting = o->visibility_id > frame || frame - o->visibility_id > OBJECT_CULL_RATE;
						bool passedRoughTesting = true;
						if(needRoughTesting)
						{
							if(testVisibility(*o, *mesh, *m, frustum))
							{
								o->visibility_id = frame;
								passedRoughTesting = true;
							}
							else
								passedRoughTesting = false;
						}

						if(!passedRoughTesting)
							continue;

						++storm3d_model_objects_rough_passed;
						Storm3D_Material *material = static_cast<Storm3D_Material *> (mesh->GetMaterial());
						if(!material)
							continue;

						int alphaType = material->GetAlphaType();
						bool hasAlpha = false;
						if(alphaType != IStorm3D_Material::ATYPE_NONE && alphaType != IStorm3D_Material::ATYPE_USE_ALPHATEST)
							hasAlpha = true;
						if(o->force_alpha > 0.0001f || o->force_lighting_alpha_enable)
							hasAlpha = true;

						bool needTesting = o->real_visibility_id > frame || frame - o->real_visibility_id > OBJECT_CULL_RATE;
						bool passedTesting = true;
						if(needTesting)
						{
							if(testVisibility(*o, *mesh, *m, realFrustum))
							{
								o->real_visibility_id = frame;
								passedTesting = true;
							}
							else
								passedTesting = false;
						}

						if(passedTesting)
						{
							++storm3d_model_objects_passed;

							material->updateScroll(timeDelta, frame);
							const Sphere &s = o->GetBoundingSphere();
							VC3 colPosition = s.position;
							const MAT &modelTM = m->GetMX();
							modelTM.TransformVector(colPosition);

							if(!material->getEffectTextureName().empty())
							{
								float dist = colPosition.GetSquareRangeTo(camera.GetPosition());
								if(dist < closestEffect)
								{
									activeEffect = material->getEffectTextureName();
									closestEffect = dist;
								}
							}

							o->sort_data = camera.GetPosition().GetSquareRangeTo(colPosition);

							if (o->renderPassMask != RENDER_PASS_MASK_VALUE_NONE)
							{
								// this is no longer necessary, handled at alpha rendering...
								//int passNumber = 0;
								//if (o->renderPassMask & (1<<RENDER_PASS_BIT_CAREFLECTION_DEPTH_MASKS))
								//	passNumber = 1;
								//if (o->renderPassMask & (1<<RENDER_PASS_BIT_CAREFLECTION_REFLECTED))
								//	passNumber = 2;
								//o->sort_data += ((passNumber * passNumber) * (10000.0f * 10000.0f));

								if (o->renderPassMask & (1<<RENDER_PASS_BIT_EARLY_ALPHA))
								{
									o->sort_data += (10000.0f * 10000.0f);
								}


								// this ok? - to get the CAReflection things rendered before alphablended stuff 
								// (as the reflection should really be "behind" water surface/glass stains, etc.)
								// and also, because of the multipass reflected hack...
								//hasAlpha = false;
								// nope, actually rather draw all as alpha - this is to avoid alpha objects not being rendered
								// behind depth masks...
								// also, additional alpha test passes do the same...
								if (o->renderPassMask & (1<<RENDER_PASS_BIT_ADDITIONAL_ALPHA_TEST_PASS))
								{
									if (additionalAlphaTestPassAllowed 
										|| !o->alphaTestPassConditional)
									{
										if (hasAlpha)
										{
											solidObjects[active_visibility][NormalObjects].push_back(o);
										}
									}
								} else {
									hasAlpha = true;
									if (o->renderPassMask & (1<<RENDER_PASS_BIT_CAREFLECTION_REFLECTED))
										solidObjects[active_visibility][NormalObjects].push_back(o);
								}
							}

							if(hasAlpha)
							{
								alphaObjects[active_visibility][NormalObjects].push_back(o);
							} else {
								solidObjects[active_visibility][NormalObjects].push_back(o);
							}
						}
						
						if(!hasAlpha)
						{
							if (!m->skyModel)
								solidObjects[active_visibility][DepthObjects].push_back(o);
						}
					}
				}
			}

			if(updateCulling)
			{
				VC3 cameraPosition = camera.GetPosition();
				SolidObjectSorter solidSorter(cameraPosition);

				AlphaObjectSorter alphaSorter(cameraPosition);

				for(int i = 0; i < 2; ++i)
				{
					ModelObjectList &solidList = solidObjects[active_visibility][i];
					if(!solidList.empty())
						sort(solidList.begin(), solidList.end(), solidSorter);

					ModelObjectList &alphaList = alphaObjects[active_visibility][i];
					if(!alphaList.empty())
						sort(alphaList.begin(), alphaList.end(), alphaSorter);
				}

				storm.getProceduralManagerImp().setActiveEffect(activeEffect);
			}
		}

		if(!updateCulling)
		{
			{
				ModelObjectList &solidList = solidObjects[active_visibility][0];
				if(!solidList.empty())
				{
					ModelObjectList::iterator it = solidList.begin();
					for(; it != solidList.end(); ++it)
					{
						Storm3D_Model_Object *o = *it;
						if(!o)
							continue;
						Storm3D_Mesh *mesh = static_cast<Storm3D_Mesh *> (o->GetMesh());
						if(!mesh)
							continue;
						Storm3D_Material *material = static_cast<Storm3D_Material *> (mesh->GetMaterial());
						if(!material)
							continue;

						material->updateScroll(timeDelta, frame);
					}
				}
			}

			{
				ModelObjectList &alphaList = alphaObjects[active_visibility][0];
				if(!alphaList.empty())
				{
					ModelObjectList::iterator it = alphaList.begin();
					for(; it != alphaList.end(); ++it)
					{
						Storm3D_Model_Object *o = *it;
						if(!o)
							continue;
						Storm3D_Mesh *mesh = static_cast<Storm3D_Mesh *> (o->GetMesh());
						if(!mesh)
							continue;
						Storm3D_Material *material = static_cast<Storm3D_Material *> (mesh->GetMaterial());
						if(!material)
							continue;

						material->updateScroll(timeDelta, frame);
					}
				}
			}
		}

		if(active_visibility == 0)
		{
			storm.enableLocalReflection(foundLocalReflection, localReflectionHeight);
			
			foundLocalReflection = false;
			localReflectionHeight = 0.f;
		}
	}

	bool shouldRender(RenderType renderType, Storm3D_Model_Object *object, const Frustum *frustum, const Frustum *frustum2, const IStorm3D_Model *skipModel, RenderFlags flags, const VC2 &minplane, const VC2 &maxplane, int activeVisibility) const
	{

		if(object->parent_model == skipModel)
			return false;

		if(renderCollision)
		{
			if(object->GetNoCollision())
				return false;
		}
		else
		{
			if(object->GetNoRender())
				return false;
		}

		if(flags == SkipTerrainObjects && object->parent_model->terrain_object)
			return false;
		if(flags == SkipTerrainLightmappedObjects && object->parent_model->terrain_object && object->parent_model->terrain_lightmapped_object)
			return false;

		if(object->isDistortionOnly() && renderType != Distortion)
			return false;

		if(frustum)
		{
			Storm3D_Mesh *mesh = static_cast<Storm3D_Mesh *> (object->GetMesh());
			if(!testVisibility(*object, *mesh, *object->parent_model, *frustum))
				return false;
		}
		if(frustum2)
		{
			Storm3D_Mesh *mesh = static_cast<Storm3D_Mesh *> (object->GetMesh());
			if(!testVisibility(*object, *mesh, *object->parent_model, *frustum2))
				return false;
		}

		if(renderType == FakeDepth)
		{
			if(object->parent_model->bones.empty())
			{
				if(object->parent_model->objects.size() > 5)
					return false;

				// Do not render if 2d bounding circle does not reach projection plane
				Storm3D_Mesh *mesh = static_cast<Storm3D_Mesh *> (object->GetMesh());
				const VC3 &pos3 = object->GetGlobalPosition();
				VC2 pos2(pos3.x, pos3.z);
				float radius2 = mesh->GetRadius();
				if(!contains2d(pos2, radius2, minplane, maxplane))
					return false;
			}
			else
			{
				// Do not render if 2d bounding circle does not reach projection plane
				const VC3 &pos3 = object->parent_model->GetPosition();
				VC2 pos2(pos3.x, pos3.z);
				float radius2 = object->parent_model->GetRadius();
				if(!contains2d(pos2, radius2, minplane, maxplane))
					return false;
			}
		}
		else if(renderType == FakeProjection)
		{
			if(!object->parent_model->bones.empty())
				return false;
		}

		if(renderType == FakeDepth || renderType == Depth)
		{
			if(!object->parent_model->cast_shadows)
				return false;

			std::string name = object->GetName();
			if(name.find("Decal") != name.npos)
				return false;
		}

		if(renderType == Glow)
		{
			Storm3D_Mesh *mesh = static_cast<Storm3D_Mesh *> (object->GetMesh());
			Storm3D_Material *material = static_cast<Storm3D_Material *> (mesh->GetMaterial());

			float glow = material->GetGlow() * material->GetGlowFactor();
			if(material->GetAlphaType() != IStorm3D_Material::ATYPE_NONE)
				glow *= 1.f - material->GetTransparency();

			if(glow <= 0.01f)
				return false;

			if (object->parent_model->skyModel && !skyModelGlowAllowed)
				return false;
		}

		if(renderType == Distortion)
		{
			Storm3D_Mesh *mesh = static_cast<Storm3D_Mesh *> (object->GetMesh());
			Storm3D_Material *material = static_cast<Storm3D_Material *> (mesh->GetMaterial());

			if(material->getEffectTextureName().empty())
			{
				if(!material->GetDistortionTexture())
					return false;
			}
			else
			{
				if(!storm.getProceduralManagerImp().hasDistortion())
					return false;
			}
		}

		if(!renderBoned && !object->parent_model->bones.empty())
			return false;

		if(activeVisibility == 1)
		{
			Storm3D_Mesh *mesh = static_cast<Storm3D_Mesh *> (object->GetMesh());
			Storm3D_Material *material = static_cast<Storm3D_Material *> (mesh->GetMaterial());
			if(material->hasLocalReflection())
				return false;
		}

		return true;
	}

	bool applyTexture(IStorm3D_Texture *texture, int stage) const
	{
		Storm3D_Texture *t = static_cast<Storm3D_Texture *> (texture);
		if(t)
		{
			t->AnimateVideo();
			t->Apply(stage);
			return true;
		}

		glActiveTexture(GL_TEXTURE0 + stage);
		glClientActiveTexture(GL_TEXTURE0 + stage);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_3D);
		glDisable(GL_TEXTURE_CUBE_MAP);
		glBindTexture(GL_TEXTURE_2D, 0);
		return false;
	}

	void applyGeneralMaterial(RenderType renderType, Storm3D_Material &material, Storm3D_Model &model, Storm3D_Model_Object &object)
	{
		Storm3D_ShaderManager *shaderManager = Storm3D_ShaderManager::GetSingleton();

		if(renderType == BaseTextures)
		{
			if(material.getEffectTextureName().empty())
				applyTexture(material.GetBaseTexture(), 0);
			else
				storm.getProceduralManagerImp().apply(0);

			shaderManager->SetObjectDiffuse(material.GetColor());
		}
		else if(renderType == BaseLighting)
		{
			if(material.getEffectTextureName().empty())
				applyTexture(material.GetBaseTexture(), 0);
			else
				storm.getProceduralManagerImp().apply(0);

			shaderManager->SetObjectDiffuse(material.GetColor());

			bool baseTex = true;
			if(!material.GetBaseTexture())
				baseTex = false;

			if(forceWhiteBase)
			{
				baseTex = false;
				shaderManager->SetObjectDiffuse(COL(1.f, 1.f, 1.f));
			}
			else
			{
				shaderManager->SetObjectDiffuse(material.GetColor());
			}

			bool localReflection = false;
			bool reflection = false;
			if(enableReflection && material.hasLocalReflection())
			{
				foundLocalReflection = true;
				Sphere sphere = object.GetBoundingSphere();
				const MAT &modelTM = model.GetMX();
				modelTM.TransformVector(sphere.position);

#ifdef PROJECT_CLAW_PROTO
				// Seems like using bounding sphere y breaks the ground reflection in claw...
				localReflectionHeight = model.GetPosition().y;
#else
				localReflectionHeight = sphere.position.y;
#endif

				if(applyTexture(storm.getReflectionTexture(), 3))
				{
					localReflection = true;

					float ref = material.getReflectionBlendFactor();
					float factor[4] = { ref, ref, ref, ref };
					glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 1, factor);
				}
			}
			else if(applyTexture(material.GetReflectionTexture(), 3))
			{
				float ref = material.GetReflectionFactor();
				reflection = true;

				float factor[4] = { ref, ref, ref, ref };
				glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 1, factor);
			}
/*  // done in shader
			if(localReflection)
				device.SetTextureStageState(3, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
			else
				device.SetTextureStageState(3, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
*/
			if(applyTexture(material.GetBaseTexture2(), 1))
			{
				if(baseTex)
				{
					if(localReflection)
						lightingPixelShader_lightmap_localreflection.apply();
					else if(reflection)
						lightingPixelShader_lightmap_reflection.apply();
					else
						lightingPixelShader_lightmap.apply();
				}
				else
					lightingPixelShader_lightmap_notexture.apply();
			}
			else
			{
				if(baseTex)
				{
					if(localReflection)
						lightingPixelShader_noLightmap_localreflection.apply();
					else if(reflection)
						lightingPixelShader_noLightmap_reflection.apply();
					else
						lightingPixelShader_noLightmap.apply();
				}
				else
					lightingPixelShader_noLightmap_notexture.apply();
			}

			COL colorFactor(1.f, 1.f, 1.f);
			if(model.terrain_object)
			{
				if(model.terrain_inbuilding_object)
					colorFactor = terrainObjectColorFactorBuilding;
				else
					colorFactor = terrainObjectColorFactorOutside;
			}

			shaderManager->SetModelAmbient(model.GetSelfIllumination() * colorFactor);

			if(enableMaterialAmbient)
			{
				if(enableGlow)
					shaderManager->SetObjectAmbient(material.GetSelfIllumination() / 2);
				else
				{
					float glow = material.GetGlow() * material.GetGlowFactor() * 0.5f;
					if(material.GetAlphaType() != material.ATYPE_NONE)
						glow *= 1.f - material.GetTransparency();

					shaderManager->SetObjectAmbient(material.GetSelfIllumination() * 0.5f + COL(glow, glow, glow));
				}
			}
			else
				shaderManager->SetObjectAmbient(COL());

			int light_amount = 0;
			if(object.light_index[0] == -1)
			{
				for(int i = 0; i < LIGHT_MAX_AMOUNT; ++i)
				{
					int index = model.light_index[i];
					if(index == -1 || index <  0 || index >= int(lights.size()))
					{
						shaderManager->SetLight(i, VC3(), COL(), 1.f);
					}
					else
					{
						TerrainLight &l = lights[index];

						COL lightColor = l.color * colorFactor;
						shaderManager->SetLight(i, l.position, lightColor, l.radius);

						if(lightColor.r > 0.001f || lightColor.g > 0.001f || lightColor.b > 0.001f)
							light_amount = i + 1;
					}
				}
			}
			else
			{
				for(int i = 0; i < LIGHT_MAX_AMOUNT; ++i)
				{
					int index = object.light_index[i];
					if(index == -1 || index <  0 || index >= int(lights.size()))
					{
						shaderManager->SetLight(i, VC3(), COL(), 1.f);
					}
					else
					{
						TerrainLight &l = lights[index];

						COL lightColor = l.color * colorFactor;
						shaderManager->SetLight(i, l.position, lightColor, l.radius);

						if(lightColor.r > 0.001f || lightColor.g > 0.001f || lightColor.b > 0.001f)
							light_amount = i + 1;
					}
				}
			}

			shaderManager->setLightingParameters(reflection, localReflection, light_amount);

			if(forcedDirectionalLightEnabled)
				shaderManager->SetSun(forcedDirectionalLightDirection, forcedDirectionalLightStrength);
			else if(model.terrain_object || model.always_use_sun)
				shaderManager->SetSun(model.sun_direction, model.sun_strength);
			else
			{
				std::string oname = object.GetName();
				if(oname.find("BuildingOuterWall") != oname.npos || oname.find("BuildingRoof") != oname.npos)
					shaderManager->SetSun(model.sun_direction, 1.f);
				else
					shaderManager->SetSun(VC3(), 0.f);
			}
		}
		else if(renderType == Depth)
		{
			if(material.getEffectTextureName().empty())
				applyTexture(material.GetBaseTexture(), 0);
			else
				storm.getProceduralManagerImp().apply(0);
		}
		else if(renderType == FakeDepth)
		{
			if(material.getEffectTextureName().empty())
				applyTexture(material.GetBaseTexture(), 0);
			else
				storm.getProceduralManagerImp().apply(0);
		}
		else if(renderType == SpotProjection)
		{
			if(material.getEffectTextureName().empty())
				applyTexture(material.GetBaseTexture(), 2);
			else
				storm.getProceduralManagerImp().apply(2);

			shaderManager->SetObjectDiffuse(material.GetColor());
		}
		else if(renderType == FakeProjection)
		{
		}
		else if(renderType == Glow)
		{
			if(material.getEffectTextureName().empty())
				applyTexture(material.GetBaseTexture(), 0);
			else
				storm.getProceduralManagerImp().apply(0);

			float glow = material.GetGlow() * material.GetGlowFactor();
			if(material.GetAlphaType() != IStorm3D_Material::ATYPE_NONE && material.GetAlphaType() != IStorm3D_Material::ATYPE_ADD)
				glow *= 1.f - material.GetTransparency();

			COL selfIllum(glow, glow, glow);

			shaderManager->SetObjectAmbient(selfIllum);
			shaderManager->SetObjectDiffuse(material.GetColor());
		}
		else if(renderType == Distortion)
		{
			if(material.getEffectTextureName().empty())
				applyTexture(material.GetDistortionTexture(), 0);
			else
				storm.getProceduralManagerImp().applyOffset(0);

			// FIXME: hack...
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		}

		shaderManager->SetTextureOffset(material.getScrollOffset1());

		// Special
		{ 
			bool doubleSided = false;
			bool wireFrame = false;
			material.GetSpecial(doubleSided, wireFrame);

			if(fillMode == Storm3D_TerrainModels::Wireframe)
				wireFrame = true;

			if(wireFrame)
				glPolygonMode(GL_FRONT, GL_LINE);
			else
				glPolygonMode(GL_FRONT, GL_FILL);
		}
	}

	void applySolidMaterial(RenderType renderType, Storm3D_Material &material, Storm3D_Model &model, Storm3D_Model_Object &object, Storm3D_Spotlight *spot)
	{
		applyGeneralMaterial(renderType, material, model, object);

		// Alpha
		{
			int alphaType = material.GetAlphaType();
			if(alphaType == IStorm3D_Material::ATYPE_NONE)
				glDisable(GL_ALPHA_TEST);
//#ifdef PROJECT_AOV
			// I don't want to break anything, so this is inside ifdef (although it should be ok for others as well)
			else if(enableAlphaTest && alphaType != IStorm3D_Material::ATYPE_NONE && renderType != FakeProjection)
//#else
			//else if(enableAlphaTest && alphaType == IStorm3D_Material::ATYPE_USE_ALPHATEST && renderType != FakeProjection)
//#endif
				glEnable(GL_ALPHA_TEST);
		}

		if(renderType == Depth)
			glEnable(GL_ALPHA_TEST);
	}

	void applyAlphaMaterial(RenderType renderType, Storm3D_Material &material, Storm3D_Model &model, Storm3D_Model_Object &object, Storm3D_Spotlight *spot)
	{
		Storm3D_ShaderManager *shaderManager = Storm3D_ShaderManager::GetSingleton();

		applyGeneralMaterial(renderType, material, model, object);
		int alphaType = material.GetAlphaType();

		if(renderType == SpotProjection)
			shaderManager->SetTransparencyFactor(object.spot_transparency_factor);

		if(renderType == Distortion)
			return;

		// Alpha
		{
			if(alphaType == IStorm3D_Material::ATYPE_ADD)
			{
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			}
			else if(alphaType == IStorm3D_Material::ATYPE_MUL)
			{
				if(spot)
					glBlendFunc(GL_ZERO, GL_ONE);
				else
					glBlendFunc(GL_ZERO, GL_SRC_COLOR);
			}
			else if(alphaType == IStorm3D_Material::ATYPE_USE_TRANSPARENCY || (renderType == SpotProjection && object.force_lighting_alpha_enable))
			{
				if(spot)
					glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				else
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			else if(alphaType == IStorm3D_Material::ATYPE_USE_TEXTRANSPARENCY || object.force_alpha > 0.0001f)
			{
				if(spot)
					glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				else
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
		}

		if(renderType == BaseLighting)
		{
			if(alphaType == IStorm3D_Material::ATYPE_ADD) {
				glDisable(GL_FOG);
				GLfloat c[4] = { 0, 0, 0, 0 };
				glFogfv(GL_FOG_COLOR, c);
			} else {
				glEnable(GL_FOG);
				COL cfog = Storm3D_ShaderManager::GetSingleton()->fogColor;
				GLfloat c[4] = { cfog.r, cfog.g, cfog.b, 1 };
				glFogfv(GL_FOG_COLOR, c);
			}
		}
	}

	bool renderSolid(RenderType renderType, Storm3D_Scene &scene, Storm3D_Spotlight *spot, Storm3D_FakeSpotlight *fakeSpot, const IStorm3D_Model *skipModel, RenderFlags flags)
	{
		Storm3D_ShaderManager::GetSingleton()->ClearCache();

//#ifdef PROJECT_AOV
		static int atp_alpharef = NORMAL_ALPHA_TEST_VALUE;
//#endif
		frozenbyte::storm::setCulling(CULL_CCW);
		glAlphaFunc(GL_GREATER, ((float) NORMAL_ALPHA_TEST_VALUE)/255.0f);

		if(renderType == BaseTextures || renderType == BaseLighting)
		{
			glDisable(GL_ALPHA_TEST);
			glDisable(GL_BLEND);
		}

		int objectType = NormalObjects;
		if(renderType == Depth || renderType == FakeDepth)
			objectType = DepthObjects;

		Frustum *frustum = 0;
		Frustum realFrustum;
		Frustum *frustum2 = 0;
		Frustum realFrustum2;
		if(spot)
		{
			Storm3D_Camera &camera = spot->getCamera();
			realFrustum = camera.getFrustum();
			frustum = &realFrustum;

			if(renderType == SpotProjection)
			{
				Storm3D_Camera &camera2 = static_cast<Storm3D_Camera &> (*scene.GetCamera());
				realFrustum2 = camera2.getFrustum();
				frustum2 = &realFrustum2;
			}
		}
		else if(fakeSpot)
		{
			Storm3D_Camera &camera = fakeSpot->getCamera();
			realFrustum = camera.getFrustum();
			frustum = &realFrustum;
		}

		bool renderedObjects = false;

		VC2 minPlane;
		VC2 maxPlane;

		if(fakeSpot)
			fakeSpot->getPlane(minPlane, maxPlane);

		ModelObjectList::iterator it = solidObjects[active_visibility][objectType].begin();
		for(; it != solidObjects[active_visibility][objectType].end(); ++it)
		{
			Storm3D_Model_Object *object = *it;
			if(!object)
				continue;

			if(!shouldRender(renderType, object, frustum, frustum2, skipModel, flags, minPlane, maxPlane, active_visibility))
				continue;

			Storm3D_Mesh *mesh = static_cast<Storm3D_Mesh *> (object->GetMesh());
			if(!mesh)
				continue;
			Storm3D_Material *material = static_cast<Storm3D_Material *> (mesh->GetMaterial());
			if(!material)
				continue;
			Storm3D_Model *model = object->parent_model;
			if(!model)
				continue;

//#ifdef PROJECT_AOV
			// for performance considerations, this is only enabled for AOV.
			// (should other projects see use for this, remove the ifdefs)
			if (object->renderPassMask & (1<<RENDER_PASS_BIT_ADDITIONAL_ALPHA_TEST_PASS)
				&& (additionalAlphaTestPassAllowed || !object->alphaTestPassConditional))
			{
				if (atp_alpharef != object->alphaTestValue)
				{
					atp_alpharef = object->alphaTestValue;
					glAlphaFunc(GL_GREATER, float(object->alphaTestValue) / 255.0f);
				}
			} else {
				if (atp_alpharef != NORMAL_ALPHA_TEST_VALUE)
				{
					atp_alpharef = NORMAL_ALPHA_TEST_VALUE;
					glAlphaFunc(GL_GREATER, float(NORMAL_ALPHA_TEST_VALUE) / 255.0f);
				}
			}
//#endif

			applySolidMaterial(renderType, *material, *model, *object, spot);
			renderObject(&scene, object);

			renderedObjects = true;
		}

		if(fillMode == Storm3D_TerrainModels::Wireframe)
			glPolygonMode(GL_FRONT, GL_LINE);
		else
			glPolygonMode(GL_FRONT, GL_FILL);

		return renderedObjects;
	}

	void renderAlpha(RenderType renderType, Storm3D_Scene &scene, Storm3D_Spotlight *spot, Storm3D_FakeSpotlight *fakeSpot, const IStorm3D_Model *skipModel, RenderFlags flags)
	{
		Storm3D_ShaderManager::GetSingleton()->ClearCache();

		glEnable(GL_ALPHA_TEST);
		glEnable(GL_BLEND);
		glAlphaFunc(GL_GREATER, 1.0f/255.0f);
		glDepthMask(GL_FALSE);

		int objectType = NormalObjects;

		Frustum *frustum = 0;
		Frustum realFrustum;
		Frustum *frustum2 = 0;
		Frustum realFrustum2;
		if(spot)
		{
			Storm3D_Camera &camera = spot->getCamera();
			realFrustum = camera.getFrustum();
			frustum = &realFrustum;

			if(renderType == SpotProjection)
			{
				Storm3D_Camera &camera2 = static_cast<Storm3D_Camera &> (*scene.GetCamera());
				realFrustum2 = camera2.getFrustum();
				frustum2 = &realFrustum2;
			}
		}
		else if(fakeSpot)
		{
			Storm3D_Camera &camera = fakeSpot->getCamera();
			realFrustum = camera.getFrustum();
			frustum = &realFrustum;
		}
		else
		{
			Storm3D_Camera &camera = static_cast<Storm3D_Camera &> (*scene.GetCamera());
			realFrustum = camera.getFrustum();
			frustum = &realFrustum;
		}

		VC2 minPlane;
		VC2 maxPlane;
		if(fakeSpot)
			fakeSpot->getPlane(minPlane, maxPlane);

		std::vector<Storm3D_Model_Object *> special_render_pass_objects[RENDER_PASS_BITS_AMOUNT];

		ModelObjectList::iterator it = alphaObjects[active_visibility][objectType].begin();
		for(; it != alphaObjects[active_visibility][objectType].end(); ++it)
		{
			Storm3D_Model_Object *object = *it;
			if(!object)
				continue;

			if(!shouldRender(renderType, object, frustum, frustum2, skipModel, flags, minPlane, maxPlane, active_visibility))
				continue;

			Storm3D_Mesh *mesh = static_cast<Storm3D_Mesh *> (object->GetMesh());
			if(!mesh)
				continue;
			Storm3D_Material *material = static_cast<Storm3D_Material *> (mesh->GetMaterial());
			if(!material)
				continue;
			Storm3D_Model *model = object->parent_model;
			if(!model)
				continue;

			if (object->renderPassMask)
			{
				bool renderAsNormal = false;
				for (int i = 0; i < RENDER_PASS_BITS_AMOUNT; i++)
				{
					if (object->renderPassMask & (1<<i))
					{
						if (i == RENDER_PASS_BIT_EARLY_ALPHA
							|| i == RENDER_PASS_BIT_ADDITIONAL_ALPHA_TEST_PASS)
						{
							renderAsNormal = true;
						} else {
							special_render_pass_objects[i].push_back(object);
						}
					}
				}
				if (renderAsNormal)
				{
					applyAlphaMaterial(renderType, *material, *model, *object, spot);
					renderObject(&scene, object);
				}
			} else {
				applyAlphaMaterial(renderType, *material, *model, *object, spot);
				renderObject(&scene, object);
			}
		}

		bool specialPassesDone = false;

		for (int i = 0; i < RENDER_PASS_BITS_AMOUNT; i++)
		{
			if (i == RENDER_PASS_BIT_CAREFLECTION_DEPTH_MASKS)
			{
				if (special_render_pass_objects[RENDER_PASS_BIT_CAREFLECTION_DEPTH_MASKS].size() > 0)
				{
					specialPassesDone = true;

					Storm3D_ShaderManager::GetSingleton()->ClearCache();

					glDisable(GL_ALPHA_TEST);
					glDisable(GL_BLEND);
					glAlphaFunc(GL_GREATER, float(NORMAL_ALPHA_TEST_VALUE)/ 255.0f);
					glDepthMask(GL_TRUE);
					glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
				}
			}
			else if (i == RENDER_PASS_BIT_CAREFLECTION_REFLECTED)
			{
				if (special_render_pass_objects[RENDER_PASS_BIT_CAREFLECTION_REFLECTED].size() > 0)
				{
					specialPassesDone = true;

					Storm3D_ShaderManager::GetSingleton()->ClearCache();

					glDisable(GL_ALPHA_TEST);
					glDisable(GL_BLEND);
					glAlphaFunc(GL_GREATER, float(NORMAL_ALPHA_TEST_VALUE)/ 255.0f);
					glDepthMask(GL_TRUE);
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

					// NOTE: this is necessary at least when reflected along single axis (a single axis scale is negative)
					// as that changes CCW <-> CW
					frozenbyte::storm::setCulling(CULL_CW);
				}
			}
			else if (i == RENDER_PASS_BIT_CAMOTIONBLUR1)
				// no need to do this again, as it is right after the above one...
				//|| i == RENDER_PASS_BIT_CAMOTIONBLUR2
				//|| i == RENDER_PASS_BIT_CAMOTIONBLUR3)
			{
				if (special_render_pass_objects[RENDER_PASS_BIT_CAMOTIONBLUR1].size() > 0
					|| special_render_pass_objects[RENDER_PASS_BIT_CAMOTIONBLUR2].size() > 0
					|| special_render_pass_objects[RENDER_PASS_BIT_CAMOTIONBLUR3].size() > 0)
				{
					specialPassesDone = true;

					Storm3D_ShaderManager::GetSingleton()->ClearCache();

					glEnable(GL_ALPHA_TEST);
					glEnable(GL_BLEND);
					glAlphaFunc(GL_GREATER, float(0x01)/ 255.0f);
					glDepthMask(GL_FALSE);
				}
			}
			else if (i == RENDER_PASS_BIT_DELAYED_ALPHA)
			{
				if (special_render_pass_objects[RENDER_PASS_BIT_DELAYED_ALPHA].size() > 0
					&& specialPassesDone)
				{
					Storm3D_ShaderManager::GetSingleton()->ClearCache();

					glEnable(GL_ALPHA_TEST);
					glEnable(GL_BLEND);
					glAlphaFunc(GL_GREATER, float(0x01)/ 255.0f);
					glDepthMask(GL_FALSE);
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

					// restoring this in case of the reflection / depth masks have been screwing it up.
					// or should it be NONE for alpha objects?
					frozenbyte::storm::setCulling(CULL_CCW);
				}
			}
			else if (i == RENDER_PASS_BIT_EARLY_ALPHA
				|| i == RENDER_PASS_BIT_ADDITIONAL_ALPHA_TEST_PASS)
			{
				// early alpha rendered with other normal alpha objects.
				// same goes for additional alpha test pass objects.
				/*
				if (special_render_pass_objects[RENDER_PASS_BIT_EARLY_ALPHA].size() > 0
					&& specialPassesDone)
				{
					Storm3D_ShaderManager::GetSingleton()->ClearCache();

					device.SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
					device.SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
					device.SetRenderState(D3DRS_ALPHAREF, 0x01);
					device.SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
					device.SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

					device.SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);

					// restoring this in case of the reflection / depth masks have been screwing it up.
					// or should it be NONE for alpha objects?
					frozenbyte::storm::setCulling(device, D3DCULL_CCW);
				}
				*/
			}

			for(int ind = 0; ind < (int)special_render_pass_objects[i].size(); ind++)
			{
				Storm3D_Model_Object *object = special_render_pass_objects[i][ind];
				assert(object);
				Storm3D_Mesh *mesh = static_cast<Storm3D_Mesh *> (object->GetMesh());
				assert(mesh);
				Storm3D_Material *material = static_cast<Storm3D_Material *> (mesh->GetMaterial());
				assert(material);
				Storm3D_Model *model = object->parent_model;
				assert(model);

				//VC3 origPos = object->position;
				VC3 origPos = object->parent_model->GetPosition();
				VC3 origScale = object->parent_model->GetScale();
				assert(object->renderPassMask & (1 << i));
				//if (object->renderPassMask & (1 << i))
				{
					if (i == RENDER_PASS_BIT_CAREFLECTION_REFLECTED
						|| i == RENDER_PASS_BIT_CAMOTIONBLUR1
						|| i == RENDER_PASS_BIT_CAMOTIONBLUR2
						|| i == RENDER_PASS_BIT_CAMOTIONBLUR3)
					{
						model->SetPosition(origPos + object->renderPassOffset[i]);
						model->SetScale(object->renderPassScale[i]);
						model->GetMX();
					}
					//object->SetPosition(origPos + object->renderPassOffset[i]);
					//if (i == RENDER_PASS_BIT_CAREFLECTION_DEPTH_MASKS
					//	|| i == RENDER_PASS_BIT_CAREFLECTION_REFLECTED)
					//{
					//	applySolidMaterial(renderType, *material, *model, *object, spot);
					//} else {
						applyAlphaMaterial(renderType, *material, *model, *object, spot);
					//}
					renderObject(&scene, object);
				}
				//object->position = origPos;
				if (i == RENDER_PASS_BIT_CAREFLECTION_REFLECTED
					|| i == RENDER_PASS_BIT_CAMOTIONBLUR1
					|| i == RENDER_PASS_BIT_CAMOTIONBLUR2
					|| i == RENDER_PASS_BIT_CAMOTIONBLUR3)
				{
					object->parent_model->SetPosition(origPos);
					object->parent_model->SetScale(origScale);
				}
			}
		}

		if (specialPassesDone)
		{
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

			// TODO: restore the full original state? if that is necessary...
		}

		Storm3D_ShaderManager::GetSingleton()->SetTransparencyFactor(1.f);


	}

	void renderObject(Storm3D_Scene *scene, Storm3D_Model_Object *object)
	{
		Storm3D_Mesh *mesh = static_cast<Storm3D_Mesh *> (object->GetMesh());
		mesh->ReBuild();

		Storm3D_ShaderManager::GetSingleton()->SetShader(object);
		mesh->RenderBuffers(object);

		if(scene)
			scene->AddPolyCounter(mesh->GetRenderFaceCount());
	}
};

//! Constructor
Storm3D_TerrainModels::Storm3D_TerrainModels(Storm3D &storm)
{
	boost::scoped_ptr<Storm3D_TerrainModelsData> tempData(new Storm3D_TerrainModelsData(storm));
	data.swap(tempData);
}

//! Destructor
Storm3D_TerrainModels::~Storm3D_TerrainModels()
{
}

//! Add terrain model
/*!
	\param model_ model to add
*/
void Storm3D_TerrainModels::addModel(IStorm3D_Model &model_)
{
	Storm3D_Model *model = static_cast<Storm3D_Model *> (&model_); 

	if(data->models.find(model) == data->models.end())
	{
		data->models.insert(model);
		data->insert(model);
	}
}

//! Remove terrain model
/*!
	\param model_ model to remove
*/
void Storm3D_TerrainModels::removeModel(IStorm3D_Model &model_)
{
	Storm3D_Model *model = static_cast<Storm3D_Model *> (&model_); 
	if(model->observer)
		model->observer->remove(model);

	for(unsigned int i = 0; i < data->disabledCullingModels.size(); ++i)
	{
		if(model == data->disabledCullingModels[i])
		{
			data->disabledCullingModels.erase(data->disabledCullingModels.begin() + i);
			break;
		}
	}

	model->observer = 0;
	data->models.erase(model);
	data->erase(model);
	data->observers.erase(model);
}

void Storm3D_TerrainModels::enableCulling(Storm3D_Model &model, bool enable)
{
	if(enable)
	{
		for(unsigned int i = 0; i < data->disabledCullingModels.size(); ++i)
		{
			if(&model == data->disabledCullingModels[i])
			{
				data->disabledCullingModels.erase(data->disabledCullingModels.begin() + i);
				return;
			}
		}
	}
	else
	{
		for(unsigned int i = 0; i < data->disabledCullingModels.size(); ++i)
		{
			if(&model == data->disabledCullingModels[i])
				return;
		}

		data->disabledCullingModels.push_back(&model);
	}
}

//! Pop terrain model from stack
/*!
	\return model
*/
IStorm3D_Model *Storm3D_TerrainModels::popModel()
{
	if(data->models.empty())
		return 0;

	std::set<Storm3D_Model *>::iterator it = data->models.begin();
	Storm3D_Model *model = *it;

	data->models.erase(it);
	data->observers.erase(model);
	model->observer = 0;

	return model;
}

//! Build model tree of given size
/*!
	\param size tree size
*/
void Storm3D_TerrainModels::buildTree(const VC3 &size)
{
	data->buildTree(size);
}

//! Does the model tree exist?
/*!
	\return true if tree exists
*/
bool Storm3D_TerrainModels::hasTree() const
{
	return data->tree;
}

//! Raytrace
/*!
	\param position ray start position
	\param direction ray direction
	\param rayLength ray length
	\param info reference to collision info structure
	\param accurate true to use accurate collision detection
*/
void Storm3D_TerrainModels::RayTrace(const VC3 &position, const VC3 &direction, float rayLength, Storm3D_CollisionInfo &info, bool accurate) const
{
	assert(data->tree);

	Ray ray(position, direction, rayLength);
	data->tree->RayTrace(ray, info, accurate);
}

//! Check sphere collisions
/*!
	\param position collision sphere position
	\param radius collision sphere radius
	\param info reference to collision info structure
	\param accurate true to use accurate collision detection
*/
void Storm3D_TerrainModels::SphereCollision(const VC3 &position, float radius, Storm3D_CollisionInfo &info, bool accurate) const
{
	assert(data->tree);

	Sphere sphere(position, radius);
	data->tree->SphereCollision(sphere, info, accurate);
}

//! Calculate visibility
/*!
	\param scene scene
	\param timeDelta time difference
*/
void Storm3D_TerrainModels::calculateVisibility(Storm3D_Scene &scene, int timeDelta)
{
	Storm3D_Camera camera = static_cast<Storm3D_Camera &> (*scene.GetCamera());
	Storm3D_Camera haxCamera = camera;

	// Temp hax ..
	float fov = haxCamera.GetFieldOfView() * 1.2f;
	haxCamera.SetFieldOfView(fov);

	data->findVisibleModels(camera, haxCamera, timeDelta);
}

//! Render textures
/*!
	\param materialType material type
	\param scene scene
*/
void Storm3D_TerrainModels::renderTextures(MaterialType materialType, Storm3D_Scene &scene)
{
	glActiveTexture(GL_TEXTURE0);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
	for (unsigned int i = 1; i < 8; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}


	if(materialType == SolidOnly)
		data->renderSolid(BaseTextures, scene, 0, 0, 0, None);
	else if(materialType == AlphaOnly)
	{
		data->renderAlpha(BaseTextures, scene, 0, 0, 0, None);

		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
	}
}

//! Render lighting
/*!
	\param materialType material type
	\param scene scene
*/
void Storm3D_TerrainModels::renderLighting(MaterialType materialType, Storm3D_Scene &scene)
{
	if(materialType == SolidOnly)
		data->renderSolid(BaseLighting, scene, 0, 0, 0, None);
	else if(materialType == AlphaOnly)
	{
		data->renderAlpha(BaseLighting, scene, 0, 0, 0, None);

		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
	}
}

//! Render depth
/*!
	\param scene scene
	\param camera camera
	\param spot spotlight
	\param skipModel
*/
void Storm3D_TerrainModels::renderDepth(Storm3D_Scene &scene, Storm3D_Camera &camera, Storm3D_Spotlight &spot, const IStorm3D_Model *skipModel)
{
	data->renderSolid(Depth, scene, &spot, 0, skipModel, None);
}

//! Render depth
/*!
	\param scene scene
	\param camera camera
	\param spot fake spotlight
	\return
*/
bool Storm3D_TerrainModels::renderDepth(Storm3D_Scene &scene, Storm3D_Camera &camera, Storm3D_FakeSpotlight &spot)
{
	RenderFlags flag = None;
	if(!spot.shouldRenderObjectShadows())
		flag = SkipTerrainLightmappedObjects;

	return data->renderSolid(FakeDepth, scene, 0, &spot, 0, flag);
}

//! Render projection
/*!
	\param materialType material type
	\param scene scene
	\param spot spotlight
*/
void Storm3D_TerrainModels::renderProjection(MaterialType materialType, Storm3D_Scene &scene, Storm3D_Spotlight &spot)
{
	if(materialType == SolidOnly)
		data->renderSolid(SpotProjection, scene, &spot, 0, 0, None);
	else if(materialType == AlphaOnly)
	{
		data->renderAlpha(SpotProjection, scene, &spot, 0, 0, None);

		glBlendFunc(GL_ONE, GL_ONE);
	}
}

//! Render projection
/*!
	\param scene scene
	\param spot fake spotlight
*/
void Storm3D_TerrainModels::renderProjection(Storm3D_Scene &scene, Storm3D_FakeSpotlight &spot)
{
	data->renderSolid(FakeProjection, scene, 0, &spot, 0, None);

	glBlendFunc(GL_ZERO, GL_SRC_COLOR);
}

//! Render glow textures
/*!
	\param scene scene
*/
void Storm3D_TerrainModels::renderGlows(Storm3D_Scene &scene)
{
	frozenbyte::storm::PixelShader::disable();
	glActiveTexture(GL_TEXTURE0);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
	for (unsigned int i = 1; i < 8; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	data->renderSolid(Glow, scene, 0, 0, 0, None);
	data->renderAlpha(Glow, scene, 0, 0, 0, None);

	glBlendFunc(GL_ONE, GL_ONE);
}

//! Render distortion textures
/*!
	\param scene scene
*/
void Storm3D_TerrainModels::renderDistortion(Storm3D_Scene &scene)
{
	frozenbyte::storm::PixelShader::disable();
	glActiveTexture(GL_TEXTURE0);
	glClientActiveTexture(GL_TEXTURE0);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
	for (unsigned int i = 1; i < 8; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	data->renderAlpha(Distortion, scene, 0, 0, 0, None);
}

//! Set fill mode
/*!
	\param mode fill mode
*/
void Storm3D_TerrainModels::setFillMode(FillMode mode)
{
	data->fillMode = mode;
}

//! Set lightmap filtering
/*!
	\param filter true to filter lightmaps
*/
void Storm3D_TerrainModels::filterLightmap(bool filter)
{
	data->filterLightmap = filter;
}

//! Set collision rendering
/*!
	\param enable true to enable collision rendering
*/
void Storm3D_TerrainModels::setCollisionRendering(bool enable)
{
	data->renderCollision = enable;
}

//! Enable or disable alpha testing
/*!
	\param enable true to enable alpha testing
*/
void Storm3D_TerrainModels::enableAlphaTest(bool enable)
{
	data->enableAlphaTest = enable;
}

//! Enable or disable boned rendering
/*!
	\param enable true to enable boned rendering
*/
void Storm3D_TerrainModels::renderBoned(bool enable)
{
	data->renderBoned = enable;
}

//! Enable or disable ambient material
/*!
	\param enable true to enable ambient material
*/
void Storm3D_TerrainModels::enableMaterialAmbient(bool enable)
{
	data->enableMaterialAmbient = enable;
}

//! Force a white base texture
/*!
	\param force true to force white base texture
*/
void Storm3D_TerrainModels::forceWhiteBaseTexture(bool force)
{
	data->forceWhiteBase = force;
}

//! Enable or disable glow
/*!
	\param enable true to enable glow
*/
void Storm3D_TerrainModels::enableGlow(bool enable)
{
	data->enableGlow = enable;
}

void Storm3D_TerrainModels::enableAdditionalAlphaTestPass(bool enable)
{
	data->additionalAlphaTestPassAllowed = enable;
}

void Storm3D_TerrainModels::enableSkyModelGlow(bool enable)
{
	data->skyModelGlowAllowed = enable;
}

//! Enable or disable distortion
/*!
	\param enable true to enable distortion
*/
void Storm3D_TerrainModels::enableDistortion(bool enable)
{
	data->enableDistortion = enable;
}

//! Enable or disable reflection
/*!
	\param enable true to enable reflection
*/
void Storm3D_TerrainModels::enableReflection(bool enable)
{
	data->enableReflection = enable;
}

//! Render background
/*!
	\param model model object
*/
void Storm3D_TerrainModels::renderBackground(Storm3D_Model *model)
{
	if(!model)
		return;

	Storm3D_ShaderManager *shaderManager = Storm3D_ShaderManager::GetSingleton();
	shaderManager->SetTextureOffset(VC2());

	set<IStorm3D_Model_Object *>::iterator it = model->objects.begin();
	for(; it != model->objects.end(); ++it)
	{
		Storm3D_Model_Object *object = static_cast<Storm3D_Model_Object *> (*it);
		Storm3D_Mesh *mesh = static_cast<Storm3D_Mesh *> (object->GetMesh());
		if(!mesh)
			continue;
		Storm3D_Material *material = static_cast<Storm3D_Material *> (mesh->GetMaterial());
		if(!material)
			continue;

		data->applyTexture(material->GetBaseTexture(), 0);
		shaderManager->SetObjectDiffuse(material->GetColor());

		mesh->ReBuild();
	
		shaderManager->SetShader(object);
		shaderManager->BackgroundShader();
		mesh->RenderBuffers(object);

	}
}

//! Add light
/*!
	\param position light position
	\param radius light radius
	\param color light color
	\return number of lights
*/
int Storm3D_TerrainModels::addLight(const VC3 &position, float radius, const COL &color)
{
	TerrainLight light;
	light.position = position;
	light.radius = radius;
	light.color = color;

	data->lights.push_back(light);
	return data->lights.size() - 1;
}

//! Set position of light
/*!
	\param index index of light to manipulate
	\param position position
*/
void Storm3D_TerrainModels::setLightPosition(int index, const VC3 &position)
{
	if(index >= 0 && index < int(data->lights.size()))
		data->lights[index].position = position;
}

//! Set radius of light
/*!
	\param index index of light to manipulate
	\param radius radius
*/
void Storm3D_TerrainModels::setLightRadius(int index, float radius)
{
	if(index >= 0 && index < int(data->lights.size()))
		data->lights[index].radius = radius;
}

//! Set color of light
/*!
	\param index index of light to manipulate
	\param color color
*/
void Storm3D_TerrainModels::setLightColor(int index, const COL &color)
{
	if(index >= 0 && index < int(data->lights.size()))
		data->lights[index].color = color;
}

//! Get light at given index
/*!
	\param index index of light to retrieve
	\return light
*/
const TerrainLight &Storm3D_TerrainModels::getLight(int index) const
{
	if(index < 0 || index >= int(data->lights.size()))
	{
		assert(!"Light index out of bounds.");

		static TerrainLight empty;
		return empty;
	}

	return data->lights[index];
}

//! Remove light from given index
/*!
	\param index index of light to remove
*/
void Storm3D_TerrainModels::removeLight(int index)
{
	if(index >= 0 && index < int(data->lights.size()))
		data->lights.erase(data->lights.begin() + index);
}

//! Clear all lights
void Storm3D_TerrainModels::clearLights()
{
	data->lights.clear();
}

//! Set terrain object color factor in buildings
/*!
	\param factor color factor
*/
void Storm3D_TerrainModels::setTerrainObjectColorFactorBuilding(const COL &factor)
{
	data->terrainObjectColorFactorBuilding = factor;
}

//! Set terrain object color factor outside
/*!
	\param factor color factor
*/
void Storm3D_TerrainModels::setTerrainObjectColorFactorOutside(const COL &factor)
{
	data->terrainObjectColorFactorOutside = factor;
}

void Storm3D_TerrainModels::setForcedDirectional(bool enabled, const VC3 &direction, const COL &color)
{
	/*
	data->forcedDirectionalLightEnabled = true; //enabled;
	data->forcedDirectionalLightDirection = VC3(1, 1, 1); //direction,
	data->forcedDirectionalLightStrength = 1.f; //(color.r + color.g + color.b) / 3.f;
	*/

	data->forcedDirectionalLightEnabled = enabled;
	data->forcedDirectionalLightDirection = direction,
	data->forcedDirectionalLightStrength = (color.r + color.g + color.b) / 3.f / 2.f;

	data->forcedDirectionalLightDirection *= data->forcedDirectionalLightStrength;
}
