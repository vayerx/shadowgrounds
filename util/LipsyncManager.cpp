#include <boost/shared_ptr.hpp>
#include <fstream>
#include <map>

#include "precompiled.h"

// Copyright 2002-2004 Frozenbyte Ltd.

#include "LipsyncManager.h"
#include "../sound/LipsyncManager.h"
#include "../editor/parser.h"
#include "../editor/string_conversions.h"
#include "../util/assert.h"
#include "../system/Timer.h"
#include "../system/Logger.h"
#include "../filesystem/file_package_manager.h"
#include "../filesystem/input_stream.h"
#include <IStorm3D.h>
#include <IStorm3D_Scene.h>
#include <IStorm3D_Texture.h>

using namespace boost;
using namespace std;
using namespace frozenbyte;
using namespace frozenbyte::editor;

namespace util {

	struct TextureReleaser
	{
		void operator () (IStorm3D_Texture *t) const
		{
			if(t)
				t->Release();
		}
	};

	struct Character
	{
		shared_ptr<IStorm3D_Model> model;

		shared_ptr<IStorm3D_Texture> picture;
		shared_ptr<IStorm3D_Material> pictureMaterial;
	};

	struct ActiveCharacter
	{
		IStorm3D_Texture *renderTarget;
		shared_ptr<IStorm3D_Model> model;
		shared_ptr<IStorm3D_Material> pictureMaterial;
		shared_ptr<IStorm3D_Scene> scene;

		int lightIndex;
		VC3 lightPosition;
		COL lightColor;
		COL ambient;
		float scale;

		VC3 cameraPosition;
		VC3 cameraTarget;
		std::string id;

		float aspectRatio;
		float noiseLevel;

		ActiveCharacter()
		:	renderTarget(0),
			lightIndex(-1),
			scale(1.f),
			aspectRatio(1.f),
			noiseLevel(0.3f)
		{
		}

		void update()
		{
			if(!renderTarget)
				return;

			const Storm3D_SurfaceInfo &info = renderTarget->GetSurfaceInfo();
			aspectRatio = float(info.width) / float(info.height);
		}

		void apply()
		{
			if(!model || !scene)
				return;

			IStorm3D_Camera &cam = *scene->GetCamera();
			cam.SetAspectRatio(aspectRatio);
			cam.SetPosition(cameraPosition);
			cam.SetTarget(cameraTarget);
			cam.SetUpVec(VC3(0,1.f,0));
			cam.SetVisibilityRange(100.f);
			cam.SetFieldOfView(PI / 5.5f);

			model->SetLighting(0, lightIndex);
			model->SetSelfIllumination(ambient);
			model->SetScale(VC3(scale, scale, scale));
		}

		void render()
		{
			apply();

			if(scene && renderTarget)
			{
				/*
				if(material && noiseLevel > 0.001f)
				{
					const Storm3D_SurfaceInfo &info = renderTarget->GetSurfaceInfo();

					float x1 = (rand() % 50) / 50.f;
					float y1 = (rand() % 50) / 50.f;
					float x2 = 1.5f + x1;
					float y2 = 1.5f + y1;

					material->SetTransparency(noiseLevel);
					scene->Render2D_Picture(material.get(), VC2(), VC2(float(info.width), float(info.height)), 1.f, 0.f, x1, y1, x2, y2);
				}
				*/

				if(pictureMaterial)
				{
					const Storm3D_SurfaceInfo &info = renderTarget->GetSurfaceInfo();
					scene->Render2D_Picture(pictureMaterial.get(), VC2(0,0), VC2(float(info.width), float(info.height)));
				}

				scene->RenderSceneToDynamicTexture(renderTarget);
			}
		}
	};

	typedef map<string, Character> CharacterMap;


struct LipsyncManager::Data
{
	IStorm3D *storm;
	IStorm3D_Terrain *terrain;
	//boost::shared_ptr<IStorm3D_Material> material[2];
	//boost::shared_ptr<IStorm3D_Texture> noise;

	CharacterMap characters;

	sfx::LipsyncManager manager;
	int lastUpdateTime;

	ActiveCharacter activeCharacters[2];
	bool active;
	int numCharacters;

	VC3 originalCameraPosition;
	VC3 originalCameraTarget;

	COL originalBackgroundColor;

	Data(IStorm3D *storm_, IStorm3D_Terrain *terrain_)
	:	storm(storm_),
		terrain(terrain_),
		manager(storm),
		lastUpdateTime(0),
		active(false),
		numCharacters(2)
	{
		FB_ASSERT(storm);
		lastUpdateTime = getCurrentTime();

		/*
		noise.reset(storm->CreateNewTexture("noise_02.dds"), TextureReleaser());
		if(noise)
		{
			for(int i = 0; i < 2; ++i)
			{
				material[i].reset(storm->CreateNewMaterial("lipsync_noise"));
				material[i]->SetBaseTexture(noise.get());
				//material[i]->SetAlphaType(IStorm3D_Material::ATYPE_NONE);
				material[i]->SetAlphaType(IStorm3D_Material::ATYPE_USE_TEXTRANSPARENCY);
				material[i]->SetTransparency(0.01f);
			}
		}
		*/

		originalCameraPosition = VC3(0.f,2.4f,-7.2f);
		originalCameraTarget = VC3(0,1.8f,0);
		originalBackgroundColor = COL(19.f/255.f, 19.f/255.f, 19.f/255.f);

		//for(int i = 0; i < 3; ++i)
		for(int i = 0; i < 2; ++i)
		{
			activeCharacters[i].scene.reset(storm->CreateNewScene());
			activeCharacters[i].scene->SetBackgroundColor(originalBackgroundColor);

			activeCharacters[i].lightPosition = VC3(-2.5f, 5.f, -10.f);
			activeCharacters[i].lightColor = COL(0.03f, 0.03f, 0.03f);
			activeCharacters[i].ambient = COL(0.3f, 0.3f, 0.3f);
			activeCharacters[i].scale = 10.f;
			activeCharacters[i].cameraPosition = originalCameraPosition;

			activeCharacters[i].cameraTarget = originalCameraTarget;
			activeCharacters[i].renderTarget = storm->getRenderTarget(i);

			if(i == 0)
			{
				activeCharacters[i].cameraPosition.x = -1.3f;
				activeCharacters[i].lightPosition.x = -2.5f;
			}
			else if(i == 1)
			{
				activeCharacters[i].cameraPosition.x =  1.3f;
				activeCharacters[i].lightPosition.x =  2.5f;
			}

			activeCharacters[i].lightIndex = terrain->addLight(activeCharacters[i].lightPosition, 20.f, activeCharacters[i].lightColor);
			activeCharacters[i].update();
		}
	}

	void init()
	{
		EditorParser parser(false, true);
		filesystem::InputStream lipsyncfile = filesystem::FilePackageManager::getInstance().getFile("Data/Animations/lipsync.txt");
		lipsyncfile >> parser;

		const ParserGroup &root = parser.getGlobals();
		const ParserGroup &chars = root.getSubGroup("Characters");
		for(int i = 0; i < chars.getSubGroupAmount(); ++i)
		{
			const std::string name = chars.getSubGroupName(i);
			const ParserGroup &group = chars.getSubGroup(name);
			const std::string model = group.getValue("model");
			const std::string bones = group.getValue("bones");
			const std::string picture = group.getValue("picture");

			Character &character = characters[name];

			if(!picture.empty())
			{
				IStorm3D_Material *material = storm->CreateNewMaterial("lipsync");
				IStorm3D_Texture *texture = storm->CreateNewTexture(picture.c_str());
				
				if(!texture)
				{
					string msg = "Cannot find lipsync picture: ";
					msg += picture;
					Logger::getInstance()->error(msg.c_str());
				}
				else
				{
					material->SetBaseTexture(texture);

					character.picture.reset(texture, TextureReleaser());
					character.pictureMaterial.reset(material);
				}
			}
			else if(!model.empty() && !bones.empty())
			{
				IStorm3D_Model *m = storm->CreateNewModel();
				if(!m->LoadS3D(model.c_str()))
				{
					string msg = "Cannot find lipsync model: ";
					msg += model;
					Logger::getInstance()->error(msg.c_str());
				}

				if(!m->LoadBones(bones.c_str()))
				{
					string msg = "Cannot find lipsync model bones: ";
					msg += bones;
					Logger::getInstance()->error(msg.c_str());
				}

				character.model.reset(m);
			}
		}
	}

	void setCharacter(CharPosition position, const string &id)
	{
		ActiveCharacter &ac = activeCharacters[position];
		if(ac.scene && ac.model)
			ac.scene->RemoveModel(ac.model.get());

		ac.id = id;

		if(id.empty())
		{
			ac.model.reset();
			ac.pictureMaterial.reset();
		}
		else
		{
			if(characters.find(id) == characters.end())
				Logger::getInstance()->error("LipsyncManager::Data::setCharacter -- cannot find given char id");

			Character &c = characters[id];
			ac.model = c.model;
			ac.pictureMaterial = c.pictureMaterial;

			if(ac.model && ac.scene)
			{
				ac.scene->AddModel(ac.model.get());
				ac.model->SetTypeFlag(position);
			}
		}
	}

	void setIdle(const string &id, const string &data, int fadeTime)
	{
		FB_ASSERT(!id.empty() && !data.empty());
		IStorm3D_Model *m = characters[id].model.get();

		if(m)
			manager.setIdle(m, data, fadeTime);
	}

	void setExpression(const string &id, const string &data, int fadeTime)
	{
		FB_ASSERT(!id.empty() && !data.empty());
		IStorm3D_Model *m = characters[id].model.get();

		if(m)
			manager.setExpression(m, data, fadeTime);
	}

	void play(const string &id, const boost::shared_ptr<sfx::AmplitudeArray> &amplitudes, int time)
	{
		FB_ASSERT(!id.empty());
		IStorm3D_Model *m = characters[id].model.get();

		if(m)
		{
			manager.play(m, amplitudes, time);
		}
	}

	void update()
	{
		if(!active)
			return;

		int newTime = getCurrentTime();
		int ms = newTime - lastUpdateTime;
		manager.update(ms, newTime);

		for(int i = 0; i < numCharacters; ++i)
		{
			ActiveCharacter &c = activeCharacters[i];
			c.render();
		}

		lastUpdateTime = newTime;
	}

	int getCurrentTime() const
	{
		return Timer::getTime();
	}
};

LipsyncManager::LipsyncManager(IStorm3D *storm, IStorm3D_Terrain *terrain)
{
	scoped_ptr<Data> tempData(new Data(storm, terrain));
	tempData->init();

	data.swap(tempData);
}

LipsyncManager::~LipsyncManager()
{
}

const std::string &LipsyncManager::getCharacter(CharPosition position) const
{
	return data->activeCharacters[position].id;
}

boost::shared_ptr<sfx::AmplitudeArray> LipsyncManager::getAmplitudeBuffer(const std::string &file) const
{
	return data->manager.getAmplitudeBuffer(file);
}

bool LipsyncManager::isActive() const
{
	return data->active;
}

void LipsyncManager::setCharacter(CharPosition position, const std::string &id)
{
	data->setCharacter(position, id);
}

void LipsyncManager::setIdle(const std::string &character, const std::string &idleAnimation, int fadeTime)
{
	data->setIdle(character, idleAnimation, fadeTime);
}

void LipsyncManager::setExpression(const std::string &character, const std::string &idleAnimation, int fadeTime)
{
	data->setExpression(character, idleAnimation, fadeTime);
}

void LipsyncManager::playSpeech(const std::string &character, const boost::shared_ptr<sfx::AmplitudeArray> &amplitudes, int time)
{
	data->play(character, amplitudes, time);
}

void LipsyncManager::setActive(bool active, int numChars)
{
	if(numChars >= 2)
		numChars = 2;

	data->active = active;
	data->numCharacters = numChars;
}

void LipsyncManager::update()
{
	data->update();
}

void LipsyncManager::setCamera(CharPosition pos, const VC3 &cameraPos, const VC3 &cameraTarget, float aspectRatio)
{
	data->activeCharacters[pos].cameraPosition = cameraPos;
	data->activeCharacters[pos].cameraTarget = cameraTarget;

	if(aspectRatio > 0.0f)
	{
		data->activeCharacters[pos].aspectRatio = aspectRatio;
	}
}

void LipsyncManager::resetCamera(CharPosition pos)
{
	data->activeCharacters[pos].cameraPosition = data->originalCameraPosition;
	data->activeCharacters[pos].cameraTarget = data->originalCameraTarget;
	data->activeCharacters[pos].update();
}

void LipsyncManager::setBackground(CharPosition pos, COL color)
{
	data->activeCharacters[pos].scene->SetBackgroundColor(color);
}

void LipsyncManager::resetBackground(CharPosition pos)
{
	data->activeCharacters[pos].scene->SetBackgroundColor(data->originalBackgroundColor);
}

} // util
