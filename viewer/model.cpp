// Copyright 2002-2004 Frozenbyte Ltd.

#include "model.h"
#include "bone_items.h"
#include "../editor/storm.h"
#include "../editor/dialog.h"
#include "../editor/parser.h"
#include "../editor/storm_model_utils.h"
#include "../filesystem/input_file_stream.h"

#include <set>
#include <map>
#include <fstream>
#include <vector>
#include <cassert>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <istorm3d.h>
#include <istorm3d_bone.h>
#include <istorm3d_mesh.h>
#include <istorm3d_terrain_renderer.h>

namespace frozenbyte {
namespace viewer {
namespace {

	struct Animation
	{
		IStorm3D_BoneAnimation *animation;

		Animation(IStorm3D_BoneAnimation *animation_)
		:	animation(animation_)
		{
		}

		~Animation()
		{
			if(animation)
				animation->Release();
		}
	};

} // end of unnamed namespace

int BLEND_TIME = 300;
//int BLEND_TIME = 1000;

struct ModelData
{
	editor::Storm &storm;
	boost::shared_ptr<IStorm3D_Model> model;

	std::set<std::string> fileNames;
	std::string bones;

	std::vector<std::string> animations[2];
	std::map<std::string, boost::shared_ptr<Animation> > stormAnimations;

	float yAngle;
	float xAngle;
	float scale;

	BoneItems boneItems;

	ModelData(editor::Storm &storm_)
	:	storm(storm_)
	{
		yAngle = 0;
		xAngle = 0;
		scale = 1.f;
	}

	void createModel()
	{
		if(model)
			storm.scene->RemoveModel(model.get());

		boost::shared_ptr<IStorm3D_Model> tempModel(storm.storm->CreateNewModel());
		model = tempModel;

		storm.scene->AddModel(model.get());
		if(model)
		{
			model->SetScale(VC3(scale, scale, scale));
			model->FreeMemoryResources();
		}
	}

	void addModel(const std::string &fileName)
	{
		boost::shared_ptr<IStorm3D_Model> sourceModel(storm.storm->CreateNewModel());
		sourceModel->LoadS3D(fileName.c_str());

		if(model)
		{
			editor::addCloneModel(sourceModel, model);
			model->FreeMemoryResources();
		}
	}

	void reloadData()
	{
		if(fileNames.empty())
			return;

		createModel();
		std::set<std::string>::iterator it = fileNames.begin();
		model->LoadS3D((*it).c_str());

		++it;
		for(; it != fileNames.end(); ++it)
			addModel(*it);

		if(!bones.empty())
			model->LoadBones(bones.c_str());

		/*
		//IStorm3D_Texture *t = storm.storm->CreateNewTexture("fractalreflections.dds");
		//IStorm3D_Texture *t = storm.storm->CreateNewTexture("reflection.dds");
		//IStorm3D_Texture *t = storm.storm->CreateNewTexture("cube.dds");
		//IStorm3D_Texture *t = storm.storm->CreateNewTexture("cube_map_test1.dds");
		//IStorm3D_Texture *t = storm.storm->CreateNewTexture("cref.dds");

		// HAX HAX
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
				IStorm3D_Material *material = mesh->GetMaterial();
				if(!material)
					continue;

				//material->SetReflectionTexture(t, IStorm3D_Material::TEX_GEN_REFLECTION, IStorm3D_Material::MTL_BOP_ADD, 0.5f);
				material->SetAlphaType(IStorm3D_Material::ATYPE_USE_TRANSPARENCY);
				material->SetTransparency(0.5f);
			}
		}
		*/

		model->FreeMemoryResources();
	}

	IStorm3D_BoneAnimation *getAnimation(int groupIndex, int index)
	{
		std::string &fileName = animations[groupIndex][index];
		std::map<std::string, boost::shared_ptr<Animation> >::iterator it = stormAnimations.find(fileName);

		if(it != stormAnimations.end())
			return (*it).second.get()->animation;

		boost::shared_ptr<Animation> animation(new Animation(storm.storm->CreateNewBoneAnimation(fileName.c_str())));
		stormAnimations[fileName] = animation;

		return animation.get()->animation;
	}
};

Model::Model(editor::Storm &storm)
{
	boost::scoped_ptr<ModelData> tempData(new ModelData(storm));
	data.swap(tempData);
}

Model::~Model()
{
}

void Model::loadGeometry(const std::string &fileName)
{
	data->fileNames.clear();
	data->bones.clear();
	data->animations[0].clear();
	data->animations[1].clear();
	data->yAngle = 0;
	data->xAngle = 0;

	data->createModel();
	data->model->LoadS3D(fileName.c_str());
	data->model->FreeMemoryResources();

	data->fileNames.insert(fileName);
}

void Model::addGeometry(const std::string &fileName)
{
	if(!data->model)
		return;

	if(data->fileNames.find(fileName) == data->fileNames.end())
		data->fileNames.insert(fileName);
	else
		return;

	data->addModel(fileName);
}

void Model::loadBones(const std::string &fileName)
{
	if(data->model)
	{
		data->model->LoadBones(fileName.c_str());
		data->bones = fileName;
	}
}

void Model::setScale(float scale)
{
	data->scale = scale;
	if(data->model)
		data->model->SetScale(VC3(scale, scale, scale));
}

void Model::save(const std::string &fileName)
{
	std::ofstream stream(fileName.c_str());
	editor::Parser parser;

	editor::ParserGroup &models = parser.getGlobals().getSubGroup("Models");
	for(std::set<std::string>::iterator it = data->fileNames.begin(); it != data->fileNames.end(); ++it)
		models.addLine(*it);

	parser.getGlobals().setValue("Bones", data->bones);

	for(int i = 0; i < 2; ++i)
	for(unsigned int j = 0; j < data->animations[i].size(); ++j)
		parser.getGlobals().getSubGroup("Animations").getSubGroup(std::string("Animation") + boost::lexical_cast<std::string> (i)).addLine(data->animations[i][j]);

	data->boneItems.save(parser);
	stream << parser;
}

void Model::load(const std::string &fileName)
{
	data->fileNames.clear();
	data->bones.clear();
	data->animations[0].clear();
	data->animations[1].clear();
	data->yAngle = 0;
	data->xAngle = 0;

	editor::Parser parser;
	filesystem::createInputFileStream(fileName.c_str()) >> parser;

	editor::ParserGroup &models = parser.getGlobals().getSubGroup("Models");
	for(int i = 0; i < models.getLineCount(); ++i)
		data->fileNames.insert(models.getLine(i));

	data->bones = parser.getGlobals().getValue("Bones");
	data->reloadData();

	for(int j = 0; j < 2; ++j)
	{
		editor::ParserGroup &group = parser.getGlobals().getSubGroup("Animations").getSubGroup(std::string("Animation") + boost::lexical_cast<std::string> (j));
		for(int i = 0; i < group.getLineCount(); ++i)
			data->animations[j].push_back(group.getLine(i));

		std::sort(data->animations[j].begin(), data->animations[j].end());
	}

	data->boneItems.load(parser);
	data->boneItems.applyToModel(data->model, data->storm);
}

void Model::reload(const editor::Dialog &dialog)
{
	//data->model.reset();
	//data->stormAnimations.clear();
	freeResources();
	data->storm.recreate(dialog.getWindowHandle(), true);

	if(data->storm.storm && data->storm.scene)
	{
		// Test
		unsigned short buffer[32*32] = { 0 };
		data->storm.terrain = data->storm.storm->CreateNewTerrain(32);
		data->storm.terrain->setHeightMap(buffer, VC2I(32,32), VC3(500,.01f,500), 4, 0, 1, 1);

		/*
		{
			IStorm3D_Texture *t = data->storm.storm->CreateNewTexture("missing.dds");
			int textureIndex = data->storm.terrain->addTerrainTexture(*t);

			DWORD buffer[32*32] = { 0 };
			for(int i = 0; i < 32*32; ++i)
				buffer[i] = 0x10101010;

			IStorm3D_Texture *blend = data->storm.storm->CreateNewTexture(32, 32, IStorm3D_Texture::TEXTYPE_BASIC);
			blend->Copy32BitSysMembufferToTexture(buffer);
			for(int i = 0; i < 16; ++i)
			{
				data->storm.terrain->setBlendMap(i, *blend, textureIndex, -1);
			}
		}
		*/

		//data->storm.terrain->getRenderer().setRenderMode(IStorm3D_TerrainRenderer::TexturesOnly);
		data->storm.scene->AddTerrain(data->storm.terrain);
		data->storm.viewerCamera = true;

		//data->reloadData();
		//data->boneItems.applyToModel(data->model, data->storm);
		//rotateModel(0, 0);
		loadResources();
	}
}

void Model::freeResources()
{
	data->model.reset();
	data->stormAnimations.clear();
}

void Model::loadResources()
{
	data->reloadData();
	data->boneItems.applyToModel(data->model, data->storm);
	rotateModel(0, 0);
}

void Model::rotateModel(float yAngleDelta, float xAngleDelta)
{
	if(!data->model)
		return;

	data->yAngle += yAngleDelta;
	data->xAngle += xAngleDelta;

	QUAT a;
	a.MakeFromAngles(0, data->yAngle, 0);
	QUAT b;
	b.MakeFromAngles(data->xAngle, 0, 0);
	QUAT quaternion = a * b;
	
	data->model->SetRotation(quaternion);
}

void Model::addAnimation(int groupIndex, const std::string &fileName)
{
	assert((groupIndex >= 0) && (groupIndex < 2));
	if(std::find(data->animations[groupIndex].begin(), data->animations[groupIndex].end(), fileName) != data->animations[groupIndex].end())
		return;

	data->animations[groupIndex].push_back(fileName);
	std::sort(data->animations[groupIndex].begin(), data->animations[groupIndex].end());
}

void Model::removeAnimation(int groupIndex, int index)
{
	assert((groupIndex >= 0) && (groupIndex < 2));
	assert((index >= 0) && (index < int(data->animations[groupIndex].size())));

	data->animations[groupIndex].erase(data->animations[groupIndex].begin() + index);
}

void Model::playAnimation(int groupIndex, int index, bool loop)
{
	//assert((groupIndex >= 0) && (groupIndex < 2));
	//assert((index >= 0) && (index < int(data->animations[groupIndex].size())));
	if(groupIndex < 0 || groupIndex >= 2 || index < 0 || index >= int(data->animations[groupIndex].size()))
	{
		return;
	}

	IStorm3D_BoneAnimation *animation = data->getAnimation(groupIndex, index);
	if(groupIndex == 0)
		data->model->BlendToAnimation(0, animation, BLEND_TIME, loop);
	else
		data->model->BlendWithAnimationIn(0, animation, BLEND_TIME, loop);
	data->model->SetAnimationPaused(false);
}

int Model::getAnimationTime(int groupIndex, int index)
{
	if(groupIndex < 0 || groupIndex >= 2 || index < 0 || index >= int(data->animations[groupIndex].size()))
	{
		return 0;
	}

	IStorm3D_BoneAnimation *animation = data->getAnimation(groupIndex, index);
	return data->model->GetAnimationTime(animation);
}

void Model::stopAnimation(int groupIndex, int index)
{
	assert((groupIndex >= 0) && (groupIndex < 2));
	assert((index >= 0) && (index < int(data->animations[groupIndex].size())));

	if(groupIndex == 0)
		data->model->SetAnimation(0, 0, false);
	else
	{
		IStorm3D_BoneAnimation *animation = data->getAnimation(groupIndex, index);
		data->model->BlendWithAnimationOut(0, animation, BLEND_TIME);
	}
}

void Model::setAnimationSpeedFactor(int groupIndex, int index, float speed)
{
	assert((groupIndex >= 0) && (groupIndex < 2));
	assert((index >= 0) && (index < int(data->animations[groupIndex].size())));

	IStorm3D_BoneAnimation *animation = data->getAnimation(groupIndex, index);
	data->model->SetAnimationSpeedFactor(animation, speed);
}

void Model::setAnimationPaused(bool paused)
{
	data->model->SetAnimationPaused(paused);
}

int Model::getAnimationCount(int groupIndex) const
{
	assert((groupIndex >= 0) && (groupIndex < 2));
	return data->animations[groupIndex].size();
}

std::string Model::getAnimation(int groupIndex, int index) const
{
	assert((groupIndex >= 0) && (groupIndex < 2));
	assert((index >= 0) && (index < int(data->animations[groupIndex].size())));

	return data->animations[groupIndex][index];
}

void Model::attachItems()
{
	data->boneItems.showDialog(data->model.get());
	data->boneItems.applyToModel(data->model, data->storm);
}

IStorm3D_Model *Model::getModel() const
{
	return data->model.get();
}

bool Model::hasModel() const
{
	return data->model;
}

} // end of namespace viewer
} // end of namespace frozenbyte
