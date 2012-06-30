#ifndef BURNINGMAP_H
#define BURNINGMAP_H

namespace game
{
    class Game;
}

class BurningMap {
public:
    BurningMap(int pathfind_sizeX, int pathfind_sizeY);
    ~BurningMap();

    void update(game::Game *game);
    int getCost(int pathfind_i, int pathfind_j);

    static const int pathfindSizeDiv = 8;

private:
    unsigned char *map;
    int sizeX;
    int sizeY;
};

#endif
