
#include "precompiled.h"

#include "DebugDataView.h"

#include <Storm3D_UI.h>

#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../game/GameScene.h"
#include "../game/UnitList.h"
#include "../game/Unit.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_debug.h"
#include "../game/CoverMap.h"
#include "../util/LightAmountManager.h"
#include "../util/LightMap.h"
#include "../util/AreaMap.h"
#include "../util/GridOcclusionCuller.h"
#include "../game/areamasks.h"
#include "../ui/GameController.h"
#include "../system/Timer.h"

#define DEBUG_DATA_VIEW_SIZE_X 512
#define DEBUG_DATA_VIEW_SIZE_Y 512


// HACK:
extern ListNode *linkedListNodePool;
extern int nextLinkedListPoolNode;
extern int linkedListNodePoolAlloced;

//extern int disposable_frametime_msec;
extern int disposable_frames_since_last_clear;
extern int disposable_ticks_since_last_clear;

extern int pathfind_findroutes_since_last_clear;
extern int pathfind_findroutes_failed_since_last_clear;

extern int game_actedAmount;
extern int game_quickNoActAmount;
extern int game_slowNoActAmount;

extern int gamescene_raytraces_since_last_clear;
extern int gamescene_lostraces_since_last_clear;


#ifdef PATHFIND_DEBUG
  namespace frozenbyte
	{
		namespace ai
		{
			extern unsigned char *pathfind_debug_data;
		}
	}
#endif
#ifdef LINEOFJUMP_DEBUG
	extern unsigned char *lineofjump_debug_data;
#endif


namespace ui
{

	DebugDataView *DebugDataView::instance = NULL;


	class DebugDataViewImpl
	{
		private:
			DebugDataViewImpl(game::Game *game)
			{
				this->game = game;
				this->debugDataTexture = NULL;
				this->debugDataMaterial = NULL;
				this->debugData = NULL;
				this->lastUpdateTime = Timer::getTime();
				this->lastPerfUpdateTime = Timer::getTime();
				this->specialLastsUntil = Timer::getTime();

				for (int i = 0; i < DEBUG_DATA_VIEW_SIZE_X; i++)
				{
					frameTimeHistory[i] = 0;
					sampleTimeHistory[i] = 0;
					pathfindHistory[i] = 0;
					pathfindFailedHistory[i] = 0;
					actHistory[i] = 0;
					slowNoActHistory[i] = 0;
					quickNoActHistory[i] = 0;
					unitAmountHistory[i] = 0;
					raytraceHistory[i] = 0;
					lostraceHistory[i] = 0;
				}
				currentHistory = 0;

				currentOcclusionArea = GRIDOCCLUSIONCULLER_DEFAULT_AREA_ORDER_NUMBER;
				currentOcclusionPosX = 0;
				currentOcclusionPosY = 0;
				currentOcclusionPosSelectStartX = 0;
				currentOcclusionPosSelectStartY = 0;

			}

			~DebugDataViewImpl()
			{
				this->game = NULL;
			}

			game::Game *game;
			IStorm3D_Texture *debugDataTexture;
			IStorm3D_Material *debugDataMaterial;
			unsigned char *debugData;
			int lastUpdateTime;
			int specialLastsUntil;
			int lastPerfUpdateTime;

			int frameTimeHistory[DEBUG_DATA_VIEW_SIZE_X];
			int sampleTimeHistory[DEBUG_DATA_VIEW_SIZE_X];
			int tickAmountHistory[DEBUG_DATA_VIEW_SIZE_X];
			int pathfindHistory[DEBUG_DATA_VIEW_SIZE_X];
			int pathfindFailedHistory[DEBUG_DATA_VIEW_SIZE_X];
			int actHistory[DEBUG_DATA_VIEW_SIZE_X];
			int slowNoActHistory[DEBUG_DATA_VIEW_SIZE_X];
			int quickNoActHistory[DEBUG_DATA_VIEW_SIZE_X];
			int unitAmountHistory[DEBUG_DATA_VIEW_SIZE_X];
			int raytraceHistory[DEBUG_DATA_VIEW_SIZE_X];
			int lostraceHistory[DEBUG_DATA_VIEW_SIZE_X];
			int currentHistory;

			int currentOcclusionArea;
			int currentOcclusionPosX;
			int currentOcclusionPosY;
			int currentOcclusionPosSelectStartX;
			int currentOcclusionPosSelectStartY;

		friend class DebugDataView;
	};


	DebugDataView::DebugDataView(game::Game *game)
	{
		this->impl = new DebugDataViewImpl(game);

	}


	DebugDataView::~DebugDataView()
	{
		cleanup();
		delete impl;
	}


	void DebugDataView::cleanup()
	{
		if (impl->debugDataTexture != NULL)
		{
			delete impl->debugDataMaterial;
			impl->debugDataMaterial = NULL;

			impl->debugDataTexture->Release();
			impl->debugDataTexture = NULL;

			delete [] impl->debugData;
			impl->debugData = NULL;
		}
	}


	void DebugDataView::run()
	{
		int curTime = Timer::getTime();

		if (impl->game->getGameScene() == NULL)
		{
			return;
		}
		IStorm3D *s3d = impl->game->getGameScene()->getStorm3D();
		IStorm3D_Scene *scene = impl->game->getGameScene()->getStormScene();

		if (game::SimpleOptions::getBool(DH_OPT_B_DEBUG_DATA_VIEW_PERF_STATS))
		{
      if (curTime >= impl->lastPerfUpdateTime + game::SimpleOptions::getInt(DH_OPT_I_DEBUG_DATA_VIEW_PERF_STATS_RATE))
			{
				int timeElapsed = curTime - impl->lastPerfUpdateTime;
				impl->lastPerfUpdateTime = curTime;

				impl->currentHistory = ((impl->currentHistory + 1) % DEBUG_DATA_VIEW_SIZE_X);

				if (disposable_frames_since_last_clear > 0)
				{
					impl->frameTimeHistory[impl->currentHistory] = timeElapsed / disposable_frames_since_last_clear;
				} else {
					impl->frameTimeHistory[impl->currentHistory] = 0;
				}
				impl->sampleTimeHistory[impl->currentHistory] = timeElapsed;

				impl->tickAmountHistory[impl->currentHistory] = disposable_ticks_since_last_clear;

				impl->pathfindHistory[impl->currentHistory] = pathfind_findroutes_since_last_clear;
				impl->pathfindFailedHistory[impl->currentHistory] = pathfind_findroutes_failed_since_last_clear;

				impl->raytraceHistory[impl->currentHistory] = gamescene_raytraces_since_last_clear;
				impl->lostraceHistory[impl->currentHistory] = gamescene_lostraces_since_last_clear;

				impl->actHistory[impl->currentHistory] = game_actedAmount;
				impl->slowNoActHistory[impl->currentHistory] = game_slowNoActAmount;
				impl->quickNoActHistory[impl->currentHistory] = game_quickNoActAmount;
				impl->unitAmountHistory[impl->currentHistory] = impl->game->units->getAllUnitAmount();

				game_actedAmount = 0;
				game_slowNoActAmount = 0;
				game_quickNoActAmount = 0;

				disposable_frames_since_last_clear = 0;
				disposable_ticks_since_last_clear = 0;

				pathfind_findroutes_since_last_clear = 0;
				pathfind_findroutes_failed_since_last_clear = 0;

				gamescene_raytraces_since_last_clear = 0;
				gamescene_lostraces_since_last_clear = 0;
			}
		}

		if (game::SimpleOptions::getBool(DH_OPT_B_SHOW_DEBUG_DATA_VIEW))
		{
			int viewAlpha = game::SimpleOptions::getInt(DH_OPT_I_DEBUG_DATA_VIEW_ALPHA);

			if (impl->debugDataTexture == NULL)
			{
				impl->debugData = new unsigned char[DEBUG_DATA_VIEW_SIZE_X * DEBUG_DATA_VIEW_SIZE_Y * 4];
				impl->debugDataTexture = s3d->CreateNewTexture(DEBUG_DATA_VIEW_SIZE_X, DEBUG_DATA_VIEW_SIZE_Y, IStorm3D_Texture::TEXTYPE_BASIC);
				impl->debugDataMaterial = s3d->CreateNewMaterial("debugdata");
				impl->debugDataMaterial->SetBaseTexture(impl->debugDataTexture);
				if (viewAlpha < 100)
					impl->debugDataMaterial->SetAlphaType(IStorm3D_Material::ATYPE_ADD);
			}

			bool show_occlusion = false;
			if ((game::SimpleOptions::getInt(DH_OPT_I_DEBUG_DATA_VIEW_TYPE_MASK) & 2048) != 0)
				show_occlusion = true;

			int occminx = 0;
			int occminy = 0;
			int occmaxx = 0;
			int occmaxy = 0;

			if (show_occlusion)
			{
				if (impl->game->gameUI->getGridOcclusionCuller() != NULL)
				{
					int newOccX = impl->currentOcclusionPosX;
					int newOccY = impl->currentOcclusionPosY;

					if (impl->game->gameUI->getController(0)->wasKeyClicked(DH_CTRL_RESERVED_71))
					{
						newOccX = impl->currentOcclusionPosX - 1;
					}
					if (impl->game->gameUI->getController(0)->wasKeyClicked(DH_CTRL_RESERVED_72))
					{
						newOccX = impl->currentOcclusionPosX + 1;
					}
					if (impl->game->gameUI->getController(0)->wasKeyClicked(DH_CTRL_RESERVED_73))
					{
						newOccY = impl->currentOcclusionPosY - 1;
					}
					if (impl->game->gameUI->getController(0)->wasKeyClicked(DH_CTRL_RESERVED_74))
					{
						newOccY = impl->currentOcclusionPosY + 1;
					}
					bool showAreaMessage = false;
					if (impl->game->gameUI->getController(0)->wasKeyClicked(DH_CTRL_SELECT_UNIT_1))
					{
						impl->currentOcclusionArea--;
						if (impl->currentOcclusionArea < 0)
						{
							impl->currentOcclusionArea = 0;
						}
						showAreaMessage = true;
					}
					if (impl->game->gameUI->getController(0)->wasKeyClicked(DH_CTRL_SELECT_UNIT_2))
					{
						impl->currentOcclusionArea++;
						if (impl->currentOcclusionArea >= GRIDOCCLUSIONCULLER_MAX_AREAS)
						{
							impl->currentOcclusionArea = GRIDOCCLUSIONCULLER_MAX_AREAS - 1;
						}
						showAreaMessage = true;
					}
					if (showAreaMessage)
					{
						char buf[128];
						sprintf(buf, "Selected occlusion area %d", impl->currentOcclusionArea);
						impl->game->gameUI->gameMessage(buf, NULL, 1, 2000, game::GameUI::MESSAGE_TYPE_CENTER_BIG);
					}

					occminx = impl->currentOcclusionPosSelectStartX;
					occminy = impl->currentOcclusionPosSelectStartY;
					occmaxx = impl->currentOcclusionPosX;
					occmaxy = impl->currentOcclusionPosY;
					if (occminx > occmaxx)
					{
						int tmp = occmaxx;
						occmaxx = occminx;
						occminx = tmp;
					}
					if (occminy > occmaxy)
					{
						int tmp = occmaxy;
						occmaxy = occminy;
						occminy = tmp;
					}

					GRIDOCCLUSIONCULLER_DATATYPE areaBitMask = impl->game->gameUI->getGridOcclusionCuller()->getAreaForOrderNumber(impl->currentOcclusionArea);
					if (impl->game->gameUI->getController(0)->wasKeyClicked(DH_CTRL_SELECT_UNIT_3))
					{
						for (int ty = occminy; ty < occmaxy + 1; ty++)
						{
							for (int tx = occminx; tx < occmaxx + 1; tx++)
							{
								impl->game->gameUI->getGridOcclusionCuller()->makeVisibleToArea(tx, ty, areaBitMask);
							}
						}
					}
					if (impl->game->gameUI->getController(0)->wasKeyClicked(DH_CTRL_SELECT_UNIT_4))
					{
						if (impl->game->gameUI->getGridOcclusionCuller()->getCameraArea(impl->currentOcclusionPosX, impl->currentOcclusionPosY) != areaBitMask)
						{
							for (int ty = occminy; ty < occmaxy + 1; ty++)
							{
								for (int tx = occminx; tx < occmaxx + 1; tx++)
								{
									impl->game->gameUI->getGridOcclusionCuller()->makeOccludedToArea(tx, ty, areaBitMask);
								}
							}
						}
					}
					if (impl->game->gameUI->getController(0)->wasKeyClicked(DH_CTRL_SELECT_UNIT_5))
					{
						for (int ty = occminy; ty < occmaxy + 1; ty++)
						{
							for (int tx = occminx; tx < occmaxx + 1; tx++)
							{
								impl->game->gameUI->getGridOcclusionCuller()->makeCameraArea(tx, ty, areaBitMask);
								impl->game->gameUI->getGridOcclusionCuller()->makeVisibleToArea(tx, ty, areaBitMask);
							}
						}
					}
					if (impl->game->gameUI->getController(0)->wasKeyClicked(DH_CTRL_SELECT_UNIT_6))
					{
						impl->game->gameUI->getTerrain()->updateOcclusionForAllObjects(impl->game->gameUI->getGridOcclusionCuller(), areaBitMask);
					}
					if (impl->game->gameUI->getGridOcclusionCuller()->isInOcclusionBoundaries(newOccX,newOccY))
					{
						impl->currentOcclusionPosX = newOccX;
						impl->currentOcclusionPosY = newOccY;
						if (impl->game->gameUI->getController(0)->isKeyDown(DH_CTRL_MULTIPLE_UNIT_SELECT))
						{
							// (keep the occ selct start pos still)
						} else {
							impl->currentOcclusionPosSelectStartX = newOccX;
							impl->currentOcclusionPosSelectStartY = newOccY;
						}
					}
				}
			}
			
			if (curTime >= impl->lastUpdateTime + game::SimpleOptions::getInt(DH_OPT_I_DEBUG_DATA_VIEW_RATE))
			{
				impl->lastUpdateTime = curTime;
				bool show_special = false;
				if (curTime < impl->specialLastsUntil)
				{
					show_special = true;
				}
				int px;
				int py;
				if (impl->game->gameUI->getFirstPerson(0) != NULL)
				{
					VC3 pos = impl->game->gameUI->getFirstPerson(0)->getPosition();
					px = impl->game->gameMap->scaledToObstacleX(pos.x);
					py = impl->game->gameMap->scaledToObstacleY(pos.z);
				} else {
					VC3 pos = impl->game->gameUI->getGameCamera()->getPosition();
					px = impl->game->gameMap->scaledToObstacleX(pos.x);
					py = impl->game->gameMap->scaledToObstacleY(pos.z);
				}

				if (impl->currentOcclusionPosX == 0
					&& impl->currentOcclusionPosY == 0)
				{
					if (impl->game->gameUI->getGridOcclusionCuller() != NULL)
					{
						float scaledX = impl->game->gameMap->obstacleToScaledX(px);
						float scaledY = impl->game->gameMap->obstacleToScaledY(py);
						int occx = impl->game->gameUI->getGridOcclusionCuller()->scaledToOcclusionX(scaledX);
						int occy = impl->game->gameUI->getGridOcclusionCuller()->scaledToOcclusionY(scaledY);
						if (impl->game->gameUI->getGridOcclusionCuller()->isInOcclusionBoundaries(occx, occy))
						{
							impl->currentOcclusionPosX = occx;
							impl->currentOcclusionPosY = occy;
							impl->currentOcclusionPosSelectStartX = occx;
							impl->currentOcclusionPosSelectStartY = occy;
						}
					}
				}

				if (impl->game->gameUI->getTerrain() != NULL)
				{
					unsigned short *forcebuf = impl->game->gameUI->getTerrain()->GetForceMap();
					unsigned short *heightbuf = impl->game->gameUI->getTerrain()->GetDoubleHeightMap();
					int obstSizeX = impl->game->gameMap->getObstacleSizeX();
					int obstSizeY = impl->game->gameMap->getObstacleSizeY();

					bool show_obstmap = false;
					if ((game::SimpleOptions::getInt(DH_OPT_I_DEBUG_DATA_VIEW_TYPE_MASK) & 1) != 0)
						show_obstmap = true;

					bool show_forcemap = false;
					if ((game::SimpleOptions::getInt(DH_OPT_I_DEBUG_DATA_VIEW_TYPE_MASK) & 2) != 0)
						show_forcemap = true;

					bool show_heightmap = false;
					if ((game::SimpleOptions::getInt(DH_OPT_I_DEBUG_DATA_VIEW_TYPE_MASK) & 4) != 0)
						show_heightmap = true;

					bool show_paths = false;
					if ((game::SimpleOptions::getInt(DH_OPT_I_DEBUG_DATA_VIEW_TYPE_MASK) & 8) != 0)
						show_paths = true;

					bool show_light = false;
					if ((game::SimpleOptions::getInt(DH_OPT_I_DEBUG_DATA_VIEW_TYPE_MASK) & 16) != 0)
						show_light = true;

					bool show_buildingarea = false;
					if ((game::SimpleOptions::getInt(DH_OPT_I_DEBUG_DATA_VIEW_TYPE_MASK) & 32) != 0)
						show_buildingarea = true;

					bool show_pathfind = false;
					if ((game::SimpleOptions::getInt(DH_OPT_I_DEBUG_DATA_VIEW_TYPE_MASK) & 64) != 0)
						show_pathfind = true;

					bool show_covermap = false;
					if ((game::SimpleOptions::getInt(DH_OPT_I_DEBUG_DATA_VIEW_TYPE_MASK) & 128) != 0)
						show_covermap = true;

					bool show_material = false;
					if ((game::SimpleOptions::getInt(DH_OPT_I_DEBUG_DATA_VIEW_TYPE_MASK) & 256) != 0)
						show_material = true;

					bool show_nodepool = false;
					if ((game::SimpleOptions::getInt(DH_OPT_I_DEBUG_DATA_VIEW_TYPE_MASK) & 512) != 0)
						show_nodepool = true;

					bool show_perf = false;
					if ((game::SimpleOptions::getInt(DH_OPT_I_DEBUG_DATA_VIEW_TYPE_MASK) & 1024) != 0)
						show_perf = true;

					util::LightAmountManager *lightman = util::LightAmountManager::getInstance();

					for (int y = 0; y < DEBUG_DATA_VIEW_SIZE_Y; y++)
					{
						for (int x = 0; x < DEBUG_DATA_VIEW_SIZE_X; x++)
						{
							int mapx = px - 256 + x;
							int mapy = py + 256 - y;
							if (mapx < 0) mapx = 0;
							if (mapy < 0) mapy = 0;
							if (mapx >= obstSizeX) mapx = obstSizeX - 1;
							if (mapy >= obstSizeY) mapy = obstSizeY - 1;
							if (show_special)
							{
								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 0] = 0;
								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = 0;
								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = 0;
								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 3] = 255;
							} else {
								if (show_forcemap)
								{
									//impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = 32 + forcebuf[(mapx / 4) + (mapy / 4) * obstSizeX / 4] / 128;
									impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = 32 + forcebuf[(mapx / 2) + (mapy / 2) * obstSizeX / 2] / 128;
								} else {
									impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = 32;
								}
								if (show_buildingarea)
								{
									if (impl->game->gameMap->getAreaMap()->isAreaAnyValue(mapx, mapy, AREAMASK_INBUILDING))
									{
										impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = 255;
									} else {
										impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = 32;
									}
								}

								if (show_heightmap)
								{
									//impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = 32 + heightbuf[(mapx / 4) + (mapy / 4) * obstSizeX / 4] / 128;
									impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = 32 + heightbuf[(mapx / 2) + (mapy / 2) * obstSizeX / 2] / 128;
								} else {
									impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = 32;
								}
								if (show_obstmap)
								{
									int val = impl->game->gameMap->obstacleHeightMap[mapx + mapy * obstSizeX] / 10;
									if (val > 255-32) val = 255-32;
									//impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 0] = 32 + (impl->game->gameMap->obstacleHeightMap[mapx + mapy * obstSizeX] & OBSTACLE_MAP_MASK_HEIGHT) / 8;
									impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 0] = 32 + val;
									/*
									if ((impl->game->gameMap->obstacleHeightMap[mapx + mapy * obstSizeX] & OBSTACLE_MAP_MASK_UNHITTABLE) != 0
										&& (impl->game->gameMap->obstacleHeightMap[mapx + mapy * obstSizeX] & OBSTACLE_MAP_MASK_HEIGHT) != 0)
									{
									*/
									if (impl->game->gameMap->isMovingObstacle(mapx, mapy)
										&& impl->game->gameMap->obstacleHeightMap[mapx + mapy * obstSizeX] != 0)
									{
										if (impl->game->gameMap->isRoundedObstacle(mapx, mapy)
											&& impl->game->gameMap->obstacleHeightMap[mapx + mapy * obstSizeX] != 0)
										{
											//impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = 32 + (impl->game->gameMap->obstacleHeightMap[mapx + mapy * obstSizeX] & OBSTACLE_MAP_MASK_HEIGHT) / 8;
											impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = 32 + val;
										} else {
											//impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = 32 + (impl->game->gameMap->obstacleHeightMap[mapx + mapy * obstSizeX] & OBSTACLE_MAP_MASK_HEIGHT) / 8;
											impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = 32 + val;
										}
									} else {
										/*
										if (impl->game->gameMap->getAreaMap()->isAreaAnyValue(mapx, mapy, AREAMASK_OBSTACLE_BUILDINGWALL)
											&& impl->game->gameMap->obstacleHeightMap[mapx + mapy * obstSizeX] != 0)
										{
											impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = 32 + val / 2;
										}
										*/
										if (impl->game->gameMap->getAreaMap()->isAreaAnyValue(mapx, mapy, AREAMASK_BREAKABLE)
											&& impl->game->gameMap->obstacleHeightMap[mapx + mapy * obstSizeX] != 0)
										{
											impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = 16 + val / 4;
										}

										if (impl->game->gameMap->getAreaMap()->isAreaAnyValue(mapx, mapy, AREAMASK_OBSTACLE_UNHITTABLE)
											&& impl->game->gameMap->obstacleHeightMap[mapx + mapy * obstSizeX] != 0)
										{
											if ((x & 1) != (y & 1))
											{
												impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 0] = 32 + val/2;
											}
										} else {
											if (impl->game->gameMap->getAreaMap()->isAreaAnyValue(mapx, mapy, AREAMASK_OBSTACLE_SEETHROUGH)
												&& impl->game->gameMap->obstacleHeightMap[mapx + mapy * obstSizeX] != 0)
											{
												if ((x & 1) == (y & 1))
												{
													impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 0] = 32 + val/2;
												}
											}
										}
									}
								} else {
									impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 0] = 32;
								}
								if (show_light)
								{
									if ((mapx & 1) == 0 && (mapy & 1) == 0)
									{
										float lAmount;
										VC3 pos;
										pos = VC3(impl->game->gameMap->obstacleToScaledX(mapx), 0, impl->game->gameMap->obstacleToScaledY(mapy));
										pos.y = impl->game->gameMap->getScaledHeightAt(pos.x, pos.z);
										IVisualObjectData *visData = NULL;
										lAmount = lightman->getDynamicLightAmount(pos, visData, 0.5f);

										int col = 32 + int(lAmount * float(255-32));
										if (impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 0] < col)
											impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 0] = col;
										impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = col;
										impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = col;

										if (lAmount < 0.001f)
										{
											unsigned char lAmount;
											lAmount = impl->game->gameMap->lightMap->getLightAmount(mapx, mapy);
											int col = lAmount;
											if (impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 0] < col)
												impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 0] = col;
											impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = col;
											impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = col;
										}
									}
								}
								if (show_pathfind)
								{
#ifdef PATHFIND_DEBUG
									if (frozenbyte::ai::pathfind_debug_data[mapx + mapy * obstSizeX] != 0)
									{
										impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = 255;
									}
#endif
#ifdef LINEOFJUMP_DEBUG
									if (lineofjump_debug_data != NULL)
									{
										if (lineofjump_debug_data[mapx + mapy * obstSizeX] != 0)
										{
											impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = 255;
										}
									}
#endif
								}
								if (show_covermap)
								{									
									int col = 255 - impl->game->gameMap->getCoverMap()->getDistanceToNearestCover(mapx, mapy);
									if (impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 0] < col)
										impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 0] = col;
									impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = col;
									impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = col;
								}
								if (show_material)
								{
									int mat = impl->game->gameMap->getAreaMap()->getAreaValue(mapx, mapy, AREAMASK_MATERIAL) >> AREASHIFT_MATERIAL;
									impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 0] = (mat & (1<<0)) * 255;
									impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = ((mat & (1<<1)) >> 1) * 255;
									impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = ((mat & (1<<2)) >> 2) * 255;
#ifndef PROJECT_SHADOWGROUNDS
									if (((mat & (1<<3)) >> 3) == 0)
									{
										impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 0] /= 2;
										impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] /= 2;
										impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] /= 2;
									}
#endif
								}
								if (show_occlusion)
								{
									if (impl->game->gameUI->getGridOcclusionCuller() != NULL)
									{
										float scaledX = impl->game->gameMap->obstacleToScaledX(mapx);
										float scaledY = impl->game->gameMap->obstacleToScaledY(mapy);
										int occx = impl->game->gameUI->getGridOcclusionCuller()->scaledToOcclusionX(scaledX);
										int occy = impl->game->gameUI->getGridOcclusionCuller()->scaledToOcclusionY(scaledY);
										bool isVis = impl->game->gameUI->getGridOcclusionCuller()->isVisibleToArea(occx, occy, impl->game->gameUI->getGridOcclusionCuller()->getAreaForOrderNumber(impl->currentOcclusionArea));
										GRIDOCCLUSIONCULLER_DATATYPE mapArea = impl->game->gameUI->getGridOcclusionCuller()->getCameraArea(occx, occy);
										bool isArea = false;
										if (mapArea == impl->game->gameUI->getGridOcclusionCuller()->getAreaForOrderNumber(impl->currentOcclusionArea))
											isArea = true;
										if (isArea)
										{
											//impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 0] = 0;
											//impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = 0;
											impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = 192;
										} else {
											if (isVis)
											{
												impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = 192;
											} else {
												//impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = 32;
											}
										}
										// HACK: ...
										// TODO: should _really_ optimize...
										float scaledYUp = impl->game->gameMap->obstacleToScaledY(mapy-1);
										int occyUp = impl->game->gameUI->getGridOcclusionCuller()->scaledToOcclusionY(scaledYUp);
										float scaledXLeft = impl->game->gameMap->obstacleToScaledX(mapx-1);
										int occxLeft = impl->game->gameUI->getGridOcclusionCuller()->scaledToOcclusionX(scaledXLeft);
										float scaledYDown = impl->game->gameMap->obstacleToScaledY(mapy+1);
										int occyDown = impl->game->gameUI->getGridOcclusionCuller()->scaledToOcclusionY(scaledYDown);
										float scaledXRight = impl->game->gameMap->obstacleToScaledX(mapx+1);
										int occxRight = impl->game->gameUI->getGridOcclusionCuller()->scaledToOcclusionX(scaledXRight);
										if (occxLeft != occx || occyUp != occy
											|| occxRight != occx || occyDown != occy)
										{
											//if (impl->currentOcclusionPosX == occx && impl->currentOcclusionPosY == occy)
											if (occx >= occminx && occy >= occminy
												&& occx <= occmaxx && occy <= occmaxy)
											{
												impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 0] = 255;
												impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = 255;
												impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = 255;
											} else {
												impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = 64;
												GRIDOCCLUSIONCULLER_DATATYPE selarea = impl->game->gameUI->getGridOcclusionCuller()->getCameraArea(occx, occy);
												bool borderArea = false;
												if (impl->game->gameUI->getGridOcclusionCuller()->isInOcclusionBoundaries(occx, occyUp)
													&& selarea != impl->game->gameUI->getGridOcclusionCuller()->getCameraArea(occx, occyUp))
													borderArea = true;
												if (impl->game->gameUI->getGridOcclusionCuller()->isInOcclusionBoundaries(occx, occyDown)
													&& selarea != impl->game->gameUI->getGridOcclusionCuller()->getCameraArea(occx, occyDown))
													borderArea = true;
												if (impl->game->gameUI->getGridOcclusionCuller()->isInOcclusionBoundaries(occxLeft, occy)
													&& selarea != impl->game->gameUI->getGridOcclusionCuller()->getCameraArea(occxLeft, occy))
													borderArea = true;
												if (impl->game->gameUI->getGridOcclusionCuller()->isInOcclusionBoundaries(occxRight, occy)
													&& selarea != impl->game->gameUI->getGridOcclusionCuller()->getCameraArea(occxRight, occy))
													borderArea = true;
												if (borderArea)
													impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = 255;
											}
										}
									}
								}
								if (show_nodepool)
								{									
									int nodeIndex = (x & 511) + (y * 512);
									if (x < 512 && y < 512
										&& nodeIndex >= 0 && nodeIndex < linkedListNodePoolAlloced)
									{
										if (nodeIndex == nextLinkedListPoolNode)
										{
											impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 0] = 255;
											impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = 0;
											impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = 0;
										} else {
											if (linkedListNodePool[nodeIndex].item != linkedListNodePool)
											{
												impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 0] = 0;
												impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = 0;
												impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = 255;
											} else {
												impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 0] = 0;
												impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = 0;
												impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = 32;
											}
										}
									} else {
										impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 0] = 0;
										impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = 0;
										impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = 0;
									}
								}
								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 3] = (unsigned char)((int)(255 * viewAlpha) / 100);	
							}
						}
					}

					if (show_paths)
					{
						LinkedList *ulist = impl->game->units->getAllUnits();
						LinkedListIterator uiter(ulist);
						while (uiter.iterateAvailable())
						{
							game::Unit *u = (game::Unit *)uiter.iterateNext();
							if (u->isActive())
							{
								if (!u->isDestroyed())
								{
									if (!u->isAtPathEnd())
									{
										frozenbyte::ai::Path *path = u->getPath();
										for (int i = u->getPathIndex(); i < path->getSize(); i += 1)
										{
											int pathx = path->getPointX(i);
											int pathy = path->getPointY(i);
											int x = DEBUG_DATA_VIEW_SIZE_X/2 + (pathx - px);
											int y = DEBUG_DATA_VIEW_SIZE_Y/2 - (pathy - py);
											if (x > 0 && y > 0
												&& x < DEBUG_DATA_VIEW_SIZE_X && y < DEBUG_DATA_VIEW_SIZE_Y)
											{
												impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = 255;
											}
										}
									}
								}
								VC3 upos = u->getPosition();
								int unitx = impl->game->gameMap->scaledToObstacleX(upos.x);
								int unity = impl->game->gameMap->scaledToObstacleX(upos.z);
								//int x = 256 + (unitx - px);
								//int y = 256 - (unity - py);
								int x = -px + DEBUG_DATA_VIEW_SIZE_X/2 + unitx;
								int y = DEBUG_DATA_VIEW_SIZE_Y - (-py - DEBUG_DATA_VIEW_SIZE_Y/2 + unity);
								if (x > 1 && y > 1
									&& x < DEBUG_DATA_VIEW_SIZE_X - 1 && y < DEBUG_DATA_VIEW_SIZE_Y - 1)
								{
									int val = 255;
									if (u->isDestroyed())
										val = 128;
									impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = val;
									impl->debugData[((x + 1) + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = val;
									impl->debugData[((x - 1) + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = val;
									impl->debugData[(x + (y + 1) * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = val;
									impl->debugData[(x + (y - 1) * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = val;
								}
							}
						}
					}
					if (show_perf)
					{
						int timeMult = 10;
						if (game::SimpleOptions::getInt(DH_OPT_I_DEBUG_DATA_VIEW_PERF_STATS_RATE) >= 100)
						{
							timeMult /= (game::SimpleOptions::getInt(DH_OPT_I_DEBUG_DATA_VIEW_PERF_STATS_RATE) / 100);
							if (timeMult < 1) timeMult = 1;
						}

						for (int x = 0; x < DEBUG_DATA_VIEW_SIZE_X; x++)
						{
							int y;

							// frame time avg
							{
								y = 0;
								int iterPos = x;
								int totalTime = 0;
								int divSum = 0;
								while (totalTime < 1000)
								{
									y += impl->frameTimeHistory[iterPos];
									totalTime += impl->sampleTimeHistory[iterPos];
									iterPos = (iterPos + DEBUG_DATA_VIEW_SIZE_X - 1) % DEBUG_DATA_VIEW_SIZE_X;
									divSum++;
									if (iterPos == x || iterPos == impl->currentHistory)
									{
										y = 0;
										divSum = 1;
										break;
									}
								}
								if (divSum > 0)
									y /= divSum;
								y *= 2;
								y = DEBUG_DATA_VIEW_SIZE_Y * 2 / 3 - 10 - y;
								if (y < 0) y = 0;
								if (y >= DEBUG_DATA_VIEW_SIZE_Y) y = DEBUG_DATA_VIEW_SIZE_Y-1;

								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 0] = 0;
								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = 128;
								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = 0;
							}

							// frame time
							{
								y = impl->frameTimeHistory[x] * 2;
								y = DEBUG_DATA_VIEW_SIZE_Y * 2 / 3 - 10 - y;
								if (y < 0) y = 0;
								if (y >= DEBUG_DATA_VIEW_SIZE_Y) y = DEBUG_DATA_VIEW_SIZE_Y-1;

								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 0] = 0;
								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = 255;
								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = 0;
							}

							// total units
							{
								int ticks = impl->tickAmountHistory[x];
								if (ticks < 1) ticks = 1;
								y = impl->unitAmountHistory[x] / 2;
								y = DEBUG_DATA_VIEW_SIZE_Y * 2 / 3 - 10 - y + 2;
								if (y < 0) y = 0;
								if (y >= DEBUG_DATA_VIEW_SIZE_Y) y = DEBUG_DATA_VIEW_SIZE_Y-1;

								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 0] = 128;
								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = 128;
								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = 0;
							}

							// quick no act
							{
								int ticks = impl->tickAmountHistory[x];
								if (ticks < 1) ticks = 1;
								y = impl->quickNoActHistory[x] / ticks * timeMult;
								y = DEBUG_DATA_VIEW_SIZE_Y * 2 / 3 - 10 - y + 2;
								if (y < 0) y = 0;
								if (y >= DEBUG_DATA_VIEW_SIZE_Y) y = DEBUG_DATA_VIEW_SIZE_Y-1;

								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 0] = 255;
								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = 255;
								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = 0;
							}

							// slow no act
							{
								int ticks = impl->tickAmountHistory[x];
								if (ticks < 1) ticks = 1;
								y = impl->slowNoActHistory[x] / ticks * timeMult;
								y = DEBUG_DATA_VIEW_SIZE_Y * 2 / 3 - 10 - y + 2;
								if (y < 0) y = 0;
								if (y >= DEBUG_DATA_VIEW_SIZE_Y) y = DEBUG_DATA_VIEW_SIZE_Y-1;

								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 0] = 128;
								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = 0;
								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = 0;
							}

							// act
							{
								int ticks = impl->tickAmountHistory[x];
								if (ticks < 1) ticks = 1;
								y = impl->actHistory[x] / ticks * timeMult;
								y = DEBUG_DATA_VIEW_SIZE_Y * 2 / 3 - 10 - y + 2;
								if (y < 0) y = 0;
								if (y >= DEBUG_DATA_VIEW_SIZE_Y) y = DEBUG_DATA_VIEW_SIZE_Y-1;

								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 0] = 255;
								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = 0;
								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = 0;
							}

							// lostraces
							{
								y = impl->lostraceHistory[x] * timeMult;
								y = DEBUG_DATA_VIEW_SIZE_Y * 2 / 3 - 10 - y + 3;
								if (y < 0) y = 0;
								if (y >= DEBUG_DATA_VIEW_SIZE_Y) y = DEBUG_DATA_VIEW_SIZE_Y-1;

								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 0] = 0;
								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = 128;
								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = 128;
							}

							// raytraces
							{
								y = impl->raytraceHistory[x] * timeMult;
								y = DEBUG_DATA_VIEW_SIZE_Y * 2 / 3 - 10 - y + 3;
								if (y < 0) y = 0;
								if (y >= DEBUG_DATA_VIEW_SIZE_Y) y = DEBUG_DATA_VIEW_SIZE_Y-1;

								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 0] = 0;
								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = 255;
								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = 255;
							}

							// pathfinds
							{
								y = impl->pathfindHistory[x] * timeMult;
								y = DEBUG_DATA_VIEW_SIZE_Y * 2 / 3 - 10 - y;
								if (y < 0) y = 0;
								if (y >= DEBUG_DATA_VIEW_SIZE_Y) y = DEBUG_DATA_VIEW_SIZE_Y-1;

								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 0] = 0;
								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = 0;
								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = 128;
							}

							// pathfinds failed
							{
								y = impl->pathfindFailedHistory[x] * timeMult;
								y = DEBUG_DATA_VIEW_SIZE_Y * 2 / 3 - 10 - y;
								if (y < 0) y = 0;
								if (y >= DEBUG_DATA_VIEW_SIZE_Y) y = DEBUG_DATA_VIEW_SIZE_Y-1;

								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 0] = 0;
								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = 0;
								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = 255;
							}

							// ticks
							{
								y = impl->tickAmountHistory[x] * timeMult;
								y = DEBUG_DATA_VIEW_SIZE_Y * 2 / 3 - 10 - y + 1;
								if (y < 0) y = 0;
								if (y >= DEBUG_DATA_VIEW_SIZE_Y) y = DEBUG_DATA_VIEW_SIZE_Y-1;

								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 0] = 255;
								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = 255;
								impl->debugData[(x + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = 255;
							}
						}
						for (int y = 0; y < DEBUG_DATA_VIEW_SIZE_X; y++)
						{
							impl->debugData[(impl->currentHistory + y * DEBUG_DATA_VIEW_SIZE_X) * 4 + 3] = 192;
						}
					}

				}

				//if (px >= 0 && py >= 0 && px < DEBUG_DATA_VIEW_SIZE_X && py < DEBUG_DATA_VIEW_SIZE_Y)
				//{				
				impl->debugData[(DEBUG_DATA_VIEW_SIZE_X / 2 + (DEBUG_DATA_VIEW_SIZE_Y / 2) * DEBUG_DATA_VIEW_SIZE_X) * 4 + 0] = 32;
				impl->debugData[(DEBUG_DATA_VIEW_SIZE_X / 2 + (DEBUG_DATA_VIEW_SIZE_Y / 2) * DEBUG_DATA_VIEW_SIZE_X) * 4 + 1] = 32;
				impl->debugData[(DEBUG_DATA_VIEW_SIZE_X / 2 + (DEBUG_DATA_VIEW_SIZE_Y / 2) * DEBUG_DATA_VIEW_SIZE_X) * 4 + 2] = 255;
				impl->debugData[(DEBUG_DATA_VIEW_SIZE_X / 2 + (DEBUG_DATA_VIEW_SIZE_Y / 2) * DEBUG_DATA_VIEW_SIZE_X) * 4 + 3] = 255;
				//}

				impl->debugDataTexture->Copy32BitSysMembufferToTexture((DWORD *)impl->debugData);
			}
			scene->Render2D_Picture(impl->debugDataMaterial, VC2(0,0), VC2(DEBUG_DATA_VIEW_SIZE_X*2, DEBUG_DATA_VIEW_SIZE_Y*2));
		} else {
			cleanup();
		}
	}


	DebugDataView *DebugDataView::getInstance(game::Game *game)
	{
		if (instance == NULL)
		{
			instance = new DebugDataView(game);
		}
		return instance;
	}


	void DebugDataView::cleanInstance()
	{
		if (instance != NULL)
		{
			delete instance;
			instance = NULL;
		}
	}

}


