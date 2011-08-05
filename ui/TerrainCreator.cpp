
#include "precompiled.h"

#include "TerrainCreator.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_memory.h"
#include "../util/ModelTextureUncompress.h"
#include "../util/Parser.h"
#include "../util/LightAmountManager.h"
#include "../game/GameMap.h"
#include "../ui/Terrain.h"
#include <Storm3D_UI.h>
#include <istorm3d_terrain_decalsystem.h>

#include <time.h>
#include "../util/Debug_MemoryManager.h"

#include "../util/fb_assert.h"

namespace ui {

TerrainCreator::TerrainCreator(IStorm3D *s3d, IStorm3D_Scene *scene)
{
	storm3d = s3d;
	this->scene = scene;
}

TerrainCreator::~TerrainCreator()
{
    //cannot do it like this...
    //unloadTextures();
	storm3d = NULL;
	scene = NULL;
}

void TerrainCreator::setCameraRange(float range)
{
	cameraRange = range;
}

Terrain *TerrainCreator::createTerrain(game::GameMap *gameMap, ui::LightManager *lightManager, ui::AmbientSoundManager *ambientSoundManager, const char *configFile)
{
	/*
	std::string textFileName = configFile;
	textFileName += "/legacy.txt";

	Parser::Parser parser(textFileName.c_str());
	const Parser::string_map scene_properties = parser.GetProperties();

	std::string forceMapName = Parser::GetString(scene_properties, "forceheightmap");
	*/

	/*if (configFile == NULL)
	{
		fb_assert(!"TerrainCreator::createTerrain - Null configFile parameter given.");
	}*/

	std::string forceMapName;
	Terrain *terrain = new Terrain(storm3d, scene, configFile, forceMapName.c_str(), gameMap->getAreaMap(), gameMap, lightManager, ambientSoundManager);

	int memRes = game::SimpleOptions::getInt(DH_OPT_I_TERRAIN_MEMORY_RESERVE);
	if (memRes < 0) memRes = 0;
	int preCache = game::SimpleOptions::getInt(DH_OPT_I_TERRAIN_MEMORY_PRECACHE);

	if (preCache < 0) 
		preCache = 0;
	if (preCache > memRes) 
		preCache = memRes;

//	terrain->GetTerrain()->SetMemoryReserve(memRes);
//	terrain->GetTerrain()->SetPreCache(preCache);

	std::string binFileString = configFile;
	binFileString += "/scene.bin";

	// These maps should really come from bin file

	gameMap->setData(terrain->GetHeightMap(), terrain->GetDoubleHeightMap(), terrain->getHeightMapSize(), 
	  terrain->getTerrainSize(), terrain->getTerrainHeight(), binFileString.c_str());
	gameMap->setTerrain(terrain->GetTerrain());
	
	// set proper maps and stuff to light amount manager 
	util::LightAmountManager::getInstance()->setMaps(gameMap, terrain->GetTerrain());
	terrain->GetTerrain()->setObstacleHeightmap(gameMap->getObstacleHeightMap(), gameMap->getAreaMap());

	// Haxhax, initialize here for now
	IStorm3D_Material *shadowMaterial = storm3d->CreateNewMaterial("shadow_decal");
	shadowMaterial->SetBaseTexture(storm3d->CreateNewTexture("shadow_decal.tga"));
	terrain->GetTerrain()->getDecalSystem().setShadowMaterial(shadowMaterial);

	return terrain;
}

/*
Terrain *TerrainCreator::createTerrain(game::GameMap *gameMap, const char *configFile)
{
	Parser::Parser scene_config(configFile);
	const Parser::string_map scene_properties = scene_config.FindGroup("Scene").GetProperties();

	Terrain *terrain = new Terrain(storm3d, scene);
	/ psd /
	//terrain->Create(scene_config.FindGroup("Terrain"));

	float sun_r = Parser::GetFloat(scene_properties, "sun_color_r");
	float sun_g = Parser::GetFloat(scene_properties, "sun_color_g");
	float sun_b = Parser::GetFloat(scene_properties, "sun_color_b");
	float sun_x = Parser::GetFloat(scene_properties, "sun_direction_x");
	float sun_y = Parser::GetFloat(scene_properties, "sun_direction_y");
	float sun_z = Parser::GetFloat(scene_properties, "sun_direction_z");

	// fog...
	float fog_r = Parser::GetFloat(scene_properties, "fog_color_r");
	float fog_g = Parser::GetFloat(scene_properties, "fog_color_g");
	float fog_b = Parser::GetFloat(scene_properties, "fog_color_b");
	float fog_start = Parser::GetFloat(scene_properties, "fog_start");
	float fog_end = Parser::GetFloat(scene_properties, "fog_end");
	float max_camera_range = Parser::GetFloat(scene_properties, "max_camera_range");

	float shadow_r = Parser::GetFloat(scene_properties, "shadow_color_r");
	float shadow_g = Parser::GetFloat(scene_properties, "shadow_color_g");
	float shadow_b = Parser::GetFloat(scene_properties, "shadow_color_b");
	int shadow_darkness = Parser::GetInt(scene_properties, "shadow_darkness");
	int shadow_obstacle_height = Parser::GetInt(scene_properties, "shadow_obstacle_height");
	bool shadow_smooth = false;
	if (Parser::GetInt(scene_properties, "shadow_smooth") == 1)
		shadow_smooth = true;
		
	COL shadCol = COL(shadow_r, shadow_g, shadow_b);

	terrain->GetTerrain()->SetShadow(shadow_darkness, shadow_obstacle_height, shadCol, shadow_smooth);

	int memRes = game::SimpleOptions::getInt(DH_OPT_I_TERRAIN_MEMORY_RESERVE);
	if (memRes < 0) memRes = 0;
	int preCache = game::SimpleOptions::getInt(DH_OPT_I_TERRAIN_MEMORY_PRECACHE);
	
	if (preCache < 0) 
		preCache = 0;
	if (preCache > memRes) 
		preCache = memRes;

	terrain->GetTerrain()->SetMemoryReserve(memRes);
	terrain->GetTerrain()->SetPreCache(preCache);

	bool hasFog = true;
	if (fog_start > 0 || fog_end > 0)
	{
		hasFog = true;
	} 
	else 
	{
		hasFog = false;
	}

	COL fogColor = COL(fog_r, fog_g, fog_b);
	scene->SetFogParameters(hasFog, fogColor, fog_start, fog_end);

	// ambient...
	float ambient_r = Parser::GetFloat(scene_properties, "ambient_color_r");
	float ambient_g = Parser::GetFloat(scene_properties, "ambient_color_g");
	float ambient_b = Parser::GetFloat(scene_properties, "ambient_color_b");

	scene->AddTerrain(terrain->GetTerrain());
	scene->SetAmbientLight(COL(ambient_r, ambient_g, ambient_b));

	// and visibility...
	float visRange = cameraRange;
	if (visRange > max_camera_range && max_camera_range > 0) 
		visRange = max_camera_range;
	scene->GetCamera()->SetVisibilityRange((float)visRange);

	terrain->GetTerrain()->SetSunlight(Vector(sun_x,sun_y,sun_z), Color(sun_r, sun_g, sun_b));
	terrain->GetTerrain()->GenerateTexturing();

	std::string model_map_name = Parser::GetString(scene_properties, "model_properties");
	Parser::Parser model_map(model_map_name.c_str());
	std::string model_map_string = Parser::GetString(scene_properties, "model_map");
	/ psd /
	//terrain->CreateEnvironment(model_map, model_map_string.c_str(), Parser::GetFloat(scene_properties, "model_scale"));
	terrain->AddPaint(scene_config.FindGroup("Terrain"));

	std::string sky_string = Parser::GetString(scene_config.FindGroup("Scene").GetProperties(), "sky_model");
	char *sky_fname = const_cast<char *> (sky_string.c_str());
	IStorm3D_Model *sky_model = storm3d->CreateNewModel();
	sky_model->LoadS3D(sky_fname);
	//ChangeToUncompressed(storm3d, sky_model);
	scene->SetBackGround(sky_model);

	gameMap->setData(terrain->GetHeightMap(), terrain->GetTerrain()->GetHeightmap(), terrain->getHeightMapSize(), 
	  terrain->getTerrainSize(), terrain->getTerrainHeight(), model_map_string.c_str());
	gameMap->setTerrain(terrain->GetTerrain());

	terrain->GetTerrain()->SetObstacleHeightmap(gameMap->getObstacleHeightMap());
	return terrain;
}  
*/

} // ui
