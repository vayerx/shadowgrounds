#include <windows.h>
#include <stdio.h>

// Waypoint
struct WPoint {
	int x,y;
};

extern uint16_t *HeightMapDebug;

class Router {

private:

	// Heightmap info
	uint16_t *HeightMap;
	uint16_t *MapCost;
	int Width,Height;

	// Temporary variables
	WPoint NodePos[4096];
	WPoint NodePos2[4096];

public:

	Router();
	~Router();

	int FindPath(int sx,int sy,int ex,int ey,int MaxHeight,
		WPoint *PArray);

	void SetHeightMap(uint16_t *Map,int Width,int Height);
};
