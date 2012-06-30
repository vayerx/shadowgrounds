#include "precompiled.h"

#include "BurningMap.h"

#include "Game.h"
#include "GameUI.h"
#include "GameMap.h"

#include "tracking/ScriptableTrackerObjectType.h"
#include "tracking/ScriptableTrackerObject.h"
#include "tracking/ObjectTracker.h"
#include "tracking/ITrackableObject.h"
#include "tracking/ITrackerObject.h"
#include "tracking/ITrackableObjectIterator.h"
#include "tracking/TrackableUnifiedHandleObject.h"
#include "UnifiedHandleManager.h"
#include "unified_handle.h"

//#include <Storm3D_UI.h>
//extern IStorm3D_Scene *disposable_scene;

namespace game
{
    extern std::vector<tracking::ScriptableTrackerObjectType *> gs_scriptableTrackers;
}

using namespace game;

BurningMap::BurningMap(int pathfind_sizeX, int pathfind_sizeY) :
    sizeX(pathfind_sizeX / pathfindSizeDiv), sizeY(pathfind_sizeY / pathfindSizeDiv)
{
    map = new unsigned char[sizeX * sizeY];
    memset(map, 0, sizeX * sizeY);
}

BurningMap::~BurningMap()
{
    delete map;
}

__forceinline void clampCoord(int &c, const int &max)
{
    if (c >= max) c = max - 1;
    else if (c < 0) c = 0;
}

void BurningMap::update(game::Game *game)
{
    memset(map, 0, sizeX * sizeY);

    // get all scripted trackers
    for (unsigned int i = 0; i < game::gs_scriptableTrackers.size(); i++) {
        tracking::ScriptableTrackerObjectType *type = gs_scriptableTrackers[i];
        if (type == NULL)
            continue;

        std::vector<tracking::ITrackerObject *> objects = game->objectTracker->getAllTrackersOfType(type);
        for (unsigned int i = 0; i < objects.size(); i++) {
            if (objects[i]->getType() == NULL || objects[i]->getType()->getTypeId() !=
                tracking::ScriptableTrackerObjectType::typeId)
                continue;

            tracking::ScriptableTrackerObject *object = (tracking::ScriptableTrackerObject *)objects[i];

            int burning_varnum = object->getVariableNumberByName("ignited");
            if (burning_varnum == -1)
                continue;

            int heatrad_varnum = object->getVariableNumberByName("heatradius");
            if (heatrad_varnum == -1)
                continue;

            int burntime_varnum = object->getVariableNumberByName("burntime");
            if (burntime_varnum == -1)
                continue;

            int heatrad = object->getVariable(heatrad_varnum);

            // burning
            if (object->getVariable(burning_varnum) == 1 && object->getVariable(burntime_varnum) > 0) {
                VC3 pos = object->getTrackerPosition();
                float radius = heatrad * 0.01f;

                /*disposable_scene->AddLine(pos + VC3(0, 1, 0), pos + VC3(-radius, 1, 0), COL(1,0.5f,0));
                   disposable_scene->AddLine(pos + VC3(0, 1, 0), pos + VC3(radius, 1, 0), COL(1,0.5f,0));
                   disposable_scene->AddLine(pos + VC3(0, 1, 0), pos + VC3(0, 1, -radius), COL(1,0.5f,0));
                   disposable_scene->AddLine(pos + VC3(0, 1, 0), pos + VC3(0, 1, radius), COL(1,0.5f,0));*/

                int start_x = game->gameMap->scaledToPathfindX(pos.x - radius) / pathfindSizeDiv;
                int start_y = game->gameMap->scaledToPathfindY(pos.z - radius) / pathfindSizeDiv;
                int end_x = game->gameMap->scaledToPathfindX(pos.x + radius) / pathfindSizeDiv + 1;
                int end_y = game->gameMap->scaledToPathfindY(pos.z + radius) / pathfindSizeDiv + 1;

                clampCoord(start_x, sizeX);
                clampCoord(start_y, sizeY);
                clampCoord(end_x, sizeX);
                clampCoord(end_y, sizeY);

                //int center_x = (start_x + end_x)/2;
                //int center_y = (start_y + end_y)/2;
                //int half_size_x = center_x - start_x;
                //int half_size_y = center_y - start_y;
                //int size = half_size_x * half_size_x + half_size_y * half_size_y + 1;

                for (int y = start_y; y <= end_y; y++) {
                    for (int x = start_x; x <= end_x; x++) {
                        map[y * sizeX + x] = 255;
                    }
                }
            }
        }
    }

    /*FILE *file = fopen("dump.txt", "wt");
       for(int y = 0; y < sizeY; y++)
       {
        for(int x = 0; x < sizeX; x++)
        {
            fprintf(file, "%03i,", map[y * sizeX + x]);
        }
        fputs("\n", file);
       }
       fclose(file);*/
}

int BurningMap::getCost(int pathfind_i, int pathfind_j)
{
    int i = pathfind_i / pathfindSizeDiv;
    int j = pathfind_j / pathfindSizeDiv;

    clampCoord(i, sizeX);
    clampCoord(j, sizeY);

    return map[j * sizeX + i];
}
