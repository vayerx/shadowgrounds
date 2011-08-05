// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

#include <map>
#include <vector>
#include <string>
#include <queue>
#include <atlbase.h>
#include <d3d9.h>

#include "storm3d_terrain_groups.h"
#include "storm3d_terrain_utils.h"
#include "storm3d_terrain_models.h"
#include "storm3d_material.h"
#include "storm3d_model.h"
#include "storm3d_model_object.h"
#include "storm3d_mesh.h"
#include "storm3d_texture.h"
#include "storm3d_material.h"
#include "storm3d_scene.h"
#include "storm3d_spotlight.h"
#include "VertexFormats.h"
#include "Storm3D_ShaderManager.h"
#include "Storm3D_Bone.h"
#include <c2_qtree.h>

#include "../../util/Debug_MemoryManager.h"

using namespace std;
using namespace boost;


	struct SharedModel;
	struct Instance;
	typedef vector<SharedModel> ModelList;
	typedef vector<shared_ptr<Instance> > InstanceList;

	struct AnimationDeleter
	{
		void operator ()(IStorm3D_BoneAnimation *a)
		{
			if(a)
				a->Release();
		}
	};

	struct SharedModel
	{
		shared_ptr<IStorm3D_Model> model;
		shared_ptr<IStorm3D_Model> fadeModel;
		InstanceList instances;

		std::string bones;
		std::string idleAnimation;
		boost::shared_ptr<IStorm3D_BoneAnimation> animation;

		float radius;
		float radius2d;

		SharedModel(shared_ptr<IStorm3D_Model> model_, shared_ptr<IStorm3D_Model> fadeModel_, const std::string &bones_, const std::string &idleAnimation_)
		:	model(model_),
			fadeModel(fadeModel_),
			bones(bones_),
			idleAnimation(idleAnimation_),
			radius(0),
			radius2d(0)
		{
			boost::scoped_ptr<Iterator<IStorm3D_Model_Object *> > objectIterator(model->ITObject->Begin());
			for(; !objectIterator->IsEnd(); objectIterator->Next())
			{
				IStorm3D_Model_Object *object = objectIterator->GetCurrent();
				if(!object)
					continue;

				IStorm3D_Mesh *mesh = object->GetMesh();
				if(!mesh)
					continue;

				VC3 objectPosition = object->GetPosition();
				float objectDistance = objectPosition.GetLength();
				float distance = objectDistance + mesh->GetRadius();
				if(distance > radius)
					radius = distance;


				float meshRadius = mesh->GetRadius();
				/*
				float meshRadius = 0;
				const Storm3D_Vertex *buffer = mesh->GetVertexBufferReadOnly();

				for(int i = 0; i < mesh->GetVertexCount(); ++i)
				{
					VC2 vertex(buffer[i].position.x, buffer[i].position.z);
					float vertexLength = vertex.GetLength();

					if(vertexLength > meshRadius)
						meshRadius = vertexLength;
				}
				*/

				if(meshRadius + objectDistance > radius2d)
					radius2d = meshRadius + objectDistance;
			}

			if(!bones.empty())
				model->LoadBones(bones.c_str());
		}
	};

	struct Instance
	{
		shared_ptr<IStorm3D_Model> model;
		shared_ptr<IStorm3D_Model> fadeModel;

		int modelId;
		int instanceId;

		VC3 position;
		QUAT rotation;

		COL ambient;

		/*
		VC3 lightPosition1;
		COL lightColor1;
		float lightRange1;
		VC3 lightPosition2;
		COL lightColor2;
		float lightRange2;
		*/
		signed short lightIndex[LIGHT_MAX_AMOUNT];

		Quadtree<Instance>::Entity *entity;
		float radius2d;

		bool lightmapped;
		bool inBuilding;

		Instance(shared_ptr<IStorm3D_Model> model_, shared_ptr<IStorm3D_Model> fadeModel_, const COL &ambient_)
		:	model(model_),
			fadeModel(fadeModel_),
			modelId(-1),
			instanceId(-1),
			ambient(ambient_),
			entity(0),
			//lightRange1(5.f),
			//lightRange2(5.f),
			radius2d(0),
			lightmapped(false),
			inBuilding(false)
		{
		}

		void SphereCollision(const VC3 &position,float radius, Storm3D_CollisionInfo &info, bool accurate) const
		{
			Storm3D_Model *m = static_cast<Storm3D_Model *> (model.get());
			bool noCollision = m->GetNoCollision();

			m->SetNoCollision(false);
			model->SphereCollision(position, radius, info, true);
			m->SetNoCollision(noCollision);
		}

		bool fits(const AABB &area) const
		{
			if(position.x - radius2d < area.mmin.x)
				return false;
			if(position.x + radius2d > area.mmax.x)
				return false;
			if(position.z - radius2d < area.mmin.z)
				return false;
			if(position.z + radius2d > area.mmax.z)
				return false;

			return true;
		}

		void erase(Quadtree<Instance> *tree, Storm3D_TerrainModels &terrainModels)
		{
			if(tree && entity)
				tree->erase(entity);

			if(model)
				terrainModels.removeModel(*model);
			if(fadeModel)
				terrainModels.removeModel(*fadeModel);

			model.reset();
			entity = 0;
		}
	};

	struct InstanceInfo
	{
		int modelId;
		int instanceId;

		InstanceInfo(int mid, int iid)
		:	modelId(mid),
			instanceId(iid)
		{
		}

		InstanceInfo()
		:	modelId(-1),
			instanceId(-1)
		{
		}
	};

	typedef Quadtree<Instance> Tree;
	typedef map<IStorm3D_Model *, InstanceInfo> InstanceMap;


struct Storm3D_TerrainGroupData
{
	Storm3D &storm;
	IDirect3DDevice9 &device;
	Storm3D_TerrainModels &terrainModels;

	VC2 sceneSize;

	ModelList models;
	boost::scoped_ptr<Tree> tree;

	InstanceMap instanceMap;

	Storm3D_TerrainGroupData(Storm3D &storm_, Storm3D_TerrainModels &terrainModels_, bool ps14)
	:	storm(storm_),
		device(*storm.GetD3DDevice()),
		terrainModels(terrainModels_)
	{
	}
};

namespace {

	class TerrainIterator: public IStorm3D_TerrainModelIterator
	{
		std::vector<Instance *> instances;
		int current;

		Tree *tree;
		Storm3D_TerrainModels &terrainModels;

	public:
		TerrainIterator(Tree *tree_, const VC3 &position, float radius, Storm3D_TerrainModels &terrainModels_)
		:	current(0),
			tree(tree_),
			terrainModels(terrainModels_)
		{
			tree->collectSphere(instances, position, radius);
		}

		~TerrainIterator() 
		{
		}

		void next()
		{
			++current;
		}

		void erase()
		{
			instances[current]->erase(tree, terrainModels);
			next();
		}

		bool end() const
		{
			return current >= int(instances.size());
		}

		VC3 getPosition() const
		{
			assert(!end());
			return instances[current]->position;
		}

		QUAT getRotation() const
		{
			assert(!end());
			return instances[current]->rotation;
		}

		COL getColor() const
		{
			assert(!end());
			return instances[current]->ambient;
		}

		int getModelId() const
		{
			assert(!end());
			return instances[current]->modelId;
		}

		int getInstanceId() const
		{
			assert(!end());
			return instances[current]->instanceId;
		}
	};

} // unnamed

Storm3D_TerrainGroup::Storm3D_TerrainGroup(Storm3D &storm, Storm3D_TerrainModels &models, bool ps14)
{
	boost::scoped_ptr<Storm3D_TerrainGroupData> tempData(new Storm3D_TerrainGroupData(storm, models, ps14));
	data.swap(tempData);
}

Storm3D_TerrainGroup::~Storm3D_TerrainGroup()
{
}

int Storm3D_TerrainGroup::addModel(boost::shared_ptr<Storm3D_Model> model, boost::shared_ptr<Storm3D_Model> fadeModel, const std::string &bones, const std::string &idleAnimation)
{
	assert(model);

	int index = data->models.size();
	data->models.push_back(SharedModel(model, fadeModel, bones, idleAnimation));

	return index;
}

void Storm3D_TerrainGroup::removeModels()
{
	removeInstances();
}

int Storm3D_TerrainGroup::addInstance(int modelId, const VC3 &position, const QUAT &rotation, const COL &color)
{
	assert(modelId >= 0 && modelId < int(data->models.size()));
	SharedModel &original = data->models[modelId];

	shared_ptr<IStorm3D_Model> m(original.model->GetClone(true, false, true));
	//m->SetNoCollision(original.model->GetNoCollision());

	shared_ptr<IStorm3D_Model> mf;
	if(original.fadeModel)
		mf.reset(original.fadeModel->GetClone(true, false, true));

	if(!original.idleAnimation.empty() && !original.animation)
	{
		IStorm3D_BoneAnimation *ba = data->storm.CreateNewBoneAnimation(original.idleAnimation.c_str());
		if(ba)
			original.animation.reset(ba, AnimationDeleter());
	}

	m->SetPosition(const_cast<VC3 &> (position));
	m->SetRotation(const_cast<QUAT &> (rotation));
	m->SetSelfIllumination(color);
#ifdef PHYSICS_NONE
	// PSDHAX!!!
	m->SetNoCollision(true);
#endif

	static_cast<Storm3D_Model *> (m.get())->terrain_object = true;

	if(mf)
	{
		mf->SetPosition(const_cast<VC3 &> (position));
		mf->SetRotation(const_cast<QUAT &> (rotation));
		mf->SetSelfIllumination(color);
		mf->SetNoCollision(true);
		static_cast<Storm3D_Model *> (mf.get())->terrain_object = true;

		boost::scoped_ptr<Iterator<IStorm3D_Model_Object *> > objectIterator(mf->ITObject->Begin());
		for(; !objectIterator->IsEnd(); objectIterator->Next())
		{
			IStorm3D_Model_Object *object = objectIterator->GetCurrent();
			if(!object)
				continue;

			object->SetForceAlpha(0);
		}

	}

	if(original.animation)
	{
		m->SetRandomAnimation(original.animation.get());
		if(mf)
			mf->SetRandomAnimation(original.animation.get());
	}

	data->terrainModels.addModel(*m);

	int index = original.instances.size();
	shared_ptr<Instance> instance(new Instance(m, mf, color));
	original.instances.push_back(instance);

	// ToDo:
	// Add a flag which controls whether to insert instance to collision tree.
	// A lot of objects don't care about collisions, anyway

	instance->radius2d = original.radius2d;
	instance->position = position;
	instance->rotation = rotation;
	instance->modelId = modelId;
	instance->instanceId = index;
	instance->entity = data->tree->insert(instance.get(), position, original.radius);

	// give the storm3d model some info about the terrain object instance/model ids
	static_cast<Storm3D_Model *> (m.get())->terrainInstanceId = index;
	static_cast<Storm3D_Model *> (m.get())->terrainModelId = modelId;
	// --jpk

	data->instanceMap[m.get()] = InstanceInfo(modelId, index);
	return index;
}

void Storm3D_TerrainGroup::setInstancePosition(int modelId, int instanceId, const VC3 &position)
{
	assert(modelId >= 0 && instanceId >= 0);
	if(modelId < 0 || instanceId < 0 || modelId >= int(data->models.size()))
	{
		assert(!"Storm3D_TerrainGroup::setInstancePosition - model id is out of range or instance id is negative.");
		return;
	}

	SharedModel &original = data->models[modelId];
	if(instanceId >= int(original.instances.size()))
	{
		assert(!"Storm3D_TerrainGroup::setInstancePosition - instance id is out of range.");
		return;
	}

	Instance &instance = *original.instances[instanceId];

	// had a mysterious crash somewhere around here, thus, added these asserts.
	assert(instance.instanceId == instanceId);
	assert(instance.modelId == modelId);
	assert(instance.entity != NULL);

	instance.position = position;

	instance.entity->setPosition(position);

	if(instance.model)
		instance.model->SetPosition(const_cast<VC3 &> (position));
	if(instance.fadeModel)
		instance.fadeModel->SetPosition(const_cast<VC3 &> (position));
}

void Storm3D_TerrainGroup::setInstanceRotation(int modelId, int instanceId, const QUAT &rotation)
{
	assert(modelId >= 0 && instanceId >= 0);
	if(modelId < 0 || instanceId < 0 || modelId >= int(data->models.size()))
		return;

	SharedModel &original = data->models[modelId];
	if(instanceId >= int(original.instances.size()))
		return;

	Instance &instance = *original.instances[instanceId];

	if(instance.model)
		instance.model->SetRotation(const_cast<QUAT &> (rotation));
	if(instance.fadeModel)
		instance.fadeModel->SetRotation(const_cast<QUAT &> (rotation));
}

void Storm3D_TerrainGroup::setInstanceLight(int modelId, int instanceId, int light, int lightId, const COL &color)
{
	assert(modelId >= 0 && instanceId >= 0);
	if(modelId < 0 || instanceId < 0 || modelId >= int(data->models.size()))
		return;

	SharedModel &original = data->models[modelId];
	if(instanceId >= int(original.instances.size()))
		return;

	if(light < 0 || light >= LIGHT_MAX_AMOUNT)
		return;

	Instance &instance = *original.instances[instanceId];
	instance.lightIndex[light] = lightId;

	/*
	if(light == 0)
	{
		instance.lightPosition1 = lightPosition;
		instance.lightColor1 = lightColor;
		instance.lightRange1 = lightRange;
	}
	else
	{
		instance.lightPosition2 = lightPosition;
		instance.lightColor2 = lightColor;
		instance.lightRange2 = lightRange;
	}
	*/

	if(instance.model)
	{
		//instance.model->SetLighting(light, lightPosition, lightColor, lightRange);
		instance.model->SetLighting(light, lightId);
		instance.model->SetSelfIllumination(color);
	}

	if(instance.fadeModel)
	{
		//instance.fadeModel->SetLighting(light, lightPosition, lightColor, lightRange);
		instance.fadeModel->SetLighting(light, lightId);
		instance.fadeModel->SetSelfIllumination(color);
	}
}

void Storm3D_TerrainGroup::setInstanceSun(int modelId, int instanceId, const VC3 &direction, float strength)
{
	assert(modelId >= 0 && instanceId >= 0);
	if(modelId < 0 || instanceId < 0 || modelId >= int(data->models.size()))
		return;

	SharedModel &original = data->models[modelId];
	if(instanceId >= int(original.instances.size()))
		return;

	Instance &instance = *original.instances[instanceId];
	if(instance.model)
		instance.model->SetDirectional(direction, strength);
	if(instance.fadeModel)
		instance.fadeModel->SetDirectional(direction, strength);
}

void Storm3D_TerrainGroup::setInstanceLightmapped(int modelId, int instanceId, bool lightmapped)
{
	assert(modelId >= 0 && instanceId >= 0);
	if(modelId < 0 || instanceId < 0 || modelId >= int(data->models.size()))
		return;

	SharedModel &original = data->models[modelId];
	if(instanceId >= int(original.instances.size()))
		return;

	Instance &instance = *original.instances[instanceId];
	instance.lightmapped = lightmapped;
	
	if(instance.model)
	{
		Storm3D_Model *m = static_cast<Storm3D_Model *> (instance.model.get());
		m->terrain_lightmapped_object = lightmapped;
	}
	if(instance.fadeModel)
	{
		Storm3D_Model *m = static_cast<Storm3D_Model *> (instance.fadeModel.get());
		m->terrain_lightmapped_object = lightmapped;
	}
}

void Storm3D_TerrainGroup::setInstanceFade(int modelId, int instanceId, float factor)
{
	assert(modelId >= 0 && instanceId >= 0);
	if(modelId < 0 || instanceId < 0 || modelId >= int(data->models.size()))
		return;

	SharedModel &original = data->models[modelId];
	if(instanceId >= int(original.instances.size()))
		return;

	Instance &instance = *original.instances[instanceId];
	if(factor < 0.02f)
		factor = 0.02f;

/*
	if(instance.model)
	{
		boost::scoped_ptr<Iterator<IStorm3D_Model_Object *> > objectIterator(instance.model->ITObject->Begin());
		for(; !objectIterator->IsEnd(); objectIterator->Next())
		{
			IStorm3D_Model_Object *object = objectIterator->GetCurrent();
			if(!object)
				continue;

			object->SetForceAlpha(1.f - factor);
		}
	}
*/
	if(instance.fadeModel)
	{
		boost::scoped_ptr<Iterator<IStorm3D_Model_Object *> > objectIterator(instance.fadeModel->ITObject->Begin());
		for(; !objectIterator->IsEnd(); objectIterator->Next())
		{
			IStorm3D_Model_Object *object = objectIterator->GetCurrent();
			if(!object)
				continue;

			object->SetForceAlpha(factor);
		}

		data->terrainModels.addModel(*instance.fadeModel);
	}
}

void Storm3D_TerrainGroup::setInstanceInBuilding(int modelId, int instanceId, bool inBuilding)
{
	assert(modelId >= 0 && instanceId >= 0);
	SharedModel &original = data->models[modelId];
	
	Instance &instance = *original.instances[instanceId];
	instance.inBuilding = inBuilding;

	if(instance.model)
		((Storm3D_Model *)instance.model.get())->terrain_inbuilding_object = inBuilding;
	if(instance.fadeModel)
		((Storm3D_Model *)instance.fadeModel.get())->terrain_inbuilding_object = inBuilding;
}

void Storm3D_TerrainGroup::setInstanceOccluded(int modelId, int instanceId, bool occluded)
{
	assert(modelId >= 0 && instanceId >= 0);
	SharedModel &original = data->models[modelId];
	
	Instance &instance = *original.instances[instanceId];

	if(instance.model)
		((Storm3D_Model *)instance.model.get())->SetOccluded(occluded);
	if(instance.fadeModel)
		((Storm3D_Model *)instance.fadeModel.get())->SetOccluded(occluded);
}

IStorm3D_Model * Storm3D_TerrainGroup::getInstanceModel ( int modelId, int instanceId)
{
	assert(modelId >= 0 && instanceId >= 0);
	if(modelId < 0 || instanceId < 0 || modelId >= int(data->models.size()))
		return NULL;

	SharedModel &original = data->models[modelId];
	if(instanceId >= int(original.instances.size()))
		return NULL;

	Instance &instance = *original.instances[instanceId];
	return instance.model.get();
}

void Storm3D_TerrainGroup::removeInstance(int modelId, int instanceId)
{
	assert(modelId >= 0 && instanceId >= 0);
	SharedModel &original = data->models[modelId];
	Instance &instance = *original.instances[instanceId];

	if(instance.model)
	{
		data->terrainModels.removeModel(*instance.model);

		instance.erase(data->tree.get(), data->terrainModels);
		instance.model.reset();
	}
	if(instance.fadeModel)
	{
		data->terrainModels.removeModel(*instance.fadeModel);
		instance.erase(data->tree.get(), data->terrainModels);
		instance.fadeModel.reset();
	}
}

void Storm3D_TerrainGroup::removeInstances()
{
	ModelList::iterator it = data->models.begin();
	for(; it != data->models.end(); ++it)
	{
		SharedModel &model = *it;
		
		InstanceList::iterator i = model.instances.begin();
		for(; i != model.instances.end(); ++i)
		{
			Instance &instance = *(*i);
			instance.erase(data->tree.get(), data->terrainModels);

			if(instance.model)
				data->terrainModels.removeModel(*instance.model);
			if(instance.fadeModel)
				data->terrainModels.removeModel(*instance.fadeModel);
		}

		model.instances.clear();   
	}

	data->instanceMap.clear();
}

void Storm3D_TerrainGroup::setInstanceColorsToMultiplier(const COL &color)
{
	/*
	ModelList::iterator it = data->models.begin();
	for(; it != data->models.end(); ++it)
	{
		SharedModel &model = *it;
		
		InstanceList::iterator i = model.instances.begin();
		for(i; i != model.instances.end(); ++i)
		{
			Instance &instance = *(*i);

			if(instance.model)
			{
				instance.model->SetSelfIllumination(instance.ambient * color);
				instance.model->SetLighting(instance.lightPosition, instance.lightColor * color);
			}
		}
	}
	*/
}

void Storm3D_TerrainGroup::enableCollision(bool enable)
{
	bool flag = false;
	if(!enable)
		flag = true;

	ModelList::iterator it = data->models.begin();
	for(; it != data->models.end(); ++it)
	{
		SharedModel &model = *it;
		
		InstanceList::iterator i = model.instances.begin();
		for(; i != model.instances.end(); ++i)
		{
			Instance &instance = *(*i);
			if(instance.model)
				instance.model->SetNoCollision(flag);
		}
	}
}

void Storm3D_TerrainGroup::enableBigCollision(bool enable)
{
	bool flag = false;
	if(!enable)
		flag = true;

	ModelList::iterator it = data->models.begin();
	for(; it != data->models.end(); ++it)
	{
		SharedModel &model = *it;
		if(static_cast<Storm3D_Model *> (model.model.get())->bounding_radius < 3.f)
			continue;
		
		InstanceList::iterator i = model.instances.begin();
		for(; i != model.instances.end(); ++i)
		{
			Instance &instance = *(*i);

			if(instance.model)
				instance.model->SetNoCollision(flag);
		}
	}
}

void Storm3D_TerrainGroup::enableLightmapCollision(bool enable)
{
	bool flag = false;
	if(!enable)
		flag = true;

	ModelList::iterator it = data->models.begin();
	for(; it != data->models.end(); ++it)
	{
		SharedModel &model = *it;
		
		InstanceList::iterator i = model.instances.begin();
		for(; i != model.instances.end(); ++i)
		{
			Instance &instance = *(*i);
			//if(instance.lightmapped && instance.model)
			//	instance.model->SetNoCollision(flag);
			if(instance.model)
			{
				if(instance.lightmapped)
					instance.model->SetNoCollision(flag);
				else
					instance.model->SetNoCollision(!flag);
			}
		}
	}
}

void Storm3D_TerrainGroup::setSceneSize(const VC3 &size)
{
	data->sceneSize = VC2(size.x, size.z);

	VC2 minSize(-size.x, -size.z);
	VC2 maxSize(size.x, size.z);

	data->tree.reset(new Tree(minSize, maxSize));
}

boost::shared_ptr<IStorm3D_TerrainModelIterator> Storm3D_TerrainGroup::getModelIterator(const VC3 &position, float radius)
{
	return boost::shared_ptr<IStorm3D_TerrainModelIterator> (new TerrainIterator(data->tree.get(), position, radius, data->terrainModels));
}

bool Storm3D_TerrainGroup::findObject(const VC3 &position, float radius, int &modelId, int &instanceId)
{
	Storm3D_CollisionInfo info;
	data->tree->SphereCollision(Sphere(position, radius), info, true);

	if(info.hit && info.model)
	{
		InstanceInfo instance = data->instanceMap[info.model];
		modelId = instance.modelId;
		instanceId = instance.instanceId;

		return true;
	}

	return false;
}
