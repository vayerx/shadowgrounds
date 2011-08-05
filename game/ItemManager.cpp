
#include "precompiled.h"

#include "ItemManager.h"

#include "Game.h"
#include "GameRandom.h"
#include "GameUI.h"
#include "GameScene.h"
#include "../util/ColorMap.h"
#include "scripting/GameScripting.h"
#include "Item.h"
#include "ItemType.h"
#include "ItemList.h"
#include "IItemListener.h"
#include "ProgressBar.h"
#include "ProgressBarActor.h"
#include "UnitList.h"
#include "VisualObjectModelStorage.h"
#include "../game/DHLocaleManager.h"
#include "../ui/Spotlight.h"
#include "../container/LinkedList.h"
#include "../convert/str2int.h"
#include "../util/SimpleParser.h"
#include "../system/Logger.h"
#include "../system/Timer.h"
#include "options/options_game.h"
#include "SimpleOptions.h"

#include "../util/fb_assert.h"
#include "../util/Debug_MemoryManager.h"

#include <sstream>

// <physics_include>
#include "physics/GamePhysics.h"
#include "physics/BoxPhysicsObject.h"
#include "physics/CapsulePhysicsObject.h"
#include "options/options_physics.h"
#include "SimpleOptions.h"
#include "scaledefs.h"
#include "physics/physics_collisiongroups.h"

#ifndef PHYSICS_NONE
#define PHYSICS_INUSE
#endif

// </physics_include>



#ifdef PROJECT_SURVIVOR
	#define MAX_ITEM_TYPES 80
#else
	#define MAX_ITEM_TYPES 64
#endif

#define ITEM_PICKUP_RANGE 1.2f

// note: cylinder height is -1/2*height to +1*height
#define ITEM_PICKUP_HEIGHT 2

#define ITEM_EXECUTE_RANGE 1.4f

#define ITEM_PICKUP_DELAY (1000 / GAME_TICK_MSEC)

// check for item pickups/executes every x game ticks.
#define ITEM_CHECK_INTERVAL 16


namespace game
{

namespace {

#ifdef PHYSICS_INUSE
void createPhysicsForItem( Item* item, GamePhysics* physics, float mass )
{

		// normal physics object and fluid containment physics object...
		if (true)
		{
			const VC3 &pos = item->getPosition();

			// UnitType *ut = unit->getUnitType();
			game::AbstractPhysicsObject *physobj = NULL;
			// if ( ut->getPhysicsObjectType() == UNITTYPE_PHYSICS_OBJECT_TYPE_CAPSULE )
			{
				VC3 box_measurements( 1.0f, 1.0f, 1.0f );
				QUAT rotation;

				if( item->getVisualObject() && item->getVisualObject()->getStormModel() )
				{
					AABB bounding_box = item->getVisualObject()->getStormModel()->GetBoundingBox();
					box_measurements = bounding_box.mmax - bounding_box.mmin;
					box_measurements *= 0.5f;
					
					// item->getVisualObject()->getStormModel()->SetRotation();
					// rotation = item->getVisualObject()->getStormModel()->GetRotation();
					rotation = QUAT( 
						UNIT_ANGLE_TO_RAD(item->getRotation().x), 
						UNIT_ANGLE_TO_RAD(item->getRotation().y), 
						UNIT_ANGLE_TO_RAD(item->getRotation().z) );
					
				}

				// float mass = 1.0f;
				const int collGroup = PHYSICS_COLLISIONGROUP_UNITS;
				// physobj = new game::CapsulePhysicsObject( physics, height, radius, mass, collGroup, pos );
				physobj = new game::BoxPhysicsObject( physics, box_measurements, mass, collGroup, pos );
				physobj->setRotation( rotation );
			}
			
			if (physobj != NULL)
			{
				item->setGamePhysicsObject( physobj );
				
				/*if (ut->hasPhysicsObjectDisabledAngularVelocity())
				{
					physobj->disableAngularVelocity();
				}
				if (ut->hasPhysicsObjectDisabledYMovement())
				{
					physobj->disableYMovement();
				}
				*/

				// check if breaks something
				item->getVisualObject()->setRotationInterpolationAmount(0);
			}
		}
}
#endif

}

	ItemType *itemTypes = NULL;


	ItemManager::ItemManager(Game *game)
	{
		this->game = game;
		//this->executeUnit = NULL;

		if(game->items->getAllItemAmount() != 0)
		{
			Logger::getInstance()->warning("ItemManager - Items already exist at manager creation.");
		}

		if (itemTypes == NULL)
			loadItemTypes();

		listener = NULL;
	}


	ItemManager::~ItemManager()
	{
		clearAllSpawners();

		LinkedList *items = game->items->getAllItems();

		while(!items->isEmpty())
		{
			Item *item = (Item *)items->popLast();
			delete item;
		}

		fb_assert(itemTypes != NULL);

		unloadItemTypes();
	}


	ItemType *ItemManager::getItemTypeById(int itemId)
	{
		if (itemId >= 0 && itemId < MAX_ITEM_TYPES)
		{
			return &itemTypes[itemId];
		} else {
			Logger::getInstance()->error("ItemManager::getItemTypeById - Item id number out of range.");
			fb_assert(!"ItemManager::getItemTypeById - Item id number out of range.");
			return NULL;
		}		
	}


	void ItemManager::changeItemVisual(Item *item, const char *modelFilename)
	{
		fb_assert(item != NULL);

		if (item->getVisualObject() != NULL)
		{
			delete item->getVisualObject();
			item->setVisualObject(NULL);
		}

		if (modelFilename != NULL && modelFilename[0] != '\0')
		{
			VisualObjectModel *vom = game->visualObjectModelStorage->getVisualObjectModel(modelFilename);
			VisualObject *vo = vom->getNewObjectInstance();
			item->setVisualObject(vo);

			vo->setCollidable(false);
			vo->setInScene(true);
			vo->setVisible(true);

			VC3 itempos = item->getPosition();
			VC3 itemrot = item->getRotation();
			item->setRotation(itemrot);
			item->setPosition(itempos);

		}
	}


	Item *ItemManager::createNewItem(int itemId, const VC3 &position)
	{
		Item *item = NULL;
		if (itemId >= 0 && itemId < MAX_ITEM_TYPES)
		{
			item = new Item(itemId);
			game->items->addItem(item);

			if (itemTypes[itemId].getHalo() != NULL)
			{
				IStorm3D_Terrain *terrain = game->gameUI->getTerrain()->GetTerrain();
				IStorm3D_Scene *scene = game->getGameScene()->getStormScene();
				IStorm3D *storm3d = game->getGameScene()->getStorm3D();
				assert(terrain != NULL);
				if (terrain != NULL)
				{
					ui::Spotlight *sp = new ui::Spotlight(*storm3d, *terrain,
						*scene, NULL, std::string(itemTypes[itemId].getHalo()));
					item->setHalo(sp);
					sp->setPosition(position);
				}
			}

			if (itemTypes[itemId].getModelFilename() != NULL)
			{
				VisualObjectModel *vom = game->visualObjectModelStorage->getVisualObjectModel(itemTypes[itemId].getModelFilename());
				VisualObject *vo = vom->getNewObjectInstance();
				item->setVisualObject(vo);

				if (itemTypes[itemId].isWeaponType())
				{
					vo->setScale(VC3(1.2f, 1.2f, 1.2f));
				}

				vo->setCollidable(false);
				vo->setInScene(true);
				vo->setVisible(true);
			}
			
			VC3 itempos = position;
			if (itemTypes[itemId].isWeaponType())
			{
				item->setRotation(VC3(0, 90, 90));
				item->setWeaponType(true);
			}
			item->setPosition(itempos);

			
			{
				item->setHighlightStyle( itemTypes[itemId].getHighlightStyle() );
				item->setHighlightText( itemTypes[itemId].getHighlightText() );
				item->setBlinking( itemTypes[itemId].getBlinking() );
			}

		} else {
			Logger::getInstance()->warning("ItemManager::createNewItem - Item id number out of range.");
			assert(0);
		}

		return item;
	}

	void ItemManager::enablePhysics( Item* item, int itemType )
	{
#ifdef PHYSICS_INUSE

		ItemType* type = getItemTypeById( itemType );
		// hax hax physh-x
		if( true && type != NULL && item != NULL )
		{
			if( type->getPhysicsEnabled() )
			{
				createPhysicsForItem( item, game->getGamePhysics(), type->getPhysicsMass() );
			}
		}
#endif	
	}

	void ItemManager::setListener( IItemListener* listener )
	{
		this->listener = listener;
	}

	void ItemManager::deleteItem(Item *i)
	{
		assert(i != NULL);

		if( listener )
			listener->onDestruction( i );

		if( i->spawner )
		{
			i->spawner->spawned_item = NULL;
			queueForSpawn(i->spawner, false);
		}

		delete i;
		game->items->removeItem(i);
	}

	void ItemManager::disableItem(Item *i, int disableTime)
	{
		i->setEnabled(false);
		i->setReEnableTime(disableTime);

		if(i->spawner)
			queueForSpawn(i->spawner, false);

		ItemType *it = getItemTypeById(i->getItemTypeId());
		if (it != NULL)
		{
			ItemType::DISABLE_EFFECT eff = it->getDisableEffect();
			if (eff == ItemType::DISABLE_EFFECT_DISAPPEAR)
			{
				i->getVisualObject()->setVisible(false);
			}
			else if (eff == ItemType::DISABLE_EFFECT_NOBLINK)
			{
				i->setBlinking(false);
			}
		} else {
			fb_assert(!"ItemManager::disableItem - Null item type encountered.");
		}
	}


	int ItemManager::getItemIdByName(const char *itemname)
	{
		fb_assert(itemname != NULL);

		if (itemTypes == NULL)
			loadItemTypes();		

		// TODO: this is not very effective, but on the other hand
		// this is not meant to be called very often.
		for (int i = 0; i < MAX_ITEM_TYPES; i++)
		{
			if (itemTypes[i].getName() != NULL
				&& strcmp(itemTypes[i].getName(), itemname) == 0)
				return i;
		}

		Logger::getInstance()->warning("ItemManager::getItemIdByName - No item type with given name.");
		Logger::getInstance()->debug(itemname);

		return -1;
	}


	void ItemManager::run()
	{
		LinkedList *ilist = game->items->getAllItems();
		// NOTE: need to use safe iterator, as running an item script may
		// delete it.
		SafeLinkedListIterator iter(ilist);
		while(iter.iterateAvailable())
		{
			Item *item = (Item *)iter.iterateNext();

			if (item->advanceReEnable())
			{
				// the item re-enabled, remove the disable effect.
				ItemType *it = getItemTypeById(item->getItemTypeId());
				if (it != NULL)
				{
					ItemType::DISABLE_EFFECT eff = it->getDisableEffect();
					if (eff == ItemType::DISABLE_EFFECT_DISAPPEAR)
					{
						item->getVisualObject()->setVisible(true);
					}
					else if (eff == ItemType::DISABLE_EFFECT_NOBLINK)
					{
						if (it->getBlinking())
						{
							item->setBlinking(true);
						}
					}
				}
			}

			/*
			// executable items ignore any pickup delays if
			// execute is being requested...
			if ((game->gameTimer & (ITEM_CHECK_INTERVAL-1)) == 0)
			{
				if (executeUnit != NULL)
				{
					ItemType *it = getItemTypeById(item->getItemTypeId());
					fb_assert(it != NULL);
					if (it != NULL)
					{
						if (it->isExecutable())
						{
							item->setPickupDelay(0);
						}
					}
				}
			}
			*/

			// progress bar handling...
			if (item->getProgressBar() != NULL)
			{
				Unit *unit = game->gameUI->getFirstPerson(0);
				if (item->getProgressBar()->getUnit() != NULL)
				{
					unit = item->getProgressBar()->getUnit();
				}

				if (unit != NULL)
				{
					ProgressBarActor pba(game, item->getProgressBar(), item, unit);
					pba.run(unit->getPosition(), unit->getRotation().y);
				}
			}


			// execute tips
			if (item->isEnabled())
			{
				// TODO: balance load properly.
				if ((game->gameTimer & (ITEM_CHECK_INTERVAL-1)) == (ITEM_CHECK_INTERVAL / 2))
				{
					ItemType *it = getItemTypeById(item->getItemTypeId());
					if (it->isExecutable() && (item->getCustomTipText() || it->getTipText() != NULL))
					{

						LinkedList *ulist = game->units->getOwnedUnits(game->singlePlayerNumber);
						LinkedListIterator uiter(ulist);
						while (uiter.iterateAvailable())
						{
							Unit *u = (Unit *)uiter.iterateNext();

							if (u->isActive() && !u->isDestroyed())
							{
								float checkRangeSq = ITEM_EXECUTE_RANGE * ITEM_EXECUTE_RANGE;

								// note: new behaviour, now cylinderic check, not spheric
								// note: cylinder height is -1/2*height to +1*height
								VC3 posdiff = u->getPosition() - item->getPosition();
								float heightdiff = posdiff.y;
								posdiff.y = 0;
								if (posdiff.GetSquareLength() < checkRangeSq
									&& heightdiff > -ITEM_PICKUP_HEIGHT
									&& heightdiff < ITEM_PICKUP_HEIGHT / 2.0f)
								{
									const char *txt = it->getTipText();
									if (item->getCustomTipText() != NULL)
									{
										txt = item->getCustomTipText();
									}
									game->gameUI->gameMessage(getLocaleGuiString(txt), NULL, it->getTipPriority(), 700, GameUI::MESSAGE_TYPE_EXECUTE_TIP);
									break;
								}
							}
						}
					}
				}
			}

#ifdef PHYSICS_INUSE
			// hax hax physh-x
			if( game::SimpleOptions::getBool( DH_OPT_B_PHYSICS_UPDATE )  )
			{
				if( item->getGamePhysicsObject() )
				{
					// BUGBUG: status fixed?
					item->setPosition( item->getGamePhysicsObject()->getPosition() );
					// item->setRotation( RAD2DEG( VC3( item->getGamePhysicsObject()->getRotation().x, item->getGamePhysicsObject()->getRotation().y, item->getGamePhysicsObject()->getRotation().z ) ) );
					// item->setRotation( RAD2DEG( EulerAnglesSuperHack( item->getGamePhysicsObject()->getRotation() ) )  );
					if( item->getVisualObject() )
					{
						item->getVisualObject()->setRotationQuaternion( item->getGamePhysicsObject()->getRotation() );
					}
				}
				// item->setPosition( item->getPosition() + VC3(0.001f,0,0 ) );
			}
#endif
			// run pickups...
			if (item->getPickupDelay() > 0)
			{
				item->setPickupDelay(item->getPickupDelay() - 1);
			} else {
				if (item->isEnabled())
				{
					// TODO: balance load properly.
					if ((game->gameTimer & (ITEM_CHECK_INTERVAL-1)) == 0)
					{
						LinkedList *ulist = game->units->getOwnedUnits(game->singlePlayerNumber);
						LinkedListIterator uiter(ulist);
						while (uiter.iterateAvailable())
						{
							Unit *u = (Unit *)uiter.iterateNext();

							if (u->isActive() && !u->isDestroyed())
							{
								/*
								bool runExecuteInstead = false;
								if (executeUnit == u)
								{
									ItemType *it = getItemTypeById(item->getItemTypeId());
									fb_assert(it != NULL);
									if (it != NULL)
									{
										if (it->isExecutable())
										{
											runExecuteInstead = true;
										}
									}
								}
								*/

								float checkRangeSq = ITEM_PICKUP_RANGE * ITEM_PICKUP_RANGE;
								/*
								if (runExecuteInstead)
									checkRangeSq = ITEM_EXECUTE_RANGE * ITEM_EXECUTE_RANGE;
								*/

								// note: new behaviour, now cylinderic check, not spheric
								// note: cylinder height is -1/2*height to +1*height
								VC3 posdiff = u->getPosition() - item->getPosition();
								float heightdiff = posdiff.y;
								posdiff.y = 0;
								if (posdiff.GetSquareLength() < checkRangeSq
									&& heightdiff > -ITEM_PICKUP_HEIGHT
									&& heightdiff < ITEM_PICKUP_HEIGHT / 2.0f)
								{
									item->setPickupDelay(ITEM_PICKUP_DELAY);
									game->gameScripting->runItemPickupScript(u, item);

									// TODO: should clear tip message only!
									//game->gameUI->clearGameMessage();

									// cannot check any further units, as the item may
									// have been deleted by the pickup script...
									break;
								}
							}
						}
					}
				}
			}
		}



		/*
		if ((game->gameTimer & (ITEM_CHECK_INTERVAL-1)) == 0)
		{
			// clear execute unit now that should have run the execute
			// for any items in question...
			executeUnit = NULL;
		}
		*/

		runSpawners();
	}


	Item *ItemManager::getNearestItemOfType(const VC3 &position, ItemType *itemType)
	{
		int itemTypeId = -1;

		if (itemType == NULL)
		{
			Logger::getInstance()->warning("ItemManager::getNearestItemOfType - Null itemType parameter given.");	
			fb_assert(!"ItemManager::getNearestItemOfType - Null itemType parameter given.");
			return NULL;
		}

		for (int i = 0; i < MAX_ITEM_TYPES; i++)
		{
			if (&itemTypes[i] == itemType)
			{
				itemTypeId = i;
				break;
			}
		}

		if (itemTypeId == -1)
		{
			Logger::getInstance()->warning("ItemManager::getNearestItemOfType - Failed to solve proper id for given itemType (internal error?).");
			fb_assert(!"ItemManager::getNearestItemOfType - Failed to solve proper id for given itemType (internal error?).");
			return NULL;
		}


		float closestRangeSq = 999999.0f;
		Item *closestItem = NULL;

		LinkedList *ilist = game->items->getAllItems();
		LinkedListIterator iter(ilist);
		while(iter.iterateAvailable())
		{
			Item *item = (Item *)iter.iterateNext();
			if (item->getItemTypeId() == itemTypeId)
			{
				VC3 diffVec = item->getPosition() - position;
				float rangeSq = diffVec.GetSquareLength();

				if (closestItem == NULL || rangeSq < closestRangeSq)
				{
					closestItem = item;
					closestRangeSq = rangeSq;
				}
			}
		}

		return closestItem;
	}


	void ItemManager::prepareForRender()
	{
		float modLighting = float(SimpleOptions::getInt(DH_OPT_I_ITEM_MIN_LIGHTING)) / 100.0f;

		LinkedList *ilist = game->items->getAllItems();
		LinkedListIterator iter(ilist);
		while(iter.iterateAvailable())
		{
			Item *item = (Item *)iter.iterateNext();

			if(game->gameMap->colorMap)
			{				
				VisualObject *vo = item->getVisualObject();
				if (vo != NULL)
				{
					GameMap *gameMap = game->gameMap;
					VC3 position = item->getPosition();

					position.x = position.x / gameMap->getScaledSizeX() + .5f;
					position.z = position.z / gameMap->getScaledSizeY() + .5f;

					COL color = gameMap->colorMap->getColor(position.x, position.z);

					if (color.r < modLighting) color.r = modLighting;
					if (color.g < modLighting) color.g = modLighting;
					if (color.b < modLighting) color.b = modLighting;

					if (item->isBlinking())
					{
						long time = Timer::getTime();
						color.r += 0.3f + sinf((float)time / 350.0f) / 5.f;
						color.g += 0.3f + sinf((float)time / 350.0f) / 5.f;
						color.b += 0.3f + sinf((float)time / 350.0f) / 5.f;
						if (color.r < 0) color.r = 0;
						if (color.r > 1.0f) color.r = 1.0f;
						if (color.g < 0) color.g = 0;
						if (color.g > 1.0f) color.g = 1.0f;
						if (color.b < 0) color.b = 0;
						if (color.b > 1.0f) color.b = 1.0f;
					}
					vo->setSelfIllumination(color);
				}
			}

			item->prepareForRender();
		}
	}


	void ItemManager::loadItemTypes()
	{
		// NOTE: not thread safe.

		Logger::getInstance()->debug("ItemManager - About to load item types.");

		// read in item types
		itemTypes = new ItemType[MAX_ITEM_TYPES];
		util::SimpleParser sp;
#ifdef LEGACY_FILES
		if (sp.loadFile("Data/Items/itemtypes.txt"))
#else
		if (sp.loadFile("data/item/itemtypes.txt"))
#endif
		{
			int atId = 0;
			bool insideItem = false;
			while (sp.next())
			{
				bool lineok = false;
				char *k = sp.getKey();
				if (k != NULL)
				{
					if (!insideItem)
						sp.error("ItemManager - Parse error, key=value pair outside item block.");
					char *v = sp.getValue();
					// treat empty lines as null...
					if (v[0] == '\0') v = NULL;

					if (strcmp(k, "name") == 0)
					{
						if (v != NULL)
						{
							for (int i = 0; i < atId; i++)
							{
								if (itemTypes[i].getName() != NULL
								  && strcmp(v, itemTypes[i].getName()) == 0)
								{
									sp.error("ItemManager - Duplicate item name.");
									break;
								}
							}
						}
						itemTypes[atId].setName(v);
						lineok = true;
					}
					if (strcmp(k, "model") == 0)
					{
						itemTypes[atId].setModelFilename(v);
						lineok = true;
					}
					if (strcmp(k, "weapon") == 0)
					{
						if (v != NULL && str2int(v) == 1)
						{
							itemTypes[atId].setWeaponType(true);
						} else {
							itemTypes[atId].setWeaponType(false);
						}
						lineok = true;
					}
					if (strcmp(k, "executable") == 0)
					{
						if (v != NULL && str2int(v) == 1)
						{
							itemTypes[atId].setExecutable(true);
						} else {
							itemTypes[atId].setExecutable(false);
						}
						lineok = true;
					}
					if (strcmp(k, "disable_effect") == 0)
					{
						if (v != NULL)
						{
							if (strcmp(v, "noblink") == 0)
							{
								itemTypes[atId].setDisableEffect(ItemType::DISABLE_EFFECT_NOBLINK);
								lineok = true;
							}
							if (strcmp(v, "none") == 0)
							{
								itemTypes[atId].setDisableEffect(ItemType::DISABLE_EFFECT_NONE);
								lineok = true;
							}
							if (strcmp(v, "disappear") == 0)
							{
								itemTypes[atId].setDisableEffect(ItemType::DISABLE_EFFECT_DISAPPEAR);
								lineok = true;
							}
						}
					}
					if (strcmp(k, "halo") == 0)
					{
						itemTypes[atId].setHalo(v);
						lineok = true;
					}
					if (strcmp(k, "script") == 0)
					{
						itemTypes[atId].setScript(v);
						lineok = true;
					}
					if (strcmp(k, "tiptext") == 0)
					{
						itemTypes[atId].setTipText(v);
						lineok = true;
					}
					if (strcmp(k, "tippriority") == 0)
					{
						if (v != NULL)
						{
							itemTypes[atId].setTipPriority(str2int(v));
						}
						lineok = true;
					}
					if (strcmp(k, "highlightstyle") == 0)
					{
						if (v != NULL)
						{
							itemTypes[atId].setHighlightStyle(str2int(v));
						}
						lineok = true;
					}
					if (strcmp(k, "highlighttext") == 0)
					{
						if (v != NULL )
						{
							itemTypes[atId].setHighlightText(v);
						}
						lineok = true;
					}
					if (strcmp(k, "blinking") == 0)
					{
						if (v != NULL )
						{
							bool value = str2int(v)?true:false;
							itemTypes[atId].setBlinking( value );
						}
						lineok = true;
					}
					// added by pete
					if (strcmp(k, "physics_enabled") == 0)
					{
						if (v != NULL )
						{
							bool value = str2int(v)?true:false;
							itemTypes[atId].setPhysicsEnabled( value );
						}
						lineok = true;
					}
					if (strcmp(k, "physics_mass") == 0)
					{
						if (v != NULL )
						{
							float value = 1.0f;
							std::stringstream( v ) >> value;
							itemTypes[atId].setPhysicsMass( value );
						}
						lineok = true;
					}
					
				} else {
					char *l = sp.getLine();
					if (strcmp(l, "item") == 0)
					{
						if (insideItem)
							sp.error("ItemManager - Parse error, } expected.");
						if (atId < MAX_ITEM_TYPES - 1)
						{
							atId++;
						} else {
							sp.error("ItemManager - Too many items, limit reached.");
						}
						lineok = true;
					}
					if (strcmp(l, "{") == 0)
					{
						if (insideItem)
							sp.error("ItemManager - Parse error, unexpected {.");
						insideItem = true;
						lineok = true;
					}
					if (strcmp(l, "}") == 0)
					{
						if (!insideItem)
							sp.error("ItemManager - Parse error, unexpected }.");
						insideItem = false;
						lineok = true;
					}
				}
				if (!lineok)
				{
					sp.error("ItemManager - Unknown command or bad key/value pair.");
				}
			}				
		}	else {
			Logger::getInstance()->error("ItemManager - Failed to load item types.");
		}		
	}


	void ItemManager::unloadItemTypes()
	{
		delete [] itemTypes;
		itemTypes = NULL;
	}


	/*
	void ItemManager::setExecuteUnit(Unit *unit)
	{
		fb_assert(unit != NULL);
		fb_assert(unit->getOwner() == game->singlePlayerNumber);

		// WARNING: this unit pointer is "latched", that is, takes
		// effect some ticks later on, thus, may be invalid by the
		// time we get there!
		// therefore, the executeUnit pointer should never be directly
		// used to access the unit, just to compare if some given unit		
		// pointer matches that pointer.

		this->executeUnit = unit;
	}
	*/

	bool ItemManager::doItemExecute(Unit *unit)
	{
		if (unit == NULL)
			return false;

		bool didExecute = false;

		LinkedList *ilist = game->items->getAllItems();
		// NOTE: need to use safe iterator, as running an item script may
		// delete it.
		SafeLinkedListIterator iter(ilist);
		while(iter.iterateAvailable())
		{
			Item *item = (Item *)iter.iterateNext();

			// run executes...
			if (item->isEnabled())
			{
				//LinkedList *ulist = game->units->getOwnedUnits(game->singlePlayerNumber);
				//LinkedListIterator uiter(ulist);
				//while (uiter.iterateAvailable())
				//{
				//	Unit *u = (Unit *)uiter.iterateNext();
					Unit *u = unit;

					if (u->isActive() && !u->isDestroyed())
					{
						float checkRangeSq = ITEM_EXECUTE_RANGE * ITEM_EXECUTE_RANGE;

						// note: new behaviour, now cylinderic check, not spheric
						// note: cylinder height is -1/2*height to +1*height
						VC3 posdiff = u->getPosition() - item->getPosition();
						float heightdiff = posdiff.y;
						posdiff.y = 0;
						if (posdiff.GetSquareLength() < checkRangeSq
							&& heightdiff > -ITEM_PICKUP_HEIGHT
							&& heightdiff < ITEM_PICKUP_HEIGHT / 2.0f)
						{
							bool success = game->gameScripting->runItemExecuteScript(u, item);
							if (success)
								didExecute = true;

							// cannot check any further units, as the item may
							// have been deleted by the pickup script...
							if (success)
								break;
						}
					}
				//}
			}
		}

		return didExecute;
	}

	void ItemManager::deleteAllLongTimeDisabledItems()
	{
		LinkedList *ilist = game->items->getAllItems();
		// NOTE: need to use safe iterator, as this may delete items
		SafeLinkedListIterator iter(ilist);
		while(iter.iterateAvailable())
		{
			Item *item = (Item *)iter.iterateNext();

			// run executes...
			if (!item->isEnabled())
			{
				// more than 24h (is this formula correct?)
				if (item->getReEnableTime() > GAME_TICKS_PER_SECOND*60*60*24)
				{
					this->deleteItem(item);
				}
			}
		}
	}

	void ItemManager::reEnableAllTimeDisabledItems()
	{
		LinkedList *ilist = game->items->getAllItems();
		LinkedListIterator iter(ilist);
		while(iter.iterateAvailable())
		{
			Item *item = (Item *)iter.iterateNext();

			// run executes...
			if (!item->isEnabled())
			{
				if (item->getReEnableTime() > 0)
				{
					item->setReEnableTime(1);
				}
			}
		}
	}

	ItemSpawnerGroup *ItemManager::getSpawnerGroup(const char *group_name)
	{
		for(unsigned int i = 0; i < spawnerGroups.size(); i++)
		{
			if(spawnerGroups[i]->name == group_name)
			{
				return spawnerGroups[i];
			}
		}
		return NULL;
	}

	ItemSpawnerGroup *ItemManager::createSpawnerGroup(const char *group_name)
	{
		ItemSpawnerGroup *group = new ItemSpawnerGroup();
		group->name = group_name;
		group->active = false;
		spawnerGroups.push_back(group);
		return group;
	}

	ItemSpawner *ItemManager::createSpawner(const char *group_name, int item_id, const std::string &item_name, int respawn_time, const VC3 &position, float radius)
	{
		ItemSpawnerGroup *group = getSpawnerGroup(group_name);
		if(group == NULL)
		{
			group = createSpawnerGroup(group_name);
		}

		ItemSpawner *spawner = new ItemSpawner();
		spawner->item_id = item_id;
		spawner->item_name = item_name;
		spawner->respawn_time = respawn_time;
		spawner->position = position;
		spawner->next_respawn = 0;
		spawner->spawned_item = NULL;
		spawner->radius = radius;
		spawner->group = group;
		group->spawners.push_back(spawner);
		queueForSpawn(spawner, true);
		return spawner;
	}

	inline bool ItemSpawnerCompare(const ItemSpawner *a, const ItemSpawner *b)
	{
		return a->next_respawn > b->next_respawn;
	}

	void ItemManager::clearAllSpawners(void)
	{
		for(unsigned int i = 0; i < spawnerGroups.size(); i++)
		{
			const std::vector<ItemSpawner *> &spawners = spawnerGroups[i]->spawners;
			for(unsigned int j = 0; j < spawners.size(); j++)
			{
				delete spawners[j];
			}
			delete spawnerGroups[i];
		}
		spawnerGroups.clear();
	}

	void ItemManager::queueForSpawn(ItemSpawner *spawner, bool instantly)
	{
		// insert to spawn queue
		spawner->next_respawn = game->gameTimer;
		if(!instantly)
		{
			spawner->next_respawn += spawner->respawn_time;
		}
		spawner->group->spawnQueue.push_back(spawner);
		std::push_heap(spawner->group->spawnQueue.begin(), spawner->group->spawnQueue.end(), ItemSpawnerCompare);
	}

	void ItemManager::setSpawnerGroupActive(ItemSpawnerGroup *group, bool active)
	{
		group->active = active;

		if(!active)
		{
			group->spawnQueue.clear();
		}
		else
		{
			const std::vector<ItemSpawner *> &spawners = group->spawners;

			// add all spawners to queue
			group->spawnQueue.resize(spawners.size());
			for(unsigned int i = 0; i < spawners.size(); i++)
			{
				// spawn instantly
				spawners[i]->next_respawn = game->gameTimer;
				group->spawnQueue[i] = spawners[i];
			}
			// no need to make heap since all have the same spawn time
			//std::make_heap(group->spawnQueue.begin(), group->spawnQueue.end(), ItemSpawnerCompare);
		}
	}

	void ItemManager::spawnItem(ItemSpawner *spawner)
	{
		VC3 pos;
		
		// try different positions
		for(int i = 0; i < 10; i++)
		{
			pos = spawner->position;
			pos.x += 2 * spawner->radius * game->gameRandom->nextFloat() - spawner->radius;
			pos.z += 2 * spawner->radius * game->gameRandom->nextFloat() - spawner->radius;

			// found one that isn't blocked
			game->gameMap->keepWellInScaledBoundaries(&pos.x, &pos.z);
			if(!game->gameScene->isBlockedAtScaled(pos.x, pos.z, pos.y + 1.0f))
			{
				break;
			}
		}

		// create new item
		if(spawner->spawned_item == NULL)
		{
			spawner->spawned_item = createNewItem(spawner->item_id, pos);
			if (spawner->spawned_item != NULL)
			{
				spawner->spawned_item->spawner = spawner;

#ifndef PHYSICS_NONE
				if( game::SimpleOptions::getBool( DH_OPT_B_PHYSICS_ENABLED ) ) 
				{
					enablePhysics( spawner->spawned_item, spawner->item_id );
				}
#endif
			}
		}
		// or re-enable the old one
		else
		{
			spawner->spawned_item->spawner = spawner;
			spawner->spawned_item->setReEnableTime(1);
			spawner->spawned_item->setPosition(pos);
		}
	}

	void ItemManager::runSpawners()
	{
		// for each group
		unsigned int num_groups = spawnerGroups.size();
		for(unsigned int i = 0; i < num_groups; i++)
		{
			if(!spawnerGroups[i]->active)
				continue;

			std::vector<ItemSpawner *> &spawnQueue = spawnerGroups[i]->spawnQueue;

			// get next spawner in queue
			while(!spawnQueue.empty())
			{
				// until found one that is not queued for spawn yet
				ItemSpawner *spawner = spawnQueue[0];
				if(spawner->next_respawn > game->gameTimer)
					break;

				spawnItem(spawner);
				std::pop_heap(spawnQueue.begin(), spawnQueue.end(), ItemSpawnerCompare);
				spawnQueue.pop_back();
			}
		}
	}
}

