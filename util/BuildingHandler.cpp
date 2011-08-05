
#include "precompiled.h"

#ifdef _MSC_VER
#pragma warning(disable:4103)
#pragma warning(disable:4786)
#endif

#include "BuildingHandler.h"
#include <vector>
#include <Storm3D_UI.h>

#include "../util/fb_assert.h"
#include "../util/Debug_MemoryManager.h"


// 100 ms * 50 = 5 secs
#define BUILDINGHANDLER_REMOVED_DELAY 50 


namespace frozenbyte {

struct BuildingHandlerData
{
	// All models
	std::vector<IStorm3D_Model *> buildingModels;
	
	// Models with top removed (from last frame)
	// modified to contain timeout counter too. -jpk
	std::vector<std::pair<IStorm3D_Model *, int> > removedModels;

	// Models to remove top from (this frame)
	std::vector<IStorm3D_Model *> removableModels;

	bool updateEnabled;

	BuildingHandlerData()
	{
		updateEnabled = true;
	}

	void setVisibility(IStorm3D_Model *model, bool visible)
	{
		if(!model)
			return;

		Iterator<IStorm3D_Model_Object *> *objectIterator;
		for(objectIterator = model->ITObject->Begin(); !objectIterator->IsEnd(); objectIterator->Next())
		{
			IStorm3D_Model_Object *object = objectIterator->GetCurrent();
			const char *name = object->GetName();

			int namelen = strlen(name);

			if(namelen < 12)
				continue;

			// Test name tag
			for(int i = 0; i < namelen - 12 + 1; ++i)
			{
				if (strncmp(&name[i], "BuildingRoof", 12) == 0)
				{
					if(visible == false)
						object->SetNoRender(true);
					else
						object->SetNoRender(false);
					//break;
				}
			}

		}

		delete objectIterator;
	}
};

BuildingHandler::BuildingHandler()
{
	data = new BuildingHandlerData();
}

BuildingHandler::~BuildingHandler()
{
	delete data;
}

void BuildingHandler::clear()
{
	data->buildingModels.clear();
	data->removedModels.clear();
	data->removableModels.clear();
}

void BuildingHandler::beginUpdate()
{
	if (!data->updateEnabled)
		return;

	data->removableModels.clear();
}

void BuildingHandler::removeTopFrom(IStorm3D_Model *model)
{
	if (!data->updateEnabled)
		return;

	if(std::find(data->removableModels.begin(), data->removableModels.end(), model) == data->removableModels.end())
		data->removableModels.push_back(model);
}

void BuildingHandler::removeAllTops()
{
	for(unsigned int i = 0; i < data->buildingModels.size(); ++i)
	{
		IStorm3D_Model *model = data->buildingModels[i];
		removeTopFrom(model);
	}
}

void BuildingHandler::endUpdate(bool noDelay)
{
	if (!data->updateEnabled)
		return;

	// modified to handle timeout counter too for removed buildings. 
	// -jpk

	unsigned int remModSize = data->removedModels.size();
	std::vector<IStorm3D_Model *> insertModelsList;

	for(unsigned int i = 0; i < data->removableModels.size(); ++i)
	{
		// If already removed
		IStorm3D_Model *model = data->removableModels[i];

		bool alreadyRemoved = false;
		for (unsigned int j = 0; j < remModSize; j++)
		{
			if (data->removedModels[j].first == model)
			{
				data->removedModels[j].second = BUILDINGHANDLER_REMOVED_DELAY;

				alreadyRemoved = true;
				break;
			}
		}

		// Set visibility to false
		if (!alreadyRemoved)
		{
			data->setVisibility(model, false);
			insertModelsList.push_back(model);
		}
	}

	// Set visibility back to models which are left
	std::vector<std::pair<IStorm3D_Model *, int> >::iterator it = data->removedModels.begin();

	//for(unsigned int j = 0; j < data->removedModels.size(); ++j)
	//{
	while (it != data->removedModels.end()) 
	{
		// If already removed
		IStorm3D_Model *model = (*it).first;
		
		if ((*it).second > 0 && !noDelay)
		{
			(*it).second--;
			++it;
		} 
		else 
		{
			data->setVisibility(model, true);
			it = data->removedModels.erase(it);
		}
	}

	// Add newly removed building roofs to list of removed
	for(unsigned int r = 0; r < insertModelsList.size(); ++r)
	{
		IStorm3D_Model *model = insertModelsList[r];
		data->removedModels.push_back(std::pair<IStorm3D_Model *, int>(model, BUILDINGHANDLER_REMOVED_DELAY));
	}

	//data->removedModels.swap(data->removableModels);
}

void BuildingHandler::setCollisions(bool collision)
{
	for(unsigned int i = 0; i < data->buildingModels.size(); ++i)
	{
		IStorm3D_Model *model = data->buildingModels[i];
		if(collision == true)
			model->SetNoCollision(false);
		else
			model->SetNoCollision(true);
	}
}

void BuildingHandler::addBuilding(IStorm3D_Model *model)
{
	data->buildingModels.push_back(model);

	// ..also get rid of unwanted helper model visibility...
	// -jpk
	Iterator<IStorm3D_Model_Object *> *objectIterator;
	for(objectIterator = model->ITObject->Begin(); !objectIterator->IsEnd(); objectIterator->Next())
	{
		IStorm3D_Model_Object *object = objectIterator->GetCurrent();
		const char *name = object->GetName();

		int namelen = strlen(name);

		if(namelen < 19)
			continue;

		// Test name tag
		for(int i = 0; i < namelen - 19 + 1; ++i)
		{
			if (strncmp(&name[i], "_WindowShootThrough", 19) == 0)
			{
				object->SetNoRender(true);
				//break;
			}
		}

	}

	delete objectIterator;
}

void BuildingHandler::removeBuilding(IStorm3D_Model *model)
{
	for(unsigned int i = 0; i < data->buildingModels.size(); ++i)
	{
		if(model == data->buildingModels[i])
		{
			data->buildingModels.erase(data->buildingModels.begin() + i);
			return;
		}
	}
}

void BuildingHandler::setUpdateEnabled(bool updateEnabled)
{
	if (updateEnabled && !data->updateEnabled)
	{
		// TODO: do something?? (to ensure immediate restoration of roof hiddeness)
		// showAllRoofs(); // maybe???
		// or 
		data->removedModels.clear();
		data->removableModels.clear();
		showAllRoofs();
		// FIXME: redo this whole crap.
	}
	data->updateEnabled = updateEnabled;
}

void BuildingHandler::hideAllRoofs()
{
	fb_assert(!data->updateEnabled);

	for(unsigned int i = 0; i < data->buildingModels.size(); ++i)
	{
		IStorm3D_Model *model = data->buildingModels[i];
		data->setVisibility(model, false);
	}
}

void BuildingHandler::showAllRoofs()
{
	fb_assert(!data->updateEnabled);

	for(unsigned int i = 0; i < data->buildingModels.size(); ++i)
	{
		IStorm3D_Model *model = data->buildingModels[i];
		data->setVisibility(model, true);
	}

}


} // end of namespace frozenbyte

