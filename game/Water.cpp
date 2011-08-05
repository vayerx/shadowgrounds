
#include "precompiled.h"

#include <assert.h>
#include "../ui/VisualObject.h"
#include "../ui/VisualObjectModel.h"

#include "../ui/Decoration.h"

#include "Water.h"

namespace game
{

Water::Water() 
{

	minY = 0.0f;
	maxY = 0.0f;
	minX = 0.0f;
	maxX = 0.0f;
	height = 0.0f;
	name = NULL;
	position = Vector(0.0f, 0.0f, 0.0f);
	decoration = NULL;

}

Water::~Water() 
{
}

void Water::setName(const char *name) 
{
	// helou!!!
	// with stl string this could be simply: this->name = name; :)
	if (this->name != NULL)
	{
		delete [] this->name;
		this->name = NULL;
	}
	if (name != NULL)
	{
		this->name = new char[strlen(name) + 1];
		strcpy(this->name, name);
	}
}

void Water::setDecoration(ui::Decoration *decor) 
{
	decoration = decor;
	updateBoundaries();
}

void Water::setHeight(float height) 
{
	this->height = height;
	updateBoundaries();
}

void Water::setPosition(const VC3 &position) 
{
	this->position.x = position.x;
	this->position.z = position.z;
	updateBoundaries();
}

// based on current position and decoration...
void Water::updateBoundaries() 
{	
	if(decoration) {
		float sizeX, sizeY;
		decoration->getBoundingQuadSize(&sizeX, &sizeY);
		decoration->setPosition(position);
		decoration->setHeight(height);
		minX = position.x - sizeX * 0.5f;
		maxX = position.x + sizeX * 0.5f;
		minY = position.z - sizeY * 0.5f;
		maxY = position.z + sizeY * 0.5f;		
	}
}


} // ui
