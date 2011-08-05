
#include "precompiled.h"

#include "SelectionBox.h"

#include <Storm3D_UI.h>

#include "../game/Game.h"
#include "../ogui/Ogui.h"
#include "../game/UnitList.h"
#include "../game/UnitSelections.h"
#include "../game/GameUI.h"
#include "../game/GameScene.h"
#include "../ui/GameController.h"
#include "../container/LinkedList.h"
#include "../util/Intersect.h"
//#include "../system/Timer.h"


IStorm3D_Material *selectionbox_mat = NULL;

using namespace game;

namespace ui
{

  SelectionBox::SelectionBox(game::Game *game, int player,
		int renderScaleX, int renderScaleY)
  {
    this->game = game;
    this->player = player;
    this->currentX = 0;
    this->currentY = 0;
    this->boxStartX = 0;
    this->boxStartY = 0;
		this->active = false;

		this->renderScaleX = renderScaleX;
		this->renderScaleY = renderScaleY;

		// quick hack
    IStorm3D *s3d = game->getGameScene()->getStorm3D();
		selectionbox_mat = s3d->CreateNewMaterial("foo");
		selectionbox_mat->SetColor(COL(0,1,0));
  }


  SelectionBox::~SelectionBox()
  {
		delete selectionbox_mat;
    // TODO 
    // (nop?)
  }


  void SelectionBox::selectionStarted(int startX, int startY)
  {
    boxStartX = startX;
    boxStartY = startY;
    currentX = startX;
    currentY = startY;
		active = true;
  }


  void SelectionBox::selectionPositionUpdate(int posX, int posY)
  {
    currentX = posX;
    currentY = posY;
  }


  void SelectionBox::selectionEnded(int endX, int endY)
  {
    currentX = endX;
    currentY = endY;
    if (!selectionActive())
		{
			active = false;
      return;
		}

		active = false;

		// psd: first check if clicked on waypoint/shooting target for selected unit

		// Drag box selection
		/*
		SceneSelection dragUpperLeft=game->gameUI->cursorRayTrace(boxStartX,boxStartY);
		SceneSelection dragUpperRight=game->gameUI->cursorRayTrace(boxStartX,endY);
		SceneSelection dragLowerLeft=game->gameUI->cursorRayTrace(endX,boxStartY);
		SceneSelection dragLowerRight=game->gameUI->cursorRayTrace(endX,endY);
		
		VC2 v1(dragUpperLeft.scaledMapX, dragUpperLeft.scaledMapY);
		VC2 v2(dragUpperRight.scaledMapX,dragUpperRight.scaledMapY);
		VC2 v3(dragLowerLeft.scaledMapX, dragLowerLeft.scaledMapY);
		VC2 v4(dragLowerRight.scaledMapX,dragLowerRight.scaledMapY);
		*/

		if(game->gameUI->getController(player)->isKeyDown(DH_CTRL_MULTIPLE_UNIT_SELECT)) 
		{
			//doBoxSelection(BOX_ADD_SELECTION,v1,v2,v3,v4);
			doBoxSelection(BOX_ADD_SELECTION);
		} 
		else 
		{
			//doBoxSelection(BOX_SELECT,v1,v2,v3,v4);
			doBoxSelection(BOX_SELECT);
		}

  }


  bool SelectionBox::selectionActive()
  {
		// first check if the selection has been started (and not ended)
		if (!active) return false;

		// then check that the selection box exceeds minimum size
    if (abs(currentX - boxStartX) >= SELECTION_BOX_MIN_SIZE
      || abs(currentY - boxStartY) >= SELECTION_BOX_MIN_SIZE)
    {
      return true;
    } else {
      return false;
    }
  }


  void SelectionBox::render()
  {
    if (selectionActive())
    {
			// this most certainly does not work
			// first of all, nothing happens...
			// and secondly, if something would happen, this would actually
			// make new lines constantly and not remove the old ones.
			// thus causing lots of lines...
			// and if would draw the lines to world coordinates (near terrain)
      			IStorm3D_Scene *scene = game->getGameScene()->getStormScene();
			
			int sx = boxStartX * renderScaleX / OGUI_SCALE_MULTIPLIER;
			int sy = boxStartY * renderScaleY / OGUI_SCALE_MULTIPLIER;
			int cx = currentX * renderScaleX / OGUI_SCALE_MULTIPLIER;
			int cy = currentY * renderScaleY / OGUI_SCALE_MULTIPLIER;
			if (sx > cx) { sx ^= cx; cx ^= sx; sx ^= cx; }
			if (sy > cy) { sy ^= cy; cy ^= sy; sy ^= cy; }

			VC2 p1 = VC2((float)sx, (float)sy);
			VC2 s1 = VC2((float)(cx - sx + 1), 1);
			VC2 p2 = VC2((float)sx, (float)sy);
			VC2 s2 = VC2(1, (float)(cy - sy + 1));
			VC2 p3 = VC2((float)sx, (float)cy);
			VC2 s3 = VC2((float)(cx - sx + 1), 1);
			VC2 p4 = VC2((float)cx, (float)sy);
			VC2 s4 = VC2(1, (float)(cy - sy + 1));

			scene->Render2D_Picture(selectionbox_mat, p1, s1);
			scene->Render2D_Picture(selectionbox_mat, p2, s2);
			scene->Render2D_Picture(selectionbox_mat, p3, s3);
			scene->Render2D_Picture(selectionbox_mat, p4, s4);
    }
  }


  void SelectionBox::doBoxSelection(SelectionBox::BoxSelectionType type)
//		                                VC2& a, VC2& b, VC2& c, VC2& d) 
	{
    //int now = Timer::getTime();
		IStorm3D_Scene *scene = game->getGameScene()->getStormScene();
		IStorm3D_Camera *cam = scene->GetCamera();

		float minX;
		float maxX;
		float minY;
		float maxY;
		if (boxStartX < currentX)
		{
			minX = (float)boxStartX;
			maxX = (float)currentX;
		} else {
			minX = (float)currentX;
			maxX = (float)boxStartX;
		}
		if (boxStartY < currentY)
		{
			minY = (float)boxStartY;
			maxY = (float)currentY;
		} else {
			minY = (float)currentY;
			maxY = (float)boxStartY;
		}
		minX /= 1024.0f;
		maxX /= 1024.0f;
		minY /= 768.0f;
		maxY /= 768.0f;

    LinkedListIterator iter = LinkedListIterator(
      game->units->getOwnedUnits(player));

		switch(type) {
		case SelectionBox::BOX_SELECT:

			//unitsSelected=0;
 			//for (i = 0; i < COMBATW_UNITS; i++) {
			//	Unit *u = solveUnitForNumber(i);

      game->unitSelections[player]->selectAllUnits(false);

      while (iter.iterateAvailable())
      {
        Unit *u = (Unit *)iter.iterateNext();

				VC3 position=u->getPosition();

				if(!u->isDestroyed() && u->isActive()) 
				{
					//bool isInside1=util::isPointInsideTriangle(VC2(position.x,position.z),a,b,c);
					//bool isInside2=util::isPointInsideTriangle(VC2(position.x,position.z),b,c,d);
					//if( isInside1 || isInside2 ) {

					bool unitInsideBox = false;
					VC3 result = VC3(0,0,0);
					float rhw = 0;
					float real_z = 0;
					bool infront = cam->GetTransformedToScreen(position, result, rhw, real_z);
					if (infront)
					{
						if (result.x >= minX && result.x <= maxX
							&& result.y >= minY && result.y <= maxY
							&& real_z < 200)
						{
							unitInsideBox = true;
						}
					}

					if (unitInsideBox)
					{
            game->unitSelections[player]->selectUnit(u, true);
            //u->setSelected(true);
						//unitButs[i]->SetImage(unitBackSelectedImage);
						//unitsSelected++;
						//unitSelectTime[i]=now;
					//} else { // Outside
            //game->unitSelections[player]->selectUnit(u, false);
						//u->setSelected(false);
						//unitButs[i]->SetImage(unitBackImage);
					}
				}
			}
			break;

		case SelectionBox::BOX_ADD_SELECTION:
      while (iter.iterateAvailable())
      {
        Unit *u = (Unit *)iter.iterateNext();
 			//for (i = 0; i < COMBATW_UNITS; i++) {
			//	Unit *u = solveUnitForNumber(i);
				VC3 position=u->getPosition();

				bool unitInsideBox = false;
				VC3 result = VC3(0,0,0);
				float rhw = 0;
				float real_z = 0;
				bool infront = cam->GetTransformedToScreen(position, result, rhw, real_z);
				if (infront)
				{
					if (result.x >= minX && result.x <= maxX
						&& result.y >= minY && result.y <= maxY
						&& real_z < 200)
					{
						unitInsideBox = true;
					}
				}

				if(!u->isSelected() && !u->isDestroyed() && u->isActive()) 
				{
					//bool isInside1=util::isPointInsideTriangle(VC2(position.x,position.z),a,b,c);
					//bool isInside2=util::isPointInsideTriangle(VC2(position.x,position.z),b,c,d);
					//if( isInside1 || isInside2 ) {
					if (unitInsideBox)
					{
            game->unitSelections[player]->selectUnit(u, true);
						//u->setSelected(true);
						//unitButs[i]->SetImage(unitBackSelectedImage);
						//unitsSelected++;
						//unitSelectTime[i]=now;
					} 
				}
			}
			break;

		case SelectionBox::BOX_SUB_SELECTION:
			// TODO: do we need this?
			break;
		default:
			/* euh, shouldn't happen */
      // thus, assert ;)
      assert(0);
			break;
		}
	}

}


