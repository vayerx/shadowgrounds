
#ifndef DECORATIONMANAGER_H
#define DECORATIONMANAGER_H

#define DECORID_LOWEST_POSSIBLE_VALUE 100000
#define DECORID_HIGHEST_POSSIBLE_VALUE 999999

class LinkedList;

namespace util
{
	class ColorMap;
}

namespace ui
{

  class Decoration;

  class DecorationManager
  {
    public:
      DecorationManager();

      ~DecorationManager();

      Decoration *createDecoration();

      void deleteDecoration(Decoration *decoration);

      Decoration *getDecorationByName(const char *name) const;

			int getIdForDecoration(Decoration *decor);
			Decoration *getDecorationById(int id);

      void run();

      void synchronizeAllDecorations() const;

			void updateDecorationIllumination(util::ColorMap *colorMap);

    private:
      LinkedList *decorList;
  };
}

#endif



