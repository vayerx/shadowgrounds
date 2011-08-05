#include <windows.h>
#include <stdio.h>

// Waypoint
struct WPoint {
	int x,y;
};

extern WORD *HeightMapDebug;

class Router {

private:

	// Heightmap info
	WORD *HeightMap;
	WORD *MapCost;
	int Width,Height;

	// Temporary variables
	WPoint NodePos[4096];
	WPoint NodePos2[4096];

public:

	Router();
	~Router();

	int FindPath(int sx,int sy,int ex,int ey,int MaxHeight,
		WPoint *PArray);

	void SetHeightMap(WORD *Map,int Width,int Height);
};
