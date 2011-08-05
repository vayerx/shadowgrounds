
#include "precompiled.h"

#include "UnitHighlight.h"

#include <Storm3D_UI.h>
#include "../game/Unit.h"
#include "../game/UnitType.h"
#include "VisualObjectModel.h"
#include "VisualObject.h"


using namespace game;

namespace ui
{
	UnitHighlight::UnitHighlight(IStorm3D *storm3D, IStorm3D_Scene *stormScene)
	{
		this->storm3D = storm3D;
		this->stormScene = stormScene;
		highlightUnit = NULL;
		highlightTerrain = false;
		VC3 highlightTerrainPosition = VC3(0,0,0);

#ifdef LEGACY_FILES
		visualObjectModel = new VisualObjectModel("Data/Models/Pointers/highlight.s3d");
#else
		visualObjectModel = new VisualObjectModel("data/model/pointer/highlight.s3d");
#endif

		// NOTICE: actually setting vo1 effect also sets it for vo2,
		// as no vo->disableSharedObjects() is called. 
		// (they have shared meshes)

		visualObject = visualObjectModel->getNewObjectInstance();
		visualObject->setCollidable(false);
		visualObject->setVisible(false);
		visualObject->setEffect(VISUALOBJECTMODEL_EFFECT_ADDITIVE);
		visualObject->setEffect(VISUALOBJECTMODEL_EFFECT_TRANSPARENT_30);

		visualObject2 = visualObjectModel->getNewObjectInstance();
		visualObject2->setCollidable(false);
		visualObject2->setVisible(false);
		visualObject2->setEffect(VISUALOBJECTMODEL_EFFECT_ADDITIVE);
		visualObject2->setEffect(VISUALOBJECTMODEL_EFFECT_TRANSPARENT_30);

#ifdef LEGACY_FILES
		terrainVisualObjectModel = new VisualObjectModel("Data/Models/Pointers/highlight_terrain.s3d");
#else
		terrainVisualObjectModel = new VisualObjectModel("data/model/pointer/highlight_terrain.s3d");
#endif
		
		terrainVisualObject = terrainVisualObjectModel->getNewObjectInstance();
		terrainVisualObject->setEffect(VISUALOBJECTMODEL_EFFECT_ADDITIVE);
		terrainVisualObject->setEffect(VISUALOBJECTMODEL_EFFECT_TRANSPARENT_30);
		terrainVisualObject->setCollidable(false);
		terrainVisualObject->setVisible(false);

		terrainVisualObject2 = terrainVisualObjectModel->getNewObjectInstance();
		terrainVisualObject2->setEffect(VISUALOBJECTMODEL_EFFECT_ADDITIVE);
		terrainVisualObject2->setEffect(VISUALOBJECTMODEL_EFFECT_TRANSPARENT_30);
		terrainVisualObject2->setCollidable(false);
		terrainVisualObject2->setVisible(false);

		rotX = 0;
		rotY = 0;
		rotZ = 0;
	}


	UnitHighlight::~UnitHighlight()
	{
		delete visualObject2;
		delete visualObject;
		delete visualObjectModel;

		delete terrainVisualObject2;
		delete terrainVisualObject;
		delete terrainVisualObjectModel;
	}


	void UnitHighlight::setHighlightedTerrain(VC3 &position)
	{
		// DISABLED!
		return;

		if (highlightUnit != NULL)
			setHighlightedUnit(NULL);

		// no change?
		if (highlightTerrain
			&& position.x == highlightTerrainPosition.x
			&& position.y == highlightTerrainPosition.y
			&& position.z == highlightTerrainPosition.z)
			return;

		if (!highlightTerrain)
		{
			// no old highlight object. add a new one.
			terrainVisualObject->setInScene(true);
			terrainVisualObject->setVisible(true);
			terrainVisualObject2->setVisible(true);
			terrainVisualObject2->setInScene(true);
		}

		highlightTerrain = true;
		highlightTerrainPosition = position;
	}


	void UnitHighlight::clearHighlightedTerrain()
	{
		if (highlightUnit != NULL)
			setHighlightedUnit(NULL);

		// no change?
		if (!highlightTerrain)
			return;

		if (highlightTerrain)
		{
			// remove old highlight object. no new object.
			terrainVisualObject->setVisible(false);
			terrainVisualObject->setInScene(false);
			terrainVisualObject2->setVisible(false);
			terrainVisualObject2->setInScene(false);
		}
		highlightTerrain = false;
	}


	void UnitHighlight::setHighlightedUnit(const game::Unit *unit)
	{		
		// DISABLED!
		return;

		if (highlightTerrain)
			clearHighlightedTerrain();

		// no change?
		if (unit == highlightUnit)
			return;

		if (highlightUnit != NULL && unit == NULL)
		{
			// remove old highlight object. no new object.
			visualObject->setVisible(false);
			visualObject->setInScene(false);
			visualObject2->setVisible(false);
			visualObject2->setInScene(false);
		}
		if (highlightUnit == NULL && unit != NULL)
		{
			// no old highlight object. add a new one.
			visualObject->setInScene(true);			
			visualObject->setVisible(true);
			visualObject2->setVisible(true);
			visualObject2->setInScene(true);
		}
		highlightUnit = unit;

		// rescale the highlight to match the unit's size...
		/*
		if (unit != NULL)
		{
			float s = unit->getUnitType()->getSize();
			VC3 scale = VC3(s, s, s);
			visualObject->setScale(scale);
			visualObject2->setScale(scale);
		}
		*/
	}


	void UnitHighlight::run()
	{
		if (highlightUnit != NULL)
		{
			rotY += 0.2f;
			if (rotY >= 360.0f) rotY -= 360.0f;
			// rise the object up just a bit to prevent blinking...
			// (center above the friendlytorus center)
			VC3 tmp = highlightUnit->getPosition();
			tmp.y += 0.05f;
			visualObject->setPosition(tmp);
			visualObject->setRotation(rotX, rotY, rotZ);
			visualObject2->setPosition(tmp);
			visualObject2->setRotation(rotX, (360.0f-rotY) * 2, rotZ);
			visualObject->prepareForRender();
			visualObject2->prepareForRender();
		}		
		if (highlightTerrain)
		{
			rotY += 0.2f;
			if (rotY >= 360.0f) rotY -= 360.0f;
			terrainVisualObject->setPosition(highlightTerrainPosition);
			terrainVisualObject->setRotation(rotX, rotY, rotZ);
			terrainVisualObject2->setPosition(highlightTerrainPosition);
			terrainVisualObject2->setRotation(rotX, (360.0f-rotY) * 2, rotZ);
			terrainVisualObject->prepareForRender();
			terrainVisualObject2->prepareForRender();
		}		
	}
			
}

