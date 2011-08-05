
#include "precompiled.h"

#include "Route.H"
#include "..\util\Debug_MemoryManager.h"

static int DirX[8]={1,0,-1,0,1,-1,-1,1};
static int DirY[8]={0,-1,0,1,-1,-1,1,1};

WORD *HeightMapDebug;

//******************************************************************
// Sets heightmap
//******************************************************************

void Router::SetHeightMap(WORD *Map,int Width,int Height) 
{
	if (MapCost) delete MapCost;
	MapCost=0;

	this->Width=Width;
	this->Height=Height;
	this->HeightMap=Map;

	MapCost=new WORD[Width*Height];
}

Router::Router()
{
	HeightMap=0;
	MapCost=0;
}

Router::~Router()
{
	if (MapCost) delete MapCost;
}

//******************************************************************
// Finds a route from point A to point B crossing terrain
// where height diffirence is below MaxHeight variable
// 
// Returns number of routepoints and the route into PArray pointer
// Note: route is written backwards into PArray
//******************************************************************

int Router::FindPath(int sx,int sy,int ex,int ey,int MaxHeight,WPoint *PArray)
{
	int i,i2;

	memset(MapCost,0,Width*Height*2);

	int NNodes;
	WPoint *NodePoint;
	WPoint *NodePoint2;

	NNodes=1;
	NodePos[0].x=sx;
	NodePos[0].y=sy;
	NodePoint=NodePos;
	NodePoint2=NodePos2;

	MapCost[Width*(sy)+(sx)]=1;

	// Write mapcosts into array until we have reached destination

	while (1) {

		i2=NNodes;
		NNodes=0;

		for (i=0;i<i2;i++) {
			int PP2=Width*(NodePoint[i].y)+(NodePoint[i].x);
			for (int Dir=0;Dir<8;Dir++) {
				int PX=NodePoint[i].x+DirX[Dir];
				int PY=NodePoint[i].y+DirY[Dir];
				int PP=Width*(PY)+(PX);
				if ((PX>=0)&&(PY>=0)&&(PX<Width)&&(PY<Height))
				if (abs(HeightMap[PP]-HeightMap[PP2])<MaxHeight)
				if (!MapCost[PP]) {
					NodePoint2[NNodes].x=PX;
					NodePoint2[NNodes++].y=PY;
					MapCost[PP]=MapCost[PP2]+1;
				}

//				HeightMapDebug[Width*(NodePoint[i].y)+(NodePoint[i].x)]=MapCost[Width*(NodePoint[i].y)+(NodePoint[i].x)]*10;

				if (PX==ex)
				if (PY==ey) {
					goto Skip; // ;)
				}
			}
		}

		WPoint *Temp=NodePoint;
		NodePoint=NodePoint2;
		NodePoint2=Temp;

	}

Skip:;

	WPoint Position;

	Position.x=ex;
	Position.y=ey;

	int NPoints=0;

	// Return backwards selecting the cheapest direction from array

	while (1) {

		int SDir;
		int Small=65536;

		for (int Dir=0;Dir<8;Dir++) {
			int PX=Position.x+DirX[Dir];
			int PY=Position.y+DirY[Dir];
			int PP=Width*(PY)+(PX);
			if ((PX>=0)&&
			    (PY>=0)&&
			    (PX<Width)&&
			    (PY<Height))
			if (MapCost[PP]) {
				if (MapCost[PP]<Small) {
					Small=MapCost[PP];
					SDir=Dir;
				}
			}
		}

		PArray[NPoints].x=Position.x;
		PArray[NPoints++].y=Position.y;

		Position.x+=DirX[SDir];
		Position.y+=DirY[SDir];

//		HeightMapDebug[Width*(Position.y)+(Position.x)]=65535;

		if (Position.x==sx)
		if (Position.y==sy) break;
	
	}

	return NPoints;
}
