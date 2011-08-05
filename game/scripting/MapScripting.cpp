
#include "precompiled.h"

#include "gui_configuration.h"

#include "MapScripting.h"

#include "scripting_macros_start.h"
#include "map_script_commands.h"
#include "scripting_macros_end.h"

#include <sstream>
#include <math.h>
#include "../Game.h"
#include "../GameMap.h"
#include "../GameUI.h"
#include "../GameStats.h"
#include "GameScriptingUtils.h"
#include "GameScriptData.h"
#include "../GameRandom.h"
#include "../../ui/Map.h"
#ifdef GUI_BUILD_MAP_WINDOW
#include "../../ui/MapWindow.h"
#endif
#include "../../util/ScriptProcess.h"
#include "../SimpleOptions.h"
#include "../options/options_graphics.h"
#include "../../util/Script.h"
#include "../../filesystem/output_file_stream.h"
#include "../GameScene.h"

#include <IStorm3D.h>
//#include "../../storm/include/IStorm3d_TerrainRenderer.h"
#include "../OptionApplier.h"
#include "GameScripting.h"

#include "../../util/Debug_MemoryManager.h"

using namespace ui;

namespace game
{
	std::string maps_layerid("");
	std::string maps_objective_layerid("");
	float maps_objective_range = -1.0f;
	VC3 maps_objective_position = VC3(0,0,0);
	std::string maps_objective_layer("");

	std::string maps_saved_layerid("");

	std::string maps_portal_from_layer("");
	std::string maps_portal_to_layer("");
	VC3 maps_portal_origin = VC3(0,0,0);
	VC3 maps_portal_destination = VC3(0,0,0);

	struct Portal
	{
		std::string fromLayer;
		std::string toLayer;
		VC3 fromPosition;
		VC3 toPosition;

		Portal(const std::string &fromLayer_, const std::string &toLayer_, const VC3 &fromPosition_, const VC3 &toPosition_)
		:	fromLayer(fromLayer_),
			toLayer(toLayer_),
			fromPosition(fromPosition_),
			toPosition(toPosition_)
		{
		}
	};

	typedef std::vector<Portal> PortalList;
	PortalList portalCache;

	void MapScripting::process(util::ScriptProcess *sp, 
		int command, floatint intFloat, char *stringData, ScriptLastValueType *lastValue,
		GameScriptData *gsd, Game *game)
	{
		switch(command)
		{
			case GS_CMD_SETMAPLAYERID:
				if (stringData != NULL)
				{
					maps_layerid = stringData;
				} else {
					sp->error("MapScripting::process - setMapLayerId parameter missing.");
				}
				break;

			case GS_CMD_SETMAPLAYERSTARTTOPOSITION:
				{
					if (!maps_layerid.empty())
					{
						VC2 mapvec = VC2(gsd->position.x, gsd->position.z);
						game->map->startLayer(maps_layerid, mapvec);
					} else {
						sp->warning("MapScripting::process - setMapLayerStart, no map layer id set.");
					}
				}
				break;

			case GS_CMD_SETMAPLAYERENDTOPOSITION:
				{
					if (!maps_layerid.empty())
					{
						VC2 mapvec = VC2(gsd->position.x, gsd->position.z);
						game->map->endLayer(maps_layerid, mapvec);
					} else {
						sp->warning("MapScripting::process - setMapLayerEnd, no map layer id set.");
					}
				}
				break;

			case GS_CMD_SETMAPLAYERACTIVE:
				if (stringData != NULL)
				{
					std::string stdstr = stringData;
#ifdef GUI_BUILD_MAP_WINDOW
					game->gameUI->getMapWindow()->setActiveLayer(stdstr);
#endif

					for (int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
					{
						if (game->gameUI->getFirstPerson(i) != NULL)
						{
							game::GameStats::instances[i]->addMapChange(stringData);
						}
					}

				} else {
					sp->error("MapScripting::process - setMapLayerActive parameter missing.");
				}
				break;

			case GS_CMD_SETMISSIONOBJECTIVERANGE:
				{
					float floatData = intFloat.f;
					maps_objective_range = floatData;
				}
				break;

			case GS_CMD_SETMISSIONOBJECTIVETOPOSITION:
				maps_objective_position = gsd->position;
				break;

			case GS_CMD_ADDPRIMARYMISSIONOBJECTIVE:
			case GS_CMD_ADDACTIVEMISSIONOBJECTIVE:
			case GS_CMD_ADDSECONDARYMISSIONOBJECTIVE:
				if (stringData != NULL)
				{
					if (maps_objective_position.x == 0
						&& maps_objective_position.y == 0
						&& maps_objective_position.z == 0 && maps_objective_range > 0.0f && maps_objective_range < 1000.0f)
					{
						sp->warning("MapScripting::process - addActive/Primary/SecondaryMissionObjective, objective position was not set?");
					}
#ifndef PROJECT_SURVIVOR
					if (maps_objective_range < 0.0f || maps_objective_range > 1000.0f)
					{
						sp->error("MapScripting::process - addActive/Primary/SecondaryMissionObjective, objective range out of bounds.");
						break;
					}
#endif 

#ifdef GUI_BUILD_MAP_WINDOW
					std::string idstr = stringData;

					if(game->gameUI->getMapWindow()->doesMissionObjectiveExist(idstr))
					{
						sp->error("MapScripting::process - addActive/Primary/SecondaryMissionObjective, objective with that name already exists.");
						break;
					}

					if (command == GS_CMD_ADDACTIVEMISSIONOBJECTIVE)
						game->gameUI->getMapWindow()->addActiveObjective(idstr);
					else if (command == GS_CMD_ADDPRIMARYMISSIONOBJECTIVE)
						game->gameUI->getMapWindow()->addObjective(MapWindow::Primary, idstr);
					else if (command == GS_CMD_ADDSECONDARYMISSIONOBJECTIVE)
						game->gameUI->getMapWindow()->addObjective(MapWindow::Secondary, idstr);

					if (maps_objective_range > 0.0f && maps_objective_range < 1000.0f)
					{
						game->gameUI->getMapWindow()->setObjectivePoint(idstr, maps_objective_position, maps_objective_range);
						game->gameUI->getMapWindow()->setObjectivePointLayer(idstr, maps_objective_layer);
					}
#else
					sp->warning("MapScripting::process - Map window not in build.");
#endif

				} else {
					sp->error("MapScripting::process - addPrimaryMissionObjective parameter missing.");
				}
				maps_objective_range = -1.0f;
				maps_objective_position = VC3(0,0,0);

				break;

			case GS_CMD_REMOVEMISSIONOBJECTIVE:
				if (stringData != NULL)
				{
#ifdef GUI_BUILD_MAP_WINDOW
					std::string idstr = stringData;
					game->gameUI->getMapWindow()->removeObjective(idstr);
#else
					sp->warning("MapScripting::process - Map window not in build.");
#endif
				} else {
					sp->error("MapScripting::process - removePrimaryMissionObjective parameter missing.");
				}
				break;

			case GS_CMD_COMPLETEMISSIONOBJECTIVE:				
				if (stringData != NULL)
				{
#ifdef GUI_BUILD_MAP_WINDOW
					std::string idstr = stringData;
					game->gameUI->getMapWindow()->completeObjective(idstr);
#else
					sp->warning("MapScripting::process - Map window not in build.");
#endif
				} else {
					sp->error("MapScripting::process - completeMissionObjective parameter missing.");
				}
				break;

			case GS_CMD_ISMISSIONOBJECTIVECOMPLETE:
				if (stringData != NULL)
				{
					std::string idstr = stringData;

#ifdef GUI_BUILD_MAP_WINDOW
					if (game->gameUI->getMapWindow()->isMissionObjectiveComplete(idstr))
					{
						*lastValue = 1;
					} else {
						*lastValue = 0;
					}
#else
					sp->warning("MapScripting::process - Map window not in build.");
#endif
				} else {
					sp->error("MapScripting::process - isMissionObjectiveComplete parameter missing.");
				}
				break;

			case GS_CMD_DOESMISSIONOBJECTIVEEXIST:
				if (stringData != NULL)
				{
					std::string idstr = stringData;

#ifdef GUI_BUILD_MAP_WINDOW
					if (game->gameUI->getMapWindow()->doesMissionObjectiveExist(idstr))
					{
						*lastValue = 1;
					} else {
						*lastValue = 0;
					}					
#else
					sp->warning("MapScripting::process - Map window not in build.");
#endif
				} else {
					sp->error("MapScripting::process - doesMissionObjectiveExist parameter missing.");
				}
				break;

			case GS_CMD_UPDATEMISSIONOBJECTIVETOPOSITION:
				if (stringData != NULL)
				{
#ifdef GUI_BUILD_MAP_WINDOW
					std::string idstr = stringData;

					game->gameUI->getMapWindow()->updateMissionObjectiveToPosition(idstr, gsd->position);
#else
					sp->warning("MapScripting::process - Map window not in build.");
#endif
				} else {
					sp->error("MapScripting::process - updateMissionObjectiveToPosition parameter missing.");
				}
				maps_objective_position = gsd->position;
				break;

			case GS_CMD_SETMISSIONOBJECTIVEMAPLAYER:
				if (stringData != NULL)
				{
					maps_objective_layerid = stringData;
				} else {
					sp->error("MapScripting::process - setMissionObjectiveMapLayer parameter missing.");
				}
				break;

			case GS_CMD_SAVEACTIVEMAPLAYER:
#ifdef GUI_BUILD_MAP_WINDOW
				if (game->gameUI->getMapWindow() != NULL)
				{
					maps_saved_layerid = game->gameUI->getMapWindow()->getActiveLayer();
				}
#else
				sp->warning("MapScripting::process - Map window not in build.");
#endif
				break;

			case GS_CMD_RESTORESAVEDMAPLAYER:
#ifdef GUI_BUILD_MAP_WINDOW
				if (game->gameUI->getMapWindow() != NULL)
				{
					std::string stdstr = maps_saved_layerid;
					if (stdstr == "")
					{
						stdstr = std::string("default");
					}
					gsd->setStringValue(stdstr.c_str());
					game->gameUI->getMapWindow()->setActiveLayer(stdstr);
				}
#else
				sp->warning("MapScripting::process - Map window not in build.");
#endif
				break;

			case GS_CMD_GETACTIVEMAPLAYER:
#ifdef GUI_BUILD_MAP_WINDOW
				if (game->gameUI->getMapWindow() != NULL)
				{
					std::string tmp = game->gameUI->getMapWindow()->getActiveLayer();
					gsd->setStringValue(tmp.c_str());
				}
#else
				sp->warning("MapScripting::process - Map window not in build.");
#endif
				break;

			case GS_CMD_SETMISSIONOBJECTIVELAYER:
				if (stringData != NULL)
				{
					maps_objective_layer = stringData;
				} else {
					sp->error("MapScripting::process - setMissionObjectiveLayer parameter missing.");
				}
				break;

			case GS_CMD_SETMAPPORTALFROMLAYER:
				if (stringData != NULL)
				{
					maps_portal_from_layer = stringData;
				} else {
					sp->error("MapScripting::process - setMapPortalFromLayer parameter missing.");
				}
				break;

			case GS_CMD_SETMAPPORTALTOLAYER:
				if (stringData != NULL)
				{
					maps_portal_to_layer = stringData;
				} else {
					sp->error("MapScripting::process - setMapPortalToLayer parameter missing.");
				}
				break;

			case GS_CMD_SETMAPPORTALORIGINTOPOSITION:
				maps_portal_origin = gsd->position;
				break;

			case GS_CMD_SETMAPPORTALDESTINATIONTOPOSITION:
				maps_portal_destination = gsd->position;
				break;

			case GS_CMD_ADDMAPPORTAL:
#ifdef GUI_BUILD_MAP_WINDOW
				if (game->gameUI->getMapWindow() != NULL)
				{
					game->gameUI->getMapWindow()->addPortal(maps_portal_from_layer, maps_portal_to_layer, maps_portal_origin, maps_portal_destination);
				}
				else
					portalCache.push_back(Portal(maps_portal_from_layer, maps_portal_to_layer, maps_portal_origin, maps_portal_destination));
#else
				sp->warning("MapScripting::process - Map window not in build.");
#endif

				break;

			case GS_CMD_REMOVEMAPPORTAL:
#ifdef GUI_BUILD_MAP_WINDOW
				if (game->gameUI->getMapWindow() != NULL)
				{
					game->gameUI->getMapWindow()->removePortal(maps_portal_from_layer, maps_portal_to_layer, maps_portal_origin, maps_portal_destination);
				}
#else
				sp->warning("MapScripting::process - Map window not in build.");
#endif
				break;

			case GS_CMD_clearMapFog:
				if( game && game->map )
				{
					game->map->clearMapFog();
#ifdef GUI_BUILD_MAP_WINDOW
					game->gameUI->getMapWindow()->clearMapFog();
#endif
				}
				break;

			case GS_CMD_renderMapToFile:
				{
					// Renders map to 512x512 textures and assembles a map from those and writes it to disk.

					// Constains quite a dirty hack... Renders the scene first to 512x512 rendertargets, reads and
					// assembles them to system memory, creates one large static texture from it, and finally
					// writes it to the disk using a d3dx -function.

					game->gameScripting->runSingleSimpleStringCommand( "setMapView", "1" );

					int mapMulti = SimpleOptions::getInt( DH_OPT_I_MAPRENDER_SIZE_MULTIPLIER );
					int height = mapMulti * game->getGameScene()->getGameMap()->getSizeX();
					int width  = mapMulti * game->getGameScene()->getGameMap()->getSizeY();

					std::string filename;
					if(stringData)			
					{
						filename = stringData;
					}
					else
					{
						std::stringstream filenamestream;
						filenamestream << "Data/Missions/" << game->getMissionId() << "/bin/map_default.png";
						filenamestream >> filename;
					}

					if( !SimpleOptions::getBool( DH_OPT_B_MAPRENDER_USE_USER_RESOLUTION ) )
					{


						//IStorm3D_TerrainRenderer &r = terrain->getRenderer();
						//r.enableFeature(IStorm3D_TerrainRenderer::SpotShadows, false);
						SimpleOptions::setBool( DH_OPT_B_RENDER_DISTORTION, false );
						OptionApplier::applyDisplayOptions ( game->gameUI->getStorm3D(), game->gameUI->getStormScene(), game->gameUI->getStormScene()->ITTerrain->Begin()->GetCurrent(), NULL );

						int mapBlocksY = height / 512;
						int mapBlocksX = width / 512;

						if( ( height % 512 ) != 0 )
							mapBlocksY ++;
						if( ( width % 512 ) != 0 )
							mapBlocksX ++;

						unsigned char ** mapBlocks = new unsigned char * [ mapBlocksX * mapBlocksY ];
						for(int l = 0; l < mapBlocksX * mapBlocksY; l++)
						{
							mapBlocks[l] = new unsigned char [ 512 * 512 * 4 ];
						}

						int l = 0;
						for(int x = mapBlocksY - 1; x >= 0; x--)
						{
							for(int y = 0; y < mapBlocksX; y++)
							{
								game->gameUI->getGameCamera ()->setForceMapView(true, x, y, height, width, 1.0f );
								game->gameUI->getGameCamera ()->applyToScene ();
								game->gameUI->getStormScene()->GetCamera()->Apply();
								game->gameUI->getStormScene()->RenderScene();
								//std::string dbgfilename("map_debug_");
								//dbgfilename.append( 1, '1' + (char)l);
								//dbgfilename.append(".bmp");
								//game->gameUI->getStorm3D()->TakeScreenshot( dbgfilename.c_str() );
								game->gameUI->getStorm3D()->RenderSceneToArray( game->gameUI->getStormScene(), mapBlocks[l++], 512, 512 );
							}
						}


						/*
						::frozenbyte::filesystem::OutputFileStreamBuffer * fout = new ::frozenbyte::filesystem::OutputFileStreamBuffer ( filename.c_str() );
						fout->putByte( 'F' );
						fout->putByte( 'B' );
						// Store width (little-endian)
						fout->putByte(  width & 0xFF );
						fout->putByte( (width >> 8) & 0xFF );
						// Store height 
						fout->putByte(  height & 0xFF );
						fout->putByte( (height >> 8) & 0xFF );
						*/

						unsigned char * mapData = new unsigned char [ width * height * 4 ];

						int l1 = 0;
						for(int y = 0; y < height; y++)
						{
							int blockY =  y / 512; 
							int pixelY = (y % 512) < 510 ? (y % 512) : 510; // Clamp (y % 512) to 510. Seems to render one pixel wide black border otherwise.
							for(int x = 0; x < width; x++)
							{
								int blockX =  x / 512;
								int pixelX = (x % 512) < 510 ? (x % 512) : 510;
								int blockID = blockY * mapBlocksX + blockX;
								int pixelID = pixelY * 512 * 4 + pixelX * 4 ;
								//fout->putByte( mapBlocks[ blockID ][ pixelID + 0 ] );
								//fout->putByte( mapBlocks[ blockID ][ pixelID + 1 ] );
								//fout->putByte( mapBlocks[ blockID ][ pixelID + 2 ] );
								//fout->putByte( mapBlocks[ blockID ][ pixelID + 3 ] );
								mapData[ l1 ++ ] = mapBlocks[ blockID ][ pixelID + 0 ];
								mapData[ l1 ++ ] = mapBlocks[ blockID ][ pixelID + 1 ];
								mapData[ l1 ++ ] = mapBlocks[ blockID ][ pixelID + 2 ];
								mapData[ l1 ++ ] = mapBlocks[ blockID ][ pixelID + 3 ];
							}
						}
						//delete fout;

						//IStorm3D_Texture * static_texture = game->gameUI->getStorm3D()->CreateNewTexture( "", TEXLOADFLAGS_NOCOMPRESS, 0, mapData, width*height*4 );
						IStorm3D_Texture * static_texture = game->gameUI->getStorm3D()->CreateNewTexture( width, height, IStorm3D_Texture::TEXTYPE_BASIC );
						static_texture->Copy32BitSysMembufferToTexture( (DWORD *)mapData );
						static_texture->saveToFile( filename.c_str() );

						static_texture->Release();
						delete [] mapData;
						for(int l = 0; l < mapBlocksX * mapBlocksY; l++)
						{
							delete [] mapBlocks[l];
						}
						delete [] mapBlocks;

						{
							// tmp...
							char msg[1024];
							sprintf(msg, "Successfully rendered map to file \"%s\" (%dx%d).", filename.c_str(), width, height);
							sp->debug(msg);
							game->gameScripting->runSingleSimpleStringCommand( "hintMessage", msg );
							game->gameScripting->runSingleSimpleStringCommand( "message", msg );
						}
					}
					else
					{
						game->gameUI->getGameCamera ()->setForceMapView(true, 0, 0, -1, -1, 1.0f );
						game->gameUI->getGameCamera ()->applyToScene ();
						game->gameUI->getStormScene()->GetCamera()->Apply();
						game->gameUI->getStormScene()->RenderScene();
						game->gameUI->getStorm3D()->TakeScreenshot( filename.c_str() );
					}

				}
				break;

			default:
				sp->error("MapScripting::process - Unknown command.");				
				assert(0);
		}
	}

	void MapScripting::applyPortals(Game *game)
	{
#ifdef GUI_BUILD_MAP_WINDOW
		if (game->gameUI->getMapWindow() != NULL)
		{
			PortalList::iterator it = portalCache.begin();
			for(; it != portalCache.end(); ++it)
			{
				const Portal &p = *it;
				game->gameUI->getMapWindow()->addPortal(p.fromLayer, p.toLayer, p.fromPosition, p.toPosition);
			}

			portalCache.clear();
		}
#else
		Logger::getInstance()->warning("MapScripting::applyPortals - Map window not in build.");
#endif
	}

}


