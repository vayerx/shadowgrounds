
#include "precompiled.h"

#include <string>
#include <vector>
#include <list>
#include <fstream>
#include <map>
#include <vector>
#include <boost/lexical_cast.hpp>

#include "EnvironmentalEffectManager.h"
#include "EnvironmentalEffect.h"
#include "../ui/VisualEffectManager.h"
#include "../ui/VisualEffect.h"
#include "../ui/Spotlight.h"
#include "../container/LinkedList.h"
#include "../system/Logger.h"
#include "../editor/parser.h"
#include "../filesystem/file_package_manager.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_graphics.h"
#include "../convert/str2int.h"
#include "Game.h"
#include "scripting/GameScripting.h"
#include "GameUI.h"
#include "GameScene.h"
#include <istorm3D_terrain_renderer.h>
#include <Storm3D_UI.h>

#include "../editor/parser.h"
#include "../particle_editor2/particleeffect.h"
#include "../particle_editor2/track.h"
#include "../particle_editor2/particlesystem.h"
#include "../particle_editor2/particleforces.h"

#include "../util/Debug_MemoryManager.h"

using namespace frozenbyte;

namespace game {

struct Group
{
	std::vector<std::string> effects;
};

struct GroupList
{
	Group group[5];

	bool lightningEnabled;
	int lightningVariation;
	int lightningMin;

	GroupList()
	:	lightningEnabled(false),
		lightningVariation(0),
		lightningMin(0)
	{
	}
};

typedef std::map<std::string, GroupList> EffectGroup;


struct EnvironmentalEffectManagerImpl
{
	Game *game;
	ui::VisualEffectManager *visualEffectManager;
	boost::scoped_ptr<LinkedList> effects;

	bool sunlightEnabled;
	VC3 sunlightDirection;
	COL sunlightColor;
	float sunlightRange;
	ui::Spotlight *sunlightSpot;

	int fadeTime;
	EffectGroup effectGroup;

	EnvironmentalEffectManagerImpl(Game *game_, ui::VisualEffectManager *visualEffectManager_)
	:	game(game_),
		visualEffectManager(visualEffectManager_),
		effects(new LinkedList()),
		fadeTime(10000)
	{
		sunlightDirection = VC3(0,-1,0);
		sunlightColor = COL(0,0,0);
		sunlightEnabled = false;
		sunlightSpot = NULL;
	};

	~EnvironmentalEffectManagerImpl()
	{
		while (!effects->isEmpty())
		{
			delete (EnvironmentalEffect *)effects->popLast();
		}
	}

	void initScript()
	{
		game->addCustomScriptProcess("random_lightning", 0, NULL);
	}

	void init()
	{
		editor::EditorParser parser(true, false);
#ifdef LEGACY_FILES
		filesystem::InputStream f = filesystem::FilePackageManager::getInstance().getFile("Data/Effects/environmental_effect_groups.txt");
		f >> parser;
#else
		filesystem::InputStream f = filesystem::FilePackageManager::getInstance().getFile("data/effect/environmental_effect_groups.txt");
		f >> parser;
#endif

		const editor::ParserGroup &global = parser.getGlobals();
		try
		{
			fadeTime = boost::lexical_cast<int> (global.getValue("fade_time"));
		}
		catch(...)
		{
			fadeTime = 10000;
		}

		for(int i = 0; i < global.getSubGroupAmount(); ++i)
		{
			const std::string &name = global.getSubGroupName(i);

			if(!name.empty())
				parseGroupList(effectGroup[name], global.getSubGroup(name));
		}
	}

	void parseGroupList(GroupList &list, const editor::ParserGroup &group)
	{
		try
		{
			list.lightningEnabled = boost::lexical_cast<bool> (group.getValue("lightning_enabled"));
			list.lightningVariation = boost::lexical_cast<int> (group.getValue("lightning_variation"));
			list.lightningMin = boost::lexical_cast<int> (group.getValue("lightning_min"));
		}
		catch(...)
		{
			list.lightningEnabled = false;
		}

		parseGroup(list.group[0], group.getSubGroup("VeryLow"));
		parseGroup(list.group[1], group.getSubGroup("Low"));
		parseGroup(list.group[2], group.getSubGroup("Medium"));
		parseGroup(list.group[3], group.getSubGroup("High"));
		parseGroup(list.group[4], group.getSubGroup("VeryHigh"));
	}

	void parseGroup(Group &g, const editor::ParserGroup &group)
	{
		for(int i = 0; i < group.getLineCount(); ++i)
		{
			const std::string &line = group.getLine(i);
			g.effects.push_back(line);
		}
	}

	void setEffectGroup(const char *name, bool fade)
	{
		if(!name)
			return;

		std::string nameStr = name;
		const GroupList &list = effectGroup[nameStr];

		int index = SimpleOptions::getInt(DH_OPT_I_PARTICLE_EFFECTS_LEVEL);
		if(index < 0)
			index = 0;
		if(index > 100)
			index = 100;
		index /= 25;

		const Group &group = list.group[index];
		for(unsigned int i = 0; i < group.effects.size(); ++i)
		{
			const std::string &str = group.effects[i];
			addParticleEffect(str.c_str(), fade);
		}

		if(list.lightningEnabled && SimpleOptions::getBool(DH_OPT_B_WEATHER_EFFECTS))
		{
			game->gameScripting->setGlobalIntVariableValue("random_lightning_enabled", list.lightningEnabled);
			game->gameScripting->setGlobalIntVariableValue("random_lightning_wait_variation", list.lightningVariation);
			game->gameScripting->setGlobalIntVariableValue("random_lightning_wait_min", list.lightningMin);
		}
		else
			game->gameScripting->setGlobalIntVariableValue("random_lightning_enabled", 0);
	}

	void addParticleEffect(const char *particleFilename, bool fade)
	{
		if(particleFilename)
		{
			// TODO: convert particle filename to something else?
			int visualEffectId = visualEffectManager->getVisualEffectIdByName(particleFilename);
			if (visualEffectId != -1)
			{
				ui::VisualEffect *veff = visualEffectManager->createNewVisualEffect(visualEffectId,
					NULL, NULL, VC3(0,0,0), VC3(0,0,0), VC3(0,0,0), VC3(0,0,0), game);

				assert(veff != NULL);
				veff->addReference();

				EnvironmentalEffect *eff = new EnvironmentalEffect(particleFilename, veff);
				effects->append(eff);

				if(fade && eff)
					eff->fadeIn(fadeTime);
					
			} 
			else 
				Logger::getInstance()->error("EnvironmentalEffectManager::addParticleEffect - Effect with given name not found.");
		} 
		else 
			Logger::getInstance()->error("EnvironmentalEffectManager::addParticleEffect - Null particle filename given.");
	}

	void deleteEnvironmentalEffect(EnvironmentalEffect *effect)
	{
		// TODO: assert, sure it is in the list?
		effects->remove(effect);
		delete effect;
	}
};

// --

EnvironmentalEffectManager::EnvironmentalEffectManager(Game *game, ui::VisualEffectManager *visualEffectManager)
{
	impl = new EnvironmentalEffectManagerImpl(game, visualEffectManager);
	impl->init();
}

EnvironmentalEffectManager::~EnvironmentalEffectManager()
{
	removeAllParticleEffects();
	delete impl;
}

void EnvironmentalEffectManager::addParticleEffect(const char *particleFilename, bool fade)
{
	impl->addParticleEffect(particleFilename, fade);
	/*
	if (particleFilename != NULL)
	{
		// TODO: convert particle filename to something else???
		int visualEffectId = impl->visualEffectManager->getVisualEffectIdByName(particleFilename);
		if (visualEffectId != -1)
		{
			ui::VisualEffect *veff = impl->visualEffectManager->createNewVisualEffect(visualEffectId,
				NULL, NULL, VC3(0,0,0), VC3(0,0,0), VC3(0,0,0), VC3(0,0,0), impl->game);

			assert(veff != NULL);
			veff->addReference();

			EnvironmentalEffect *eff = new EnvironmentalEffect(particleFilename, veff);
			impl->effects->append(eff);

			if(fade && eff)
				eff->fadeIn(impl->fadeTime);
				
		} 
		else 
		{
			Logger::getInstance()->error("EnvironmentalEffectManager::addParticleEffect - Effect with given name not found.");
		}
	} 
	else 
	{
		Logger::getInstance()->error("EnvironmentalEffectManager::addParticleEffect - Null particle filename given.");
	}
	*/
}

void EnvironmentalEffectManager::removeParticleEffectByFilename(const char *particleFilename, bool fade)
{
	LinkedListIterator iter(impl->effects.get());
	while (iter.iterateAvailable())
	{
		EnvironmentalEffect *eff = (EnvironmentalEffect *)iter.iterateNext();
		if(eff->filename == particleFilename)
		{
			if(fade)
			{
				eff->fadeOut(impl->fadeTime);
				break;
			}
			else
			{
				impl->deleteEnvironmentalEffect(eff);
				break;
			}
		}
	}
}

void EnvironmentalEffectManager::removeAllParticleEffects()
{
	while(!impl->effects->isEmpty())
	{
		EnvironmentalEffect *eff = (EnvironmentalEffect *)impl->effects->popLast();
		impl->deleteEnvironmentalEffect(eff);
	}
}

void EnvironmentalEffectManager::fadeOutAllParticleEffects(int time)
{
	if(time < 0) time = impl->fadeTime; 
	LinkedListIterator iter(impl->effects.get());
	while (iter.iterateAvailable())
	{
		EnvironmentalEffect *eff = (EnvironmentalEffect *)iter.iterateNext();
		eff->fadeOut(time);
	}
}

void EnvironmentalEffectManager::fadeInAllParticleEffects(int time)
{
	if(time < 0) time = impl->fadeTime;
	LinkedListIterator iter(impl->effects.get());
	while (iter.iterateAvailable())
	{
		EnvironmentalEffect *eff = (EnvironmentalEffect *)iter.iterateNext();
		eff->fadeIn(time);
	}
}

void EnvironmentalEffectManager::setEffectGroup(const char *group)
{
	removeAllParticleEffects();
	impl->setEffectGroup(group, false);
}

void EnvironmentalEffectManager::fadeEffectGroup(const char *group)
{
	fadeOutAllParticleEffects();
	impl->setEffectGroup(group, true);
}

void EnvironmentalEffectManager::init()
{
	impl->initScript();
}

void EnvironmentalEffectManager::run()
{
	frozenbyte::particle::WindParticleForce::advanceWind((float)GAME_TICK_MSEC / 1000.0f);

	VC3 effectPosition = impl->game->gameUI->getGameCamera()->getPosition();

	if(!impl->game->isCinematicScriptRunning() && impl->game->gameUI->getCameraNumber() == GAMEUI_CAMERA_NORMAL)
	{
		if(impl->game->gameUI->getFirstPerson(0))
			effectPosition = impl->game->gameUI->getFirstPerson(0)->getPosition();
	}

	SafeLinkedListIterator iter(impl->effects.get());
	while(iter.iterateAvailable())
	{
		EnvironmentalEffect *eff = (EnvironmentalEffect *)iter.iterateNext();
		eff->setPosition(effectPosition);
		eff->update(GAME_TICK_MSEC);

		if(eff->isFinished())
			impl->deleteEnvironmentalEffect(eff);
	}

	updateSunlightFocus();
}

void EnvironmentalEffectManager::disableSunlight()
{
	impl->sunlightEnabled = false;
	delete impl->sunlightSpot;
	impl->sunlightSpot = 0;

#if defined(PROJECT_CLAW_PROTO) || defined(PROJECT_SURVIVOR)
		impl->game->gameUI->getTerrain()->GetTerrain()->getRenderer().enableFeature(IStorm3D_TerrainRenderer::ForcedDirectionalLightEnabled, false);
#endif
}

void EnvironmentalEffectManager::enableSunlight()
{
	impl->sunlightEnabled = true;
	if(!impl->sunlightSpot)
	{
#if !defined(PROJECT_CLAW_PROTO) && !defined(PROJECT_SURVIVOR)
//#if !defined(PROJECT_CLAW_PROTO)
		impl->sunlightSpot = new ui::Spotlight(*impl->game->gameUI->getStorm3D(), 
			*impl->game->gameUI->getTerrain()->GetTerrain(), 
			*impl->game->getGameScene()->getStormScene(), 0, 
			"sunlight");
		impl->sunlightSpot->setRange ( 500.0f );
#else
		impl->game->gameUI->getTerrain()->GetTerrain()->getRenderer().enableFeature(IStorm3D_TerrainRenderer::ForcedDirectionalLightEnabled, true);
#endif
		updateSunlightFocus();
	}
}

void EnvironmentalEffectManager::setSunlightDirection(const VC3 &direction)
{
	impl->sunlightDirection = direction;
}

void EnvironmentalEffectManager::setSunlightColor(const COL &color)
{
	impl->sunlightColor = color;
}

void EnvironmentalEffectManager::updateSunlightFocus()
{
#if !defined(PROJECT_CLAW_PROTO) && !defined(PROJECT_SURVIVOR)
//#if !defined(PROJECT_CLAW_PROTO)
	if (impl->sunlightSpot != NULL)
	{      
		GameCamera *gcam = impl->game->gameUI->getGameCamera();
		VC3 pos = gcam->getPosition();
		impl->game->gameMap->keepWellInScaledBoundaries(&pos.x, &pos.z);
		pos.y = impl->game->gameMap->getScaledHeightAt(pos.x, pos.z);

		pos -= impl->sunlightDirection * 100.0f;

		impl->sunlightSpot->setPosition(pos);
		impl->sunlightSpot->setDirection(impl->sunlightDirection);
		impl->sunlightSpot->setSpotlightParams(impl->sunlightColor);
	}
#else
	if(impl->sunlightEnabled)
	{
		impl->game->gameUI->getTerrain()->GetTerrain()->getRenderer().setColorValue(IStorm3D_TerrainRenderer::ForcedDirectionalLightColor, impl->sunlightColor);
		impl->game->gameUI->getTerrain()->GetTerrain()->getRenderer().setVectorValue(IStorm3D_TerrainRenderer::ForcedDirectionalLightDirection, -impl->sunlightDirection);
	}
#endif
}

} // game
