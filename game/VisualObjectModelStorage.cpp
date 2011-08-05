
#include "precompiled.h"

#include "VisualObjectModelStorage.h"

#include <assert.h>
#include <string.h>

#include "../ui/VisualObjectModel.h"
#include "../container/LinkedList.h"

using namespace ui;

namespace game
{
	VisualObjectModelStorage::VisualObjectModelStorage()
	{
		models = new LinkedList();
	}

	VisualObjectModelStorage::~VisualObjectModelStorage()
	{
		clear();
	}

	void VisualObjectModelStorage::clear()
	{
		while (!models->isEmpty())
		{
			VisualObjectModel *vom = (VisualObjectModel *)models->popLast();
			delete vom;
		}
	}

	VisualObjectModel *VisualObjectModelStorage::getVisualObjectModel(const char *filename)
	{
		// TODO: optimize? is it necessary?
		LinkedListIterator iter = LinkedListIterator(models);
		while (iter.iterateAvailable())
		{
			VisualObjectModel *vom = (VisualObjectModel *)iter.iterateNext();
			if (filename == NULL && vom->getFilename() == NULL)
				return vom;
			if (vom->getFilename() != NULL && filename != NULL 
				&& strcmp(vom->getFilename(), filename) == 0)
				return vom;				
		}
		VisualObjectModel *newvom = new VisualObjectModel(filename);
		models->append(newvom);
		return newvom;
	}

}

