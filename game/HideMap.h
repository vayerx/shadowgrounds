
#ifndef HIDEMAP_H
#define HIDEMAP_H

namespace game
{
	class HideMap
	{
		public:
			enum HIDDENESS_TYPE
			{
				HIDDENESS_TYPE_NONE = 1,
				HIDDENESS_TYPE_VEGETATION = 2,
				HIDDENESS_TYPE_TREE = 3,
				HIDDENESS_TYPE_SOLID = 4
			};

			static const int maxHiddeness;


			HideMap(int sizeX, int sizeY);

			~HideMap();

			bool save(const char *filename);

			bool load(const char *filename);

			void setHiddenessType(int x, int y, HIDDENESS_TYPE hidType);

			void setHiddeness(int x, int y, int value);

			void addHiddenessToArea(int x, int y, int radius, int amount, 
				HIDDENESS_TYPE hidType);

			int getHiddenessAt(int x, int y);
			
			HIDDENESS_TYPE getHiddenessTypeAt(int x, int y);

		private:
			unsigned char *hidemap;
			int sizeX;
			int sizeY;

	}	;
}

#endif

