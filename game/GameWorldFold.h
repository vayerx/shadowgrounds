
#ifndef GAMEWORLDFOLD_H
#define GAMEWORLDFOLD_H

#include <DatatypeDef.h>

#define GAMEWORLDFOLD_MAX_FOLDS_PER_DIRECTION 256

namespace game
{
	class GameWorldFold
	{
	public:
		static GameWorldFold *getInstance();

		void cleanInstance();

		GameWorldFold();
		~GameWorldFold();

		void reset();

		// returns positive fold id number 
		int addFold(const VC3 &position, float angle);

		// returns positive fold id number at given position, or 0 if no fold at given position
		int getFoldNumberAtPosition(const VC3 &position);
		
		void moveFold(int foldNumber, const VC3 &position);

		void setFoldAngle(int foldNumber, float angle);

	private:
		static GameWorldFold *instance;

		MAT negativeFolds[GAMEWORLDFOLD_MAX_FOLDS_PER_DIRECTION];
		VC3 negativeFoldPositions[GAMEWORLDFOLD_MAX_FOLDS_PER_DIRECTION];
		int negativeFoldsUsed;
		MAT positiveFolds[GAMEWORLDFOLD_MAX_FOLDS_PER_DIRECTION];
		VC3 positiveFoldPositions[GAMEWORLDFOLD_MAX_FOLDS_PER_DIRECTION];
		int positiveFoldsUsed;
	};
}

#endif
