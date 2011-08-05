
#include "precompiled.h"

#include "ProgressBar.h"
#include "ProgressBarActor.h"
#include "Game.h"
#include "GameUI.h"
#include "../ui/UIEffects.h"
#include "../ui/GameController.h"
#include "../system/Logger.h"
#include "../convert/str2int.h"
#include "Item.h"
#include "scripting/GameScripting.h"
#include "../game/DHLocaleManager.h"

#include "../util/fb_assert.h"

#define PROGRESSBAR_BORDER_LAYER 8
#define PROGRESSBAR_BAR_LAYER 9
#define PROGRESSBAR_LABEL_LAYER 10

namespace game
{
	
	ProgressBarActor::ProgressBarActor(Game *game, ProgressBar *progressBar,
		Item *item, Unit *unit)
	{
		this->game = game;
		this->progressBar = progressBar;
		this->item = item;
		this->unit = unit;

		fb_assert(item != NULL);
		fb_assert(progressBar != NULL);
	}


	void ProgressBarActor::run(const VC3 &playerPosition, float playerAngle)
	{
		fb_assert(progressBar != NULL);
		fb_assert(game != NULL);

		bool changeLabel = false;
		bool useInterruptedLabel = false;

		bool wasVisible = progressBar->visible;

		bool interruptKeyPressed = false;
		bool continueKeyPressed = false;

		GameController *ctrl = game->gameUI->getController(0);
		if (this->unit == game->gameUI->getFirstPerson(1))
		{
			ctrl = game->gameUI->getController(1);
		}
		if (this->unit == game->gameUI->getFirstPerson(2))
		{
			ctrl = game->gameUI->getController(2);
		}
		if (this->unit == game->gameUI->getFirstPerson(3))
		{
			ctrl = game->gameUI->getController(3);
		}


		if(this->unit->isDestroyed() || this->unit->getMoveState() == Unit::UNIT_MOVE_STATE_UNCONSCIOUS)
		{
			interruptKeyPressed = true;
			continueKeyPressed = false;
		}
		else
		{
			if (ctrl->isKeyDown(DH_CTRL_ATTACK))
			{
				interruptKeyPressed = true;
			}
			if (ctrl->isKeyDown(DH_CTRL_ATTACK_SECONDARY))
			{
				interruptKeyPressed = true;
			}
			if (ctrl->isKeyDown(DH_CTRL_GRENADE))
			{
				interruptKeyPressed = true;
			}
			if (ctrl->isKeyDown(DH_CTRL_EXECUTE))
			{
				continueKeyPressed = true;
			}
		}

		bool wasInterruptedButVisible = false;
		if (progressBar->restartedWhileVisible)
		{
			progressBar->restartedWhileVisible = false;
			wasInterruptedButVisible = true;
			changeLabel = true;
		}

		progressBar->run(playerPosition, playerAngle, continueKeyPressed, interruptKeyPressed);

		if (progressBar->doesTriggerDone())
		{
			game->gameScripting->runItemProgressBarScript(item, unit, "progress_done");
			changeLabel = true;
		}
		else if (progressBar->doesTriggerInterrupted())
		{
			game->gameScripting->runItemProgressBarScript(item, unit, "progress_interrupted");
			useInterruptedLabel = true;
			changeLabel = true;
		}
		else if (progressBar->doesTriggerTick())
		{
			game->gameScripting->runItemProgressBarScript(item, unit, "progress_tick");
		}

		ui::UIEffects *eff = game->gameUI->getEffects();
		int restoreLayer = eff->getActiveMaskPictureLayer();

		// TODO: clear any previous mask pictures/texts (but only is just started
		// this one!)
		// (now, this does not work properly if the mask picture layers are already in
		// use for some reason...)

		if (!progressBar->visible && wasVisible)
		{
			eff->setActiveMaskPictureLayer(PROGRESSBAR_BORDER_LAYER);
			eff->clearMaskPictureText();
			eff->clearMaskPicture();
			eff->setActiveMaskPictureLayer(PROGRESSBAR_BAR_LAYER);
			eff->clearMaskPictureText();
			eff->clearMaskPicture();
			eff->setActiveMaskPictureLayer(PROGRESSBAR_LABEL_LAYER);
			eff->clearMaskPictureText();
			eff->clearMaskPicture();
		}

		if (progressBar->visible)
		{
			eff->setActiveMaskPictureLayer(PROGRESSBAR_BORDER_LAYER);
			eff->setMaskPicturePosition(UIEFFECTS_MASK_PICTURE_POS_XY_COORDINATES);
			eff->setMaskPicturePositionX(getLocaleGuiInt("gui_progressbar_border_position_x", 0));
			eff->setMaskPicturePositionY(getLocaleGuiInt("gui_progressbar_border_position_y", 0));
			eff->setMaskPictureSizeX(getLocaleGuiInt("gui_progressbar_border_size_x", 0));
			eff->setMaskPictureSizeY(getLocaleGuiInt("gui_progressbar_border_size_y", 0));
			if (!eff->isMaskPicture())
			{
				if (progressBar->borderImage != NULL)
				{
					eff->setMaskPicture(progressBar->borderImage);
				} else {
					eff->setMaskPicture(getLocaleGuiString("gui_progressbar_border_default_image"));
				}
			}

			eff->setActiveMaskPictureLayer(PROGRESSBAR_BAR_LAYER);
			eff->setMaskPicturePosition(UIEFFECTS_MASK_PICTURE_POS_XY_COORDINATES);
			eff->setMaskPicturePositionX(getLocaleGuiInt("gui_progressbar_bar_position_x", 0));
			eff->setMaskPicturePositionY(getLocaleGuiInt("gui_progressbar_bar_position_y", 0));
			eff->setMaskPictureSizeX(progressBar->getProgressDonePercentage() * getLocaleGuiInt("gui_progressbar_bar_size_x", 0) / 100);
			eff->setMaskPictureSizeY(getLocaleGuiInt("gui_progressbar_bar_size_y", 0));
			if (!eff->isMaskPicture())
			{
				if (progressBar->barImage != NULL)
				{
					eff->setMaskPicture(progressBar->barImage);
				} else {
					eff->setMaskPicture(getLocaleGuiString("gui_progressbar_bar_default_image"));
				}
			}

			eff->setActiveMaskPictureLayer(PROGRESSBAR_LABEL_LAYER);
			eff->setMaskPicturePosition(UIEFFECTS_MASK_PICTURE_POS_XY_COORDINATES);
			if (!eff->isMaskPicture())
			{
				eff->setMaskPicture(NULL);
				changeLabel = true;
			}

			const char *label = NULL;
			int labelPosX = 0;
			int labelPosY = 0;
			int labelSizeX = 0;
			int labelSizeY = 0;
			if (progressBar->isDone())
			{
				label = progressBar->doneLabel;
				if (label == NULL)
				{
					label = getLocaleGuiString("gui_progressbar_donelabel_default_text");
				}
				labelSizeX = getLocaleGuiInt("gui_progressbar_donelabel_size_x", 0);
				labelSizeY = getLocaleGuiInt("gui_progressbar_donelabel_size_y", 0);
				labelPosX = getLocaleGuiInt("gui_progressbar_donelabel_position_x", 0);
				labelPosY = getLocaleGuiInt("gui_progressbar_donelabel_position_y", 0);
			} else {
				if (useInterruptedLabel)
				{
					label = progressBar->interruptedLabel;
					if (label == NULL)
					{
						label = getLocaleGuiString("gui_progressbar_interruptedlabel_default_text");
					}
					labelSizeX = getLocaleGuiInt("gui_progressbar_interruptedlabel_size_x", 0);
					labelSizeY = getLocaleGuiInt("gui_progressbar_interruptedlabel_size_y", 0);
					labelPosX = getLocaleGuiInt("gui_progressbar_interruptedlabel_position_x", 0);
					labelPosY = getLocaleGuiInt("gui_progressbar_interruptedlabel_position_y", 0);
				} else {
					label = progressBar->label;
					if (label == NULL)
					{
						label = getLocaleGuiString("gui_progressbar_label_default_text");
					}
					labelSizeX = getLocaleGuiInt("gui_progressbar_label_size_x", 0);
					labelSizeY = getLocaleGuiInt("gui_progressbar_label_size_y", 0);
					labelPosX = getLocaleGuiInt("gui_progressbar_label_position_x", 0);
					labelPosY = getLocaleGuiInt("gui_progressbar_label_position_y", 0);
				}
			}

			if (changeLabel)
			{
				eff->setMaskPicturePositionX(labelPosX);
				eff->setMaskPicturePositionY(labelPosY);
				eff->setMaskPictureSizeX(labelSizeX);
				eff->setMaskPictureSizeY(labelSizeY);
				eff->setMaskPictureTextPositionX(labelPosX);
				eff->setMaskPictureTextPositionY(labelPosY);
				eff->setMaskPictureTextAreaSizeX(labelSizeX);
				eff->setMaskPictureTextAreaSizeY(labelSizeY);

				if (label != NULL)
				{
					eff->setMaskPictureFont(getLocaleGuiString("gui_progressbar_font"));
					eff->setMaskPictureText(label, OguiButton::TEXT_H_ALIGN_CENTER );
				}
			}

		} else {
			// NOTE: visibility may not necessarily mean the same thing as existance/progressing
			// in this case, we can assume it to be true though.

			// delete it now? 
			// (note: can't delete it if it's not done, or it will lose its current progress)
			if (progressBar->isDone())
			{
				item->deleteProgressBar();
			}
		}

		eff->setActiveMaskPictureLayer(restoreLayer);
	}


}

