
#include "precompiled.h"

#include <sstream>

#include "TargetDisplayWindowUpdator.h"
#include "TargetDisplayButtonManager.h"
#include "TargetDisplayWindow.h"
#include "../game/Game.h"
#include "../game/Item.h"
#include "../game/ItemList.h"
#include "../game/ItemManager.h"
#include "../container/LinkedList.h"
#include "../game/GameScene.h"
#include "../game/GameUI.h"
#include "../game/Unit.h"
#include "../game/UnitType.h"
#include "../game/UnitList.h"
#include "../util/assert.h"
#include "MapWindow.h"

#include "CombatSubWindowFactory.h"
#include <cmath>

#include <Storm3D_UI.h>

using namespace game;

namespace ui {

///////////////////////////////////////////////////////////////////////////////
namespace {
class TargetDisplayWindowUpdatorRegisterer : public CombatSubWindowFactory::ICombatSubWindowConstructor
{
public:
	TargetDisplayWindowUpdatorRegisterer( const std::string& s )
	{
		CombatSubWindowFactory::GetSingleton()->RegisterSubWindow( s, this );
	}

	ICombatSubWindow* CreateNewWindow( Ogui* ogui, game::Game* game, int player )
	{
		return new TargetDisplayWindowUpdator( game, new TargetDisplayWindow( ogui, game, player ) );
	}
};

TargetDisplayWindowUpdatorRegisterer* __attribute__((used)) temp_static_haxoring_constructing_thing = new TargetDisplayWindowUpdatorRegisterer( "TargetDisplayWindow" );


}
///////////////////////////////////////////////////////////////////////////////

namespace {

	struct Point 
	{
		int x;
		int y;
	};

	struct Rect
	{
		int x;
		int y;
		int w;
		int h;
	};

} // unnamed

///////////////////////////////////////////////////////////////////////////////

static float itemDistance = 10.0f;
static float updateItemsInsideDistance = 30.0f;

static float unitDistance = 50.0f;


//.............................................................................

Point convertVC3toScreen( VC3 pos, Game* game  )
{
	IStorm3D_Scene *scene = game->getGameScene()->getStormScene();
	IStorm3D_Camera *cam = scene->GetCamera();

	Point p;
	p.x = -1024; 
	p.y = -768;

	VC3 result = VC3(0,0,0);
	float rhw = 0;
	float real_z = 0;
	bool infront = cam->GetTransformedToScreen(pos, result, rhw, real_z);
	if (infront)
	{
		p.x = (int)(result.x * 1024.0f);
		p.y = (int)(result.y * 768.0f);
	}

	return p;
}

//.............................................................................

float calculateDistance( VC3 pos, Game* game )
{
	VC3 cam_pos;
	if( game->gameUI->getFirstPerson( 0 ) )
		cam_pos = game->gameUI->getFirstPerson( 0 )->getPosition();
	else
		cam_pos = game->gameUI->getGameCamera()->getPosition();

	return (float)( (cam_pos - pos ).GetSquareLength() );
	// return (float)( pow( ( cam_pos.x - pos.x ) , 2 ) + pow( ( cam_pos.z - pos.z ), 2 ) );
}

//.............................................................................

Rect getScreenArea(const AABB &aabb, Game  *game)
{
	VC3 v[8];
	v[0] = VC3(aabb.mmin.x, aabb.mmin.y, aabb.mmin.z);
	v[1] = VC3(aabb.mmax.x, aabb.mmin.y, aabb.mmin.z);
	v[2] = VC3(aabb.mmin.x, aabb.mmax.y, aabb.mmin.z);
	v[3] = VC3(aabb.mmin.x, aabb.mmin.y, aabb.mmax.z);
	v[4] = VC3(aabb.mmax.x, aabb.mmax.y, aabb.mmin.z);
	v[5] = VC3(aabb.mmin.x, aabb.mmax.y, aabb.mmax.z);
	v[6] = VC3(aabb.mmax.x, aabb.mmin.y, aabb.mmax.z);
	v[7] = VC3(aabb.mmax.x, aabb.mmax.y, aabb.mmax.z);

	Point start = convertVC3toScreen(v[0], game);
	Point end = start;

	for(int i = 1; i < 8; ++i)
	{
		Point p = convertVC3toScreen(v[i], game);
		start.x = std::min(start.x, p.x);
		start.y = std::min(start.y, p.y);
		end.x = std::max(end.x, p.x);
		end.y = std::max(end.y, p.y);
	}

	Rect rect;
	rect.x = start.x;
	rect.y = start.y;
	rect.w = end.x - start.x;
	rect.h = end.y - start.y;

	return rect;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef GUI_BUILD_MAP_WINDOW
Rect getTargetScreenRect( Game* game )
{
	Rect result;
	// game->gameUI;
	VC3 obj_pos;
	float radius = 2.0f;
	game->gameUI->getMapWindow()->getCurrentObjectivePosition( obj_pos );
	obj_pos.y += 18.0f;

	{
		VC3 tmp = obj_pos;
		tmp.x += radius;
		tmp.z += radius;

		Point p = convertVC3toScreen( tmp, game );
		result.x = p.x;
		result.y = p.y;
		result.w = 0;
		result.h = 0;
	}

	{
		VC3 tmp = obj_pos;
		tmp.x -= radius;
		tmp.z -= radius;

		Point p = convertVC3toScreen( tmp, game );
		if( p.x < result.x ) result.x = p.x;
		if( p.y < result.y ) result.y = p.y;
		if( p.x > result.x + result.w ) result.w = p.x - result.x;
		if( p.y > result.y + result.h ) result.h = p.y - result.y;
	}
	{
		VC3 tmp = obj_pos;
		tmp.x += radius;
		tmp.z -= radius;

		Point p = convertVC3toScreen( tmp, game );
		if( p.x < result.x ) result.x = p.x;
		if( p.y < result.y ) result.y = p.y;
		if( p.x > result.x + result.w ) result.w = p.x - result.x;
		if( p.y > result.y + result.h ) result.h = p.y - result.y;
	}
	{
		VC3 tmp = obj_pos;
		tmp.x -= radius;
		tmp.z += radius;

		Point p = convertVC3toScreen( tmp, game );
		if( p.x < result.x ) result.x = p.x;
		if( p.y < result.y ) result.y = p.y;
		if( p.x > result.x + result.w ) result.w = p.x - result.x;
		if( p.y > result.y + result.h ) result.h = p.y - result.y;
	}

	return result;
}
#endif

///////////////////////////////////////////////////////////////////////////////
/*
Rect getAsScreenRect()
{
	return Rect();
}
*/
///////////////////////////////////////////////////////////////////////////////

TargetDisplayWindowUpdator::TargetDisplayWindowUpdator( Game* game, TargetDisplayWindow* window ) :
    game( game ),
	currentFrame( 0 ),
	updateTargets( 20 ),
	removeUnnessary( 100 ),
	window( window ),
	risingMessages()
{
	FB_ASSERT( game != NULL );
	game->itemManager->setListener( this );
}

//=============================================================================

TargetDisplayWindowUpdator::~TargetDisplayWindowUpdator()
{
	FB_ASSERT( game != NULL );
	game->itemManager->setListener( NULL );
	delete window;
	window = NULL;
}

///////////////////////////////////////////////////////////////////////////////

void TargetDisplayWindowUpdator::hide( int time )
{
	window->hide( time );
}

void TargetDisplayWindowUpdator::show( int time )
{
	window->show( time );
}

//=============================================================================

void TargetDisplayWindowUpdator::update()
{
	// TargetDisplayButtonManager *manager = window->getManager();
	// rect tmp = getTargetScreenRect( game );
	// window->setRect( 0, tmp.x, tmp.y, tmp.w, tmp.h );
	
	{
		float updateDistance = itemDistance * itemDistance;

		std::list< Item* >::iterator it;
		for( it = itemsToBeUpdated.begin(); it != itemsToBeUpdated.end(); ++it )
		{
			//Point tmp = convertVC3toScreen( (*it)->getPosition(), game );
			Item *item = *it;

			float distance = calculateDistance( item->getPosition(), game );
			if( distance < updateDistance )
			{

				if(item && item->getVisualObject() && item->getVisualObject()->getStormModel())
				{
					IStorm3D_Model *m = item->getVisualObject()->getStormModel();
					Rect rect = getScreenArea(m->GetBoundingBox(), game);

					if(window->setRect(item, rect.x, rect.y, rect.w, rect.h, ( distance / updateDistance ), item->getHighlightStyle()))
					{
						if(item->hasHighlightText())
							window->setText(item, item->getHighlightText());
					}

					if( false ) // items cant have health thus they cant have the health bar
						window->setSliderValue( item, 1.0f, 1.0f );
				}

				/*
				if( window->setRect( (*it), tmp.x - 12, tmp.y - 8, 35, 35, (int)( ( distance / updateDistance ) * 100.0f ), (*it)->getStyle() ) )
				{
					if( (*it)->hasStyleText() )
						window->setText( (*it), (*it)->getStyleText() );	
				}
				*/
			}
		}
	}

	{
		float updateDistance = unitDistance * unitDistance;

		std::list< Unit* >::iterator it;
		for( it = unitsToBeUpdated.begin(); it != unitsToBeUpdated.end(); ++it )
		{
			//point tmp = convertVC3toScreen( (*it)->getPosition(), game );
			float distance = calculateDistance( (*it)->getPosition(), game );
			if( distance < updateDistance )
			{
				Unit *unit = *it;
				if(unit && unit->getVisualObject() && unit->getVisualObject()->getStormModel())
				{				
					UnitType *ut = unit->getUnitType();

					IStorm3D_Model *m = unit->getVisualObject()->getStormModel();
					Rect rect = {0, 0, 0, 0};

					// two rects, one for highlight and another for target lock rect
					for(int i = 0; i < 2; i++)
					{
						void *p = unit;
						int style = unit->getHighlightStyle();
						float rect_scale = 1.0f;
						const char *text = NULL;
						
						// target lock rect
						if(i == 1)
						{
							int max = unit->getTargetLockCounterMax();
							if(unit->getTargetLockCounter() <= 0)
							{
								// no target lock
								continue;
							}
							else if(unit->getTargetLockCounter() < max)
							{
								style = 5;
								rect_scale = 2.0f - 1.0f * (unit->getTargetLockCounter() / (float) max);
								text = NULL;
							}
							else
							{
								style = 6;
								rect_scale = 1.0f;
								text = "gui_target_locked";
							}

							p = ((char *)unit) + 2;
						}

						const TargetDisplayButtonManager::ButtonCreator &bc = window->getManager()->getButtonStyle( style );
						if(bc.rect_style == 0)
						{
							rect = getScreenArea(m->GetBoundingBox(), game);
						}
						else if(bc.rect_style == 1)
						{
							// unit size based rect
							//
							IStorm3D_Scene *scene = game->getGameScene()->getStormScene();
							IStorm3D_Camera *cam = scene->GetCamera();

							VC3 pos = m->GetPosition() + VC3(0, ut->getSize() * ut->getTargetRectHeightScale(), 0);
							VC3 pos_screen = VC3(0,0,0);
							VC3 pos_screen2 = VC3(0,0,0);
							float rhw = 0;
							float real_z = 0;
							bool visible = cam->GetTransformedToScreen(pos, pos_screen, rhw, real_z); 
							if(!visible)
								continue;

							pos += cam->GetUpVecReal() * ut->getSize() * 0.5f;
							visible = cam->GetTransformedToScreen(pos, pos_screen2, rhw, real_z); 
							if(!visible)
								continue;

							VC3 diff = pos_screen - pos_screen2;
							float radius = rect_scale * ut->getTargetRectScale() * sqrtf(diff.x * diff.x + diff.y * diff.y);

							pos_screen.x *= 1024.0f;
							pos_screen.y *= 768.0f;

							rect.x = (int)(pos_screen.x - radius * 1024.0f);
							rect.y = (int)(pos_screen.y - radius * 768.0f);
							rect.w = (int)(radius * 1024.0f * 2.0f);
							rect.h = (int)(radius * 768.0f * 2.0f);
						}

						if(window->setRect(p, rect.x, rect.y, rect.w, rect.h, ( distance / updateDistance ), style ))
						{
							if(unit->hasHighlightText() )
								window->setText(p, unit->getHighlightText() );	
							if(text)
								window->setText(p, text );	

						}

						window->setSliderValue(p, ( (float)unit->getHP() / (float)unit->getMaxHP() ), unit->getUnitType()->getHealthSliderScale() );
					}

				}
			}
		}
	}
	
	{
		float updateDistance = itemDistance * itemDistance;

		std::list< Unit* >::iterator it;
		for( it = risingMessages.begin(); it != risingMessages.end(); )
		{
			Unit *unit = *it;
			float distance = calculateDistance( unit->getPosition(), game );
			if(unit && unit->getVisualObject() && unit->getVisualObject()->getStormModel()
				&& distance < updateDistance)
			{
				IStorm3D_Model *m = unit->getVisualObject()->getStormModel();
				Rect rect = getScreenArea(m->GetBoundingBox(), game);

				// hack: allow rising messages on units with highlight
				void *p = ((char *)unit) + 1;
				if(window->setRect(p, rect.x, rect.y, rect.w, rect.h, ( distance / updateDistance ), risingMessageStyle ))
				{
					/*if(unit->hasHighlightText() )
						window->setText(p, unit->getHighlightText() );	
						*/
				}
			}
			else
			{
				// keep rising message visible even if unit was blown up
				// hack: allow rising messages on units with highlight
				void *p = ((char *)unit) + 1;
				window->updateRect(p);
			}

			// hack: allow rising messages on units with highlight
			void *p = ((char *)(*it)) + 1;
			if( window->hasEnded( p ) == false )
			{
				++it;
			}
			else
			{
				std::list< Unit* >::iterator remove = it;
				++it;
				risingMessages.erase( remove );
			}
		}
		
	}

	
	currentFrame++;

	if( currentFrame >= removeUnnessary )
	{
		currentFrame = 0;
		window->removeRest();
	}
	else
	{
		window->hideRest();
	}

	if( currentFrame%updateTargets == 0 )
		updateUpdatables();



}

void TargetDisplayWindowUpdator::risingMessage( game::Unit* unit, const std::string& text, int style )
{
	if( unit == NULL )
	{
		Logger::getInstance()->error( "TargetDisplayWindowUpdator::risingMessage() - could not create a message for a null unit" );
		return;
	}

	std::list< Unit* >::iterator i = std::find( risingMessages.begin(), risingMessages.end(), unit );
	if( i == risingMessages.end() )
	{
		float updateDistance = updateItemsInsideDistance * updateItemsInsideDistance;

		float distance = calculateDistance( unit->getPosition(), game );
		if( distance < updateDistance )
		{
			if(unit && unit->getVisualObject() && unit->getVisualObject()->getStormModel())
			{
				
				IStorm3D_Model *m = unit->getVisualObject()->getStormModel();
				Rect rect = getScreenArea(m->GetBoundingBox(), game);

				// hack: allow rising messages on units with highlight
				void *p = ((char *)unit) + 1;
				if(window->setRisingText( p, rect.x, rect.y, rect.w, rect.h, ( distance / updateDistance ), style ) )
				{
					window->setText(p, text );	
				}

				risingMessages.push_back( unit );
			}
		}

	}
	else
	{
		// TODO log a warning message
		// Logger::getInstance()->Warning( );
	}
}

///////////////////////////////////////////////////////////////////////////////

void TargetDisplayWindowUpdator::onDestruction( game::Item* item )
{
	std::list< Item* >::iterator i = std::find( itemsToBeUpdated.begin(), itemsToBeUpdated.end(), item );
	if( i != itemsToBeUpdated.end() )
	{
		itemsToBeUpdated.erase( i );
	}
}

///////////////////////////////////////////////////////////////////////////////

void TargetDisplayWindowUpdator::updateUpdatables()
{
	{
		LinkedList* list = game->items->getAllItems();
		
		itemsToBeUpdated.clear();

		float updateDistance = updateItemsInsideDistance * updateItemsInsideDistance;

		SafeLinkedListIterator i( list );

		while( i.iterateAvailable() )
		{
			Item* item = (Item*)i.iterateNext();
			VC3 pos = item->getPosition();
		

			if( item->getHighlightStyle() >= 0 && calculateDistance( pos, game ) < updateDistance )
			{
				itemsToBeUpdated.push_back( item );
			}
		}
	}

#ifdef PROJECT_SURVIVOR
	bool any_player_has_intelligence = false;
	int value = 0;
	if(util::Script::getGlobalIntVariableValue("enemyintelligence_in_use", &value) && value == 1)
	{
		for(int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
		{
			if(util::Script::getGlobalArrayVariableValue("upgraded_enemyintelligence", i, &value) && value == 1
				&& game->gameUI->getFirstPerson(i)
				&& strcmp(game->gameUI->getFirstPerson(i)->getUnitType()->getName(), "Surv_Sniper") == 0)
			{
				any_player_has_intelligence = true;
				break;
			}
		}
	}
#endif

	{
		VC3 cam_pos;
		if( game->gameUI->getFirstPerson( 0 ) )
			cam_pos = game->gameUI->getFirstPerson( 0 )->getPosition();
		else
			cam_pos = game->gameUI->getGameCamera()->getPosition();

		float updateDistance = unitDistance * unitDistance;

		unitsToBeUpdated.clear();

		IUnitListIterator *iter = game->units->getNearbyAllUnits(cam_pos, updateDistance);
		while (iter->iterateAvailable())
		{
			Unit *unit = (Unit *)iter->iterateNext();
			VC3 pos = unit->getPosition();


#ifdef PROJECT_SURVIVOR
			// this is an enemy
			if(unit->getOwner() == 1 || unit->getOwner() == 3)
			{
				// has intelligence on living enemy
				if(any_player_has_intelligence && unit->getHP() > 0)
				{
					// highlight if not already
					if(unit->getHighlightStyle() == -1)
						unit->setHighlightStyle(3);
				}
				// otherwise, if still highlighted by intelligence
				else if(unit->getHighlightStyle() == 3)
				{
					// unhighlight
					unit->setHighlightStyle(-1);
				}
			}
#endif

			// also include target locked enemies
			if( unit->getHighlightStyle() >= 0 || unit->getTargetLockCounter() > 0)
			{
				unitsToBeUpdated.push_back( unit );
			}
		}
		delete iter;
	}
}

///////////////////////////////////////////////////////////////////////////////
} // end of namespace ui
