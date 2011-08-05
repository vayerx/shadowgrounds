
#ifndef VISUALOBJECTMODELSTORAGE_H
#define VISUALOBJECTMODELSTORAGE_H

class LinkedList;
namespace ui
{
	class VisualObjectModel;
}

namespace game
{
	class VisualObjectModelStorage
	{
		public:
			VisualObjectModelStorage();

			~VisualObjectModelStorage();

			void clear();

			ui::VisualObjectModel *getVisualObjectModel(const char *filename);

		private:
			LinkedList *models;
	};
}

#endif

