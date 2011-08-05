
#include "precompiled.h"

#include "ProgressBar.h"

#include "gamedefs.h"
#include "../util/AngleRotationCalculator.h"

//#include "Unit.h"

// TEMP: for debugging
#include "../system/Logger.h"
#include "../convert/str2int.h"

#include "../util/fb_assert.h"
#include "../util/Debug_MemoryManager.h"


#define PROGRESSBAR_PLAYER_MOVE_TRESHOLD 1.0f
#define PROGRESSBAR_PLAYER_ROTATE_TRESHOLD 60.0f

// (ticks)
#define PROGRESSBAR_VISIBLE_AFTER_STOP (2000 / GAME_TICK_MSEC)


namespace game
{
	ProgressBar::ProgressBar()
	{
		interruptPercent = 1.0f;

		timeDone = 0;
		timeTotal = 1;
		timeTick = 0;

		unit = NULL;

		label = NULL;
		doneLabel = NULL;
		interruptedLabel = NULL;

		borderImage = NULL;
		barImage = NULL;

		visible = false;
		visibilityLeftCounter = 0;

		doneTriggered = false;
		nextTickTrigger = 0;

		progressing = false;
		triggerInterrupted = false;

		progressPosition = VC3(0,0,0);
		progressAngle = 0;

		restartedWhileVisible = false;
	}


	ProgressBar::~ProgressBar()
	{
		// NOTE: ProgressBarActor handles visualization deletion?
		if (isProgressing())
		{
			stopProgress();
		}
	}


	void ProgressBar::setInterruptPercent( int percent )
	{
		interruptPercent = ( (float)percent / 100.0f );
	}


	bool ProgressBar::isProgressing() const
	{
		return progressing;
	}


	bool ProgressBar::isDone() const
	{
		if( interruptPercent >= 1.0f )
		{
			if (timeDone >= timeTotal)
				return true;
			else
				return false;
		}
		else
		{
			if( timeDone >= (int)( (float)timeTotal * interruptPercent + 0.5f ) )
				return true;
			else
				return false;
		}
	}


	void ProgressBar::run(const VC3 &playerPosition, float playerAngle,
		bool continueKeyPressed, bool interruptKeyPressed)
	{
		if (isProgressing())
		{
			// has player moved/rotated too much to interrupt the progress
			bool playerInterrupted = interruptKeyPressed;

			if (!continueKeyPressed)
				playerInterrupted = true;

			VC3 moved = playerPosition - this->progressPosition;
			if (moved.GetSquareLength() > PROGRESSBAR_PLAYER_MOVE_TRESHOLD*PROGRESSBAR_PLAYER_MOVE_TRESHOLD)
				playerInterrupted = true;

			float rotAngle = util::AngleRotationCalculator::getFactoredRotationForAngles(playerAngle, this->progressAngle, 0.0f);
			if (fabs(rotAngle) > PROGRESSBAR_PLAYER_ROTATE_TRESHOLD)
				playerInterrupted = true;

			if (playerInterrupted)
			{
				progressing = false;
				triggerInterrupted = true;
				visibilityLeftCounter = PROGRESSBAR_VISIBLE_AFTER_STOP;
			}

			if (!isDone())
			{
				timeDone++;
			} else {
				progressing = false;
				visibilityLeftCounter = PROGRESSBAR_VISIBLE_AFTER_STOP;
			}
		} else {
			if (visibilityLeftCounter > 0)
			{
				visibilityLeftCounter--;
				if (visibilityLeftCounter == 0)
				{
					visible = false;
				}
			}
		}
	}


	bool ProgressBar::doesTriggerDone()
	{
		if (isDone())
		{
			if (doneTriggered)
			{
				return false;
			} else {
				doneTriggered = true;
				return true;
			}
		} else {
			return false;
		}
	}


	bool ProgressBar::doesTriggerTick()
	{
		if (timeTick > 0)
		{
			if (timeDone >= nextTickTrigger && timeDone != timeTotal)
			{
				nextTickTrigger += timeTick;
				return true;
			} else {
				return false;
			}
		} else {
			return false;
		}
	}


	bool ProgressBar::doesTriggerInterrupted()
	{
		if (triggerInterrupted)
		{
			triggerInterrupted = false;
			return true;
		} else {
			return false;
		}
	}


	//bool ProgressBar::doesTriggerResumed(); (done at item execute)
	//{
	//}


	void ProgressBar::setLabel(const char *label)
	{
		if (this->label != NULL)
		{
			delete [] this->label;
			this->label = NULL;
		}
		if (label != NULL)
		{
			this->label = new char[strlen(label) + 1];
			strcpy(this->label, label);
		}
	}


	void ProgressBar::setDoneLabel(const char *doneLabel)
	{
		if (this->doneLabel != NULL)
		{
			delete [] this->doneLabel;
			this->doneLabel = NULL;
		}
		if (doneLabel != NULL)
		{
			this->doneLabel = new char[strlen(doneLabel) + 1];
			strcpy(this->doneLabel, doneLabel);
		}
	}


	void ProgressBar::setInterruptedLabel(const char *interruptedLabel)
	{
		if (this->interruptedLabel != NULL)
		{
			delete [] this->interruptedLabel;
			this->interruptedLabel = NULL;
		}
		if (interruptedLabel != NULL)
		{
			this->interruptedLabel = new char[strlen(interruptedLabel) + 1];
			strcpy(this->interruptedLabel, interruptedLabel);
		}
	}


	void ProgressBar::setBarImage(const char *barImage)
	{
		if (this->barImage != NULL)
		{
			delete [] this->barImage;
			this->barImage = NULL;
		}
		if (barImage != NULL)
		{
			this->barImage = new char[strlen(barImage) + 1];
			strcpy(this->barImage, barImage);
		}
	}


	void ProgressBar::setBorderImage(const char *borderImage)
	{
		if (this->borderImage != NULL)
		{
			delete [] this->borderImage;
			this->borderImage = NULL;
		}
		if (borderImage != NULL)
		{
			this->borderImage = new char[strlen(borderImage) + 1];
			strcpy(this->borderImage, borderImage);
		}
	}


	void ProgressBar::stopProgress()
	{
		this->progressing = false;
		this->triggerInterrupted = false;
		this->doneTriggered = false;
		this->timeDone = 0;
		this->nextTickTrigger = 0;
	}


	void ProgressBar::interruptProgress()
	{
		this->progressing = false;
		this->triggerInterrupted = true;
		this->doneTriggered = false;
	}


	void ProgressBar::startProgress(Unit *unit, const VC3 &playerPosition, float playerAngle)
	{
		fb_assert(!this->progressing);

		if (isDone())
			return;

		this->unit = unit;

		this->progressPosition = playerPosition;
		this->progressAngle = playerAngle;
		this->progressing = true;
		this->triggerInterrupted = false;
		this->doneTriggered = false;

		if (this->visibilityLeftCounter > 0)
		{
			this->restartedWhileVisible = true;
		} else {
			this->restartedWhileVisible = 0;
		}

		this->visibilityLeftCounter = 0;
		this->visible = true;
	}

	
	void ProgressBar::restartProgress()
	{
		VC3 tmp = this->progressPosition;
		float tmp2 = this->progressAngle;
		stopProgress();
		startProgress(this->unit, tmp, tmp2);
	}


	int ProgressBar::getProgressDone() const
	{
		return this->timeDone;
	}


	int ProgressBar::getProgressDonePercentage() const
	{
		return (this->timeDone * 100) / this->timeTotal;
	}

	void ProgressBar::setTotalTime(int totalTime)
	{
		this->timeTotal = totalTime;
	}

	void ProgressBar::setTickTime(int tickTime)
	{
		this->timeTick = tickTime;
	}


}

