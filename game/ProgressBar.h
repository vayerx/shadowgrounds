
#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <DatatypeDef.h>

namespace game
{
	class Unit;

	class ProgressBar
	{
		public:
			ProgressBar();
			
			~ProgressBar();

			bool isProgressing() const;

			bool isDone() const;

			void run(const VC3 &playerPosition, float playerAngle,
				bool continueKeyPressed, bool interruptKeyPressed);

			bool doesTriggerDone();
			bool doesTriggerTick();
			bool doesTriggerInterrupted();
			//bool doesTriggerResumed(); (done at item execute)

			void setInterruptPercent( int percent );

			void setLabel(const char *label);

			void setDoneLabel(const char *doneLabel);

			void setInterruptedLabel(const char *interruptedLabel);

			void setBarImage(const char *barImage);

			void setBorderImage(const char *borderImage);

			void stopProgress();

			void interruptProgress();

			void startProgress(Unit *unit, const VC3 &playerPosition, float playerAngle);

			void restartProgress();

			int getProgressDone() const;

			int getProgressDonePercentage() const;

			void setTotalTime(int totalTime);
			void setTickTime(int tickTime);

			Unit *getUnit() { return unit; }

		private:

			float interruptPercent;

			// (these in game ticks)
			int timeDone;
			int timeTotal;
			int timeTick;

			char *label;
			char *doneLabel;
			char *interruptedLabel;

			char *borderImage;
			char *barImage;

			bool visible;
			int visibilityLeftCounter;

			bool doneTriggered;
			int nextTickTrigger;

			bool progressing;
			bool triggerInterrupted;

			VC3 progressPosition;
			float progressAngle;

			bool restartedWhileVisible;

			Unit *unit;

			friend class ProgressBarActor;
	};
}

#endif

