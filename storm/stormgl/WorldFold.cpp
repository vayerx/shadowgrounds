
#include "WorldFold.h"


#define MAX_WORLD_FOLDS 256
#define CENTER_WORLD_FOLD_INDEX (MAX_WORLD_FOLDS/2)
#define DEFAULT_WORLD_FOLD_STEP 1.0f


float worldFoldStep = DEFAULT_WORLD_FOLD_STEP;

VC3 worldFoldCenter = VC3(0,0,0);

MAT wfCameraMatrix;
VC3 wfCameraPosition = VC3(0,0,0);

MAT worldFoldMatrix[MAX_WORLD_FOLDS];
MAT worldFoldResultMatrix[MAX_WORLD_FOLDS];
bool worldFoldExists[MAX_WORLD_FOLDS];
int worldFoldKey[MAX_WORLD_FOLDS];


void worldFoldMatrixUpdate(int fromIndex)
{
	assert(fromIndex >= 0);
	assert(fromIndex < MAX_WORLD_FOLDS);

	if (fromIndex == CENTER_WORLD_FOLD_INDEX)
	{
		int i = CENTER_WORLD_FOLD_INDEX;
		worldFoldResultMatrix[i] = worldFoldMatrix[i];
		worldFoldKey[i] = (((worldFoldKey[i] + 1) & 0x000fffff) | (i << 20));
	}

	if (fromIndex <= CENTER_WORLD_FOLD_INDEX)
	{
		MAT result;
		result.CreateIdentityMatrix();
		for (int i = fromIndex; i >= 0; i--)
		{
			worldFoldResultMatrix[i] = result*worldFoldMatrix[i];
			result = worldFoldResultMatrix[i];
			worldFoldKey[i] = (((worldFoldKey[i] + 1) & 0x000fffff) | (i << 20));
		}
	}

	if (fromIndex >= CENTER_WORLD_FOLD_INDEX)
	{
		MAT result;
		result.CreateIdentityMatrix();
		for (int i = fromIndex; i < MAX_WORLD_FOLDS; i++)
		{
			worldFoldResultMatrix[i] = result*worldFoldMatrix[i];
			result = worldFoldResultMatrix[i];
			worldFoldKey[i] = (((worldFoldKey[i] + 1) & 0x000fffff) | (i << 20));
		}
	}

}


int worldFoldPositionToIndex(const VC3 &position)
{
	int index = CENTER_WORLD_FOLD_INDEX + (int)((position.x - worldFoldCenter.x + worldFoldStep) / worldFoldStep);

	if (index < 0) index = 0;
	if (index >= MAX_WORLD_FOLDS) index = MAX_WORLD_FOLDS - 1;

	return index;
}


void WorldFold::initWorldFold(float foldStep)
{
	resetWorldFold();
	worldFoldStep = foldStep;
}


void WorldFold::uninitWorldFold()
{
	// nop?
}


void WorldFold::resetWorldFold()
{
	for (int i = 0; i < MAX_WORLD_FOLDS; i++)
	{
		worldFoldMatrix[i].CreateIdentityMatrix();
		worldFoldResultMatrix[i].CreateIdentityMatrix();
		worldFoldExists[i] = false;
		worldFoldKey[i] = 1;
	}
	worldFoldCenter = VC3(0,0,0);

	wfCameraMatrix.CreateIdentityMatrix();
	wfCameraPosition = VC3(0,0,0);
}


const MAT *WorldFold::getWorldFoldForPosition(const VC3 &position)
{
	int index = worldFoldPositionToIndex(position);

	return &worldFoldResultMatrix[index];
}


const int *WorldFold::getWorldFoldKeyForPosition(const VC3 &position)
{
	int index = worldFoldPositionToIndex(position);

	return &worldFoldKey[index];
}


void WorldFold::setWorldFoldCenter(const VC3 &position)
{
	worldFoldCenter = position;
}


void WorldFold::addWorldFoldAtPosition(const VC3 &position, const MAT &fold)
{
	int index = worldFoldPositionToIndex(position);

	assert(!worldFoldExists[index]);

	worldFoldExists[index] = true;
	worldFoldMatrix[index] = fold;

	worldFoldMatrixUpdate(index);
}


void WorldFold::changeWorldFoldAtPosition(const VC3 &position, const MAT &fold)
{
	int index = worldFoldPositionToIndex(position);
	
	assert(worldFoldExists[index]);
	if (!worldFoldExists[index])
		return;

	worldFoldMatrixUpdate(index);
}


void WorldFold::setCameraPosition(const VC3 &position)
{
	if (fabs(wfCameraPosition.x - position.x) > 0.001f)
	{
		// TODO: rotate camera toward half angle when approacing a fold...
		// once past the fold, on the other side, flip rotation to negative half angle to match new fold 
		wfCameraMatrix.CreateIdentityMatrix();

		// TODO: update only if camera matrix has changed.
		worldFoldMatrixUpdate(CENTER_WORLD_FOLD_INDEX);

		wfCameraPosition = position;
	}
}

