#ifndef INCLUDED_BUILDINGHANDLER_H
#define INCLUDED_BUILDINGHANDLER_H

class IStorm3D_Model;

namespace frozenbyte {

struct BuildingHandlerData;

class BuildingHandler
{
	BuildingHandlerData *data;

public:
	BuildingHandler();
	~BuildingHandler();

	// Clears previous frame
	void beginUpdate(); 
	// Call between update
	void removeTopFrom(IStorm3D_Model *model); 
	// Update models which have changes
	void endUpdate(bool noDelay = false); 

	// Clear all models
	void clear();

	void removeAllTops(); 

	// To help with cursor raytracing
	void setCollisions(bool collisionOn);
	// Add every building here on load
	void addBuilding(IStorm3D_Model *model);
	void removeBuilding(IStorm3D_Model *model);

	void setUpdateEnabled(bool updateEnabled);
	void hideAllRoofs();
	void showAllRoofs();

};

} // end of namespace frozenbyte

#endif
