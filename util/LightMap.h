
#ifndef LIGHTMAP_H
#define LIGHTMAP_H

namespace util
{
	class ColorMap;

	// NOTE: This is NOT the graphical building lightmap,
	// this is the "alien scare" lightmap, that is, a map used by
	// aliens to check if some area is illuminated or not.

	class LightMap
	{
		public:
			LightMap(ColorMap *colorMap, int sizeX, int sizeY,
				float scaledSizeX, float scaledSizeY);

			~LightMap();

			unsigned char getLightAmount(int lightX, int lightY) const;

			// add circular light
			void addSemiDynamicLight(int lightX, int lightY, int radius, float brightnessFactor);

			// remove circular light
			void removeSemiDynamicLight(int lightX, int lightY, int radius, float brightnessFactor);

			int scaledToLightMapX(float scaledX) const;
			int scaledToLightMapY(float scaledY) const;
			int scaledToLightMapRadius(float scaledRadius) const;

		private:
			int sizeX;
			int sizeY;
			float scaledSizeX;
			float scaledSizeY;
			unsigned char *lightMap;
			ColorMap *colorMap;

			void modifySemiDynamicLightImpl(int pathX, int pathY, int radius, bool add, float brightnessFactor);
	};

}

#endif


