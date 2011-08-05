
#ifndef AREAMAP_H
#define AREAMAP_H

#define AREAMAP_DATATYPE unsigned short
#define AREAMAP_MAX_DATA_VALUE 0xffff

namespace util
{
	class AreaMapImpl;

  class AreaMap
	{
		public:
			AreaMap(int sizeX, int sizeY);

			~AreaMap();

			// NOTICE: returns the unshifted value, you should shift that
			// if you want to get it between 0-n.
			int getAreaValue(int x, int y, int areaMask) const;

			bool isAreaValue(int x, int y, int areaMask, int value) const;

			bool isAreaAnyValue(int x, int y, int areaMask) const;

			// NOTICE: expects and "unshifted" value, (like the one returned
			// by getAreaValue), if you have a value between 0-n, shift
			// if first before using this method.
			void setAreaValue(int x, int y, int areaMask, int value);

			// there's no point in this, as it is actually the same as setAreaValue, after the buggy setAreaValue has 
			// been fixed. --jpk ;)
			// same as above, but clears current value first
			//void resetAreaValue(int x, int y, int areaMask, int value);

			// same as above, but applies to whole area
			void fillAreaValue(int areaMask, int value);

			// Use of this not recommended, added for backward 
			// compatibility with terrain raytrace (ugly optimizations).
			bool isAreaAnyValue(int index, int areaMask) const;

			// Use of this not recommended, added for backward 
			// compatibility with terrain raytrace (ugly optimizations).
			bool isAreaValue(int index, int areaMask, int value) const;

			// FOR SAVE/LOAD HACK PURPOSE ONLY...
			AREAMAP_DATATYPE *getInternalBuffer();

		private:
			AreaMapImpl *impl;
	};

}

#endif


