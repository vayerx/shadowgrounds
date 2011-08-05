
#include "precompiled.h"

#include "materials.h"

#include "../util/fb_assert.h"

namespace game
{
  const char *materialName[MATERIAL_AMOUNT] =
	{
		"sand",
		"rock",
		"concrete",
		"metal_hard",
		"metal_tin",
		"wood",
		"glass",
		"plastic",
		"leaves",
		"liquid",
		"metal_grate",
		"soil",
		"grass",
		"snow_shallow",
		"snow_deep",
		"_reserved_15"
	};

#ifdef PROJECT_SHADOWGROUNDS
  int materialsInUse[MATERIAL_PALETTE_AMOUNT] =
	{
		MATERIAL_SAND,
		MATERIAL_ROCK,
		MATERIAL_CONCRETE,
		MATERIAL_METAL_HARD,
		MATERIAL_METAL_TIN,
		MATERIAL_GLASS,
		MATERIAL_WOOD,
		MATERIAL_METAL_GRATE
	};
#else
  int materialsInUse[MATERIAL_PALETTE_AMOUNT] =
	{
		MATERIAL_SAND,
		MATERIAL_ROCK,
		MATERIAL_CONCRETE,
		MATERIAL_METAL_HARD,
		MATERIAL_METAL_TIN,
		MATERIAL_WOOD,
		MATERIAL_GLASS,
		MATERIAL_PLASTIC,
		MATERIAL_LEAVES,
		MATERIAL_LIQUID,
		MATERIAL_METAL_GRATE,
		MATERIAL_SOIL,
		MATERIAL_GRASS,
		MATERIAL_SNOW_SHALLOW,
		MATERIAL_SNOW_DEEP,
		MATERIAL_RESERVED_15
	};
#endif

	void changeMaterialPalette(int paletteNum, int material)
	{
		fb_assert(paletteNum >= 0 && paletteNum < MATERIAL_PALETTE_AMOUNT);
		fb_assert(material >= 0 && material < MATERIAL_AMOUNT);
		materialsInUse[paletteNum] = material;
	}

	int getMaterialByPalette(int paletteNum)
	{
		fb_assert(paletteNum >= 0 && paletteNum < MATERIAL_PALETTE_AMOUNT);
		return materialsInUse[paletteNum];
	}

}

