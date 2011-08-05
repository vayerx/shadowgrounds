
#include "precompiled.h"

#include "GameWorldFold.h"
#include "../system/Logger.h"

#include <Storm3D_UI.h>

// HACK:
extern IStorm3D_Scene *disposable_scene;

namespace game
{
	GameWorldFold *GameWorldFold::instance = NULL;

	GameWorldFold *GameWorldFold::getInstance()
	{
		if (instance == NULL)
		{
			instance = new GameWorldFold();
		}
		return instance;
	}

	void GameWorldFold::cleanInstance()
	{
		if (instance != NULL)
		{
			delete instance;
			instance = NULL;
		}
	}


	GameWorldFold::GameWorldFold()
	{
		reset();
	}


	GameWorldFold::~GameWorldFold()
	{
	}


	void GameWorldFold::reset()
	{
		for (int i = 0; i < GAMEWORLDFOLD_MAX_FOLDS_PER_DIRECTION; i++)
		{
			positiveFolds[i] = MAT();
			negativeFolds[i] = MAT();
			positiveFoldPositions[i] = VC3(0,0,0);
			negativeFoldPositions[i] = VC3(0,0,0);
		}
		negativeFoldsUsed = 0;
		positiveFoldsUsed = 0;

		disposable_scene->resetWorldFold();
	}


	int GameWorldFold::addFold(const VC3 &position, float angle)
	{
		if (position.x > 0)
		{
			if (positiveFoldsUsed >= GAMEWORLDFOLD_MAX_FOLDS_PER_DIRECTION)
			{
				LOG_WARNING("GameWorldFold::addFold - Too many folds in positive position direction.");
				return 0;
			}

			// NOTE: currently adding folds to the end of the fold chain supported only
			for (int i = 0; i < positiveFoldsUsed; i++)
			{
				if (positiveFoldPositions[i].x > position.x)
				{
					LOG_ERROR("GameWorldFold::addFold - Attempt to add a positive position fold between existing folds (need to add after the last positive position fold).");
					return 0;
				}
			}

			VC3 positionFoldedByPrevious = position;

			MAT prevFoldMat;
			prevFoldMat.CreateIdentityMatrix();
			for (int i = 0; i < positiveFoldsUsed; i++)
			{
				prevFoldMat = prevFoldMat * positiveFolds[i];
			}

			prevFoldMat.TransformVector(positionFoldedByPrevious);

			MAT rotMat;
			MAT transMat;
			MAT transMatInv;
			MAT myFoldMat;

			rotMat.CreateRotationMatrix(QUAT(0, 0, angle * PI / 180.0f));
			transMatInv.CreateTranslationMatrix(positionFoldedByPrevious);
			transMat = transMatInv.GetInverse();
			myFoldMat = transMat * rotMat * transMatInv;

			MAT combinedFoldMat = prevFoldMat * myFoldMat;

			disposable_scene->addWorldFoldAtPosition(position, combinedFoldMat);

			positiveFolds[positiveFoldsUsed] = myFoldMat;
			positiveFoldPositions[positiveFoldsUsed] = position;

			positiveFoldsUsed++;

			return 1 + GAMEWORLDFOLD_MAX_FOLDS_PER_DIRECTION + (positiveFoldsUsed-1);
		}
		else if (position.x < 0)
		{
			if (negativeFoldsUsed >= GAMEWORLDFOLD_MAX_FOLDS_PER_DIRECTION)
			{
				LOG_WARNING("GameWorldFold::addFold - Too many folds in negative position direction.");
				return 0;
			}

			// NOTE: currently adding folds to the end of the fold chain supported only
			for (int i = 0; i < negativeFoldsUsed; i++)
			{
				if (negativeFoldPositions[i].x < position.x)
				{
					LOG_ERROR("GameWorldFold::addFold - Attempt to add a negative position fold between existing folds (need to add after the last negative position fold).");
					return 0;
				}
			}

			VC3 positionFoldedByPrevious = position;

			MAT prevFoldMat;
			prevFoldMat.CreateIdentityMatrix();
			for (int i = 0; i < negativeFoldsUsed; i++)
			{
				prevFoldMat = prevFoldMat * negativeFolds[i];
			}

			prevFoldMat.TransformVector(positionFoldedByPrevious);

			MAT rotMat;
			MAT transMat;
			MAT transMatInv;
			MAT myFoldMat;

			rotMat.CreateRotationMatrix(QUAT(0, 0, angle * PI / 180.0f));
			transMatInv.CreateTranslationMatrix(positionFoldedByPrevious);
			transMat = transMatInv.GetInverse();
			myFoldMat = transMat * rotMat * transMatInv;

			MAT combinedFoldMat = prevFoldMat * myFoldMat;

			disposable_scene->addWorldFoldAtPosition(position, combinedFoldMat);

			negativeFolds[negativeFoldsUsed] = myFoldMat;
			negativeFoldPositions[negativeFoldsUsed] = position;

			negativeFoldsUsed++;

			return 1 + (negativeFoldsUsed-1);
		} else {
			// fold at origo not supported. just ignore?
			LOG_WARNING("GameWorldFold::addFold - Fold at origo not supported.");
		}

		return 0;
	}

	int GameWorldFold::getFoldNumberAtPosition(const VC3 &position)
	{
		// TODO: ...
		return 0;
	}
	
	void GameWorldFold::moveFold(int foldNumber, const VC3 &position)
	{
		// TODO: ...
	}

	void GameWorldFold::setFoldAngle(int foldNumber, float angle)
	{
		// TODO: ...
	}

}
