
#ifndef UNITHIGHLIGHT_H
#define UNITHIGHLIGHT_H

#include <DatatypeDef.h>

class IStorm3D;
class IStorm3D_Scene;

namespace game
{
  class Unit;
}

namespace ui
{
	class VisualObject;
	class VisualObjectModel;


	class UnitHighlight
	{
		public:
			UnitHighlight(IStorm3D *storm3D, IStorm3D_Scene *stormScene);

			~UnitHighlight();

			void setHighlightedTerrain(VC3 &position);

			void clearHighlightedTerrain();

			void setHighlightedUnit(const game::Unit *unit);

			void run();
			
		private:
			IStorm3D *storm3D;
			IStorm3D_Scene *stormScene;
			const game::Unit *highlightUnit;
			bool highlightTerrain;
			VC3 highlightTerrainPosition;

			VisualObjectModel *visualObjectModel;
			VisualObject *visualObject;
			VisualObject *visualObject2;

			VisualObjectModel *terrainVisualObjectModel;
			VisualObject *terrainVisualObject;
			VisualObject *terrainVisualObject2;

			float rotX;
			float rotY;
			float rotZ;
	};
}

#endif

