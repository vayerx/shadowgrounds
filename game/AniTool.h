
#ifndef ANITOOL_H
#define ANITOOL_H

namespace game
{
	class AniToolImpl;
	class GameMap;

	class AniTool
	{
	public:
		AniTool();
		~AniTool();

		bool loadFile(const char *filename);
		void saveFile();
		void saveFileAs(const char *filename);
		void close();

		void setSelectionStart(int tickPosition); // inclusive
		void setSelectionEnd(int tickPosition);   // exclusive
		void setSelectionStartToStart();
		void setSelectionEndToEnd();

		void padTicksUntil(int tickPosition);

		void smoothMovement(int smoothAmount);
		void smoothRotation(int smoothAmount);
		void smoothAim(int smoothAmount);

		void dropMovementOnGround();

		void offsetFirstWarp(float offsetX, float offsetZ);
		void tweakMovementToward(float offsetX, float offsetZ, bool smooth);
		void dropMovementOnGround(GameMap *gameMap);

		void removeWarps();
		void deleteSelection();

		// note: may need to pad ticks.
		void movementSpeedFactor(float factor);

		void loseEveryNthTick(int tickLoseInterval);

	private:
		AniToolImpl *impl;
	};

}

#endif
